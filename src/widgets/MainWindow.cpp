#include <QDebug>
#include <QMenuBar>
#include <QSettings>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QProgressDialog>

#include "../Util.h"
#include "MainWindow.h"
#include "BinaryWidget.h"
#include "ConversionHelper.h"
#include "../formats/Format.h"
#include "PreferencesDialog.h"
#include "DisassemblerDialog.h"

MainWindow::MainWindow(const QStringList &files)
  : shown{false}, startupFiles{files}
{
  // Remove possible duplicates.
  startupFiles = startupFiles.toSet().toList();

  setWindowTitle("bmod");
  readSettings();
  createLayout();
  createMenu();
}

MainWindow::~MainWindow() {
  writeSettings();
}

void MainWindow::showEvent(QShowEvent *event) {
  QMainWindow::showEvent(event);

  if (shown) return;
  shown = true;

  if (geometry.isEmpty()) {
    resize(900, 500);
    Util::centerWidget(this);
  }
  else {
    restoreGeometry(geometry);
  }

  // Load specified files or open file dialog.
  if (startupFiles.isEmpty()) {
    openBinary();
  }
  else {
    foreach (const auto &file, startupFiles) {
      loadBinary(file);
    }
  }
}

void MainWindow::openBinary() {
  QFileDialog diag(this, tr("Open Binary"), QDir::homePath());
  diag.setNameFilters(QStringList{"Mach-O binary (*.o *.dylib *.bundle *)",
                                  "Any file (*)"});
  if (!diag.exec()) {
    if (binaryWidgets.isEmpty()) {
      qApp->quit();
    }
    return;
  }

  QString file = diag.selectedFiles().first();
  foreach (const auto *binary, binaryWidgets) {
    if (binary->getFile() == file) {
      QMessageBox::warning(this, "bmod", tr("Can't open same binary twice!"));
      return;
    }
  }

  loadBinary(file);
}

void MainWindow::saveBinary() {
  if (binaryWidgets.isEmpty()) {
    return;
  }

  int idx = tabWidget->currentIndex();
  auto *binary = binaryWidgets[idx];

  auto answer =
    QMessageBox::question(this, "bmod",
                          tr("Are you sure you want to commit changes to file \"%1\"?")
                          .arg(binary->getFile()));
  if (answer == QMessageBox::No) {
    return;
  }

  if (config.getBackupEnabled()) {
    bool backup{true};
    if (config.getBackupAsk()) {
      auto answer =
        QMessageBox::question(this, "bmod",
                              tr("Do you want to save a backup before committing?"));
      backup = (answer == QMessageBox::Yes);
    }
    if (backup) {
      saveBackup(binary->getFile());
    }
  }

  return;//

  binary->commit();

  QString text = tabWidget->tabText(idx);
  if (text.endsWith(" *")) {
    text.chop(2);
    tabWidget->setTabText(idx, text);
  }
}

void MainWindow::closeBinary() {
  int idx = tabWidget->currentIndex();
  if (idx != -1) {
    auto answer =
      QMessageBox::question(this, "bmod",
                            tr("Are you sure you want to close the binary?"));
    if (answer == QMessageBox::No) {
      return;
    }

    tabWidget->removeTab(idx);
    delete binaryWidgets.takeAt(idx);
  }

  if (binaryWidgets.isEmpty()) {
    qApp->quit();
  }
}

void MainWindow::showPreferences() {
  PreferencesDialog diag(config);
  diag.exec();
  config.save();
}

void MainWindow::showConversionHelper() {
  auto *helper = new ConversionHelper(this);
  helper->show();
}

void MainWindow::showDisassembler() {
  auto *disass = new DisassemblerDialog(this);
  disass->show();
}

void MainWindow::onRecentFile() {
  auto *action = qobject_cast<QAction*>(sender());
  if (!action) return;
  loadBinary(action->text());
}

void MainWindow::onBinaryObjectModified() {
  auto *bin = qobject_cast<BinaryWidget*>(sender());
  if (!bin) return;

  int idx = tabWidget->currentIndex();
  QString text = tabWidget->tabText(idx);
  if (!text.endsWith(" *")) {
    tabWidget->setTabText(idx, text + " *");
  }
}

void MainWindow::readSettings() {
  QSettings settings;
  geometry = settings.value("MainWindow_geometry", QByteArray()).toByteArray();

  recentFiles =
    settings.value("MainWindow_recent_files", QStringList()).toStringList();
  for (int i = recentFiles.size() - 1; i >= 0; i--) {
    if (!QFile::exists(recentFiles[i])) {
      recentFiles.removeAt(i);
    }
  }
  if (recentFiles.size() > 10) {
    recentFiles = recentFiles.mid(recentFiles.size() - 10);
  }
}

void MainWindow::writeSettings() {
  QSettings settings;
  settings.setValue("MainWindow_geometry", saveGeometry());
  settings.setValue("MainWindow_recent_files", recentFiles);
}

void MainWindow::createLayout() {
  tabWidget = new QTabWidget;

  auto *layout = new QVBoxLayout;
  layout->addWidget(tabWidget);

  auto *w = new QWidget;
  w->setLayout(layout);
  setCentralWidget(w);
}

void MainWindow::createMenu() {
  QMenu *fileMenu = menuBar()->addMenu(tr("File"));
  fileMenu->addAction(tr("Open binary"), this, SLOT(openBinary()),
                      QKeySequence::Open);

  if (!recentFiles.isEmpty()) {
    QMenu *recentMenu = fileMenu->addMenu(tr("Open recent files"));
    foreach (const auto &file, recentFiles) {
      recentMenu->addAction(file, this, SLOT(onRecentFile()));
    }
  }

  fileMenu->addAction(tr("Save binary"), this, SLOT(saveBinary()),
                      QKeySequence::Save);
  fileMenu->addAction(tr("Close binary"), this, SLOT(closeBinary()),
                      QKeySequence::Close);
#ifndef MAC
  fileMenu->addSeparator();
#endif
  fileMenu->addAction(tr("Preferences"), this, SLOT(showPreferences()),
                      QKeySequence(Qt::CTRL + Qt::Key_P));

  QMenu *toolsMenu = menuBar()->addMenu(tr("Tools"));
  toolsMenu->addAction(tr("Conversion helper"),
                       this, SLOT(showConversionHelper()),
                       QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_C));
  toolsMenu->addAction(tr("Disassembler"),
                       this, SLOT(showDisassembler()),
                       QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_D));
}

void MainWindow::loadBinary(QString file) {
  // If .app then resolve the internal binary file.
  QString appBin = Util::resolveAppBinary(file);
  if (!appBin.isEmpty()) {
    file = appBin;
  }

  QProgressDialog progDiag(this);
  progDiag.setLabelText(tr("Detecting format.."));
  progDiag.setCancelButton(nullptr);
  progDiag.setRange(0, 0);
  progDiag.show();
  qApp->processEvents();

  auto fmt = Format::detect(file);
  if (fmt == nullptr) {
    QMessageBox::critical(this, "bmod", tr("Unknown file - could not open!"));
    return;
  }
  
  qDebug() << "detected:" << Util::formatTypeString(fmt->getType());

  progDiag.setLabelText(tr("Reading and parsing binary.."));
  qApp->processEvents();
  if (!fmt->parse()) {
    QMessageBox::warning(this, "bmod", tr("Could not parse file!"));
    return;
  }

  // Add recent file.
  if (!recentFiles.contains(file)) {
    recentFiles << file;
  }
  if (recentFiles.size() > 10) {
    recentFiles.removeFirst();
  }

  auto *binWidget = new BinaryWidget(fmt);
  connect(binWidget, &BinaryWidget::modified,
          this, &MainWindow::onBinaryObjectModified);
  binaryWidgets << binWidget;
  int idx = tabWidget->addTab(binWidget, QFileInfo(file).fileName());
  tabWidget->setCurrentIndex(idx);
}

void MainWindow::saveBackup(const QString &file) {
  // Determine if prior backups have been made and, if so, how many.
  QFileInfo fi(file);
  QDir dir = fi.dir();
  int bakCount = 0, bakNum = 0;
  QStringList files;
  foreach (const auto &entry,
           dir.entryInfoList(QStringList{QString("%1.bak*").arg(fi.fileName())},
                             QDir::Files, QDir::Name)) {
    files << entry.absoluteFilePath();
    bakCount++;
    QString ext = entry.suffix();
    static QRegExp re("bak([\\d]+)$");
    if (re.indexIn(ext) != -1 && re.captureCount() == 1) {
      bakNum = re.capturedTexts()[1].toUInt();
    }
  }

  // Remove previous backups if not unlimited. And remove one due to
  // the file that will be created beneath.
  int maxAmount = config.getBackupAmount();
  if (maxAmount > 0 && bakCount >= maxAmount - 1) {
    for (int i = 0; i < bakCount - (maxAmount - 1); i++) {
      QFile::remove(files[i]);
    }
  }

  QString num = Util::padString(QString::number(++bakNum), 4);
  QFile::copy(file, QString("%1.bak%2").arg(file).arg(num));
}
