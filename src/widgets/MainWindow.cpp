#include <QDir>
#include <QDebug>
#include <QMenuBar>
#include <QSettings>
#include <QFileInfo>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QProgressDialog>

#include "../Util.h"
#include "MainWindow.h"
#include "BinaryWidget.h"
#include "../formats/Format.h"
#include "ConversionHelper.h"

MainWindow::MainWindow(const QStringList &files)
  : shown{false}, startupFiles{files}
{
  // Remove possible duplicates.
  startupFiles = startupFiles.toSet().toList();

  readSettings();
  setWindowTitle("bmod");
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
  QString file =
    QFileDialog::getOpenFileName(this, tr("Open Binary"),
                                 QDir::homePath(),
                                 tr("Mach-O (* *.dylib *.bundle *.o)"));
  if (file.isEmpty()) {
    if (binaryWidgets.isEmpty()) {
      qApp->quit();
    }
    return;
  }

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

  binary->commit();
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

void MainWindow::showConversionHelper() {
  auto *helper = new ConversionHelper(this);
  helper->show();
}

void MainWindow::onRecentFile() {
  auto *action = qobject_cast<QAction*>(sender());
  if (!action) return;
  loadBinary(action->text());
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

  QMenu *toolsMenu = menuBar()->addMenu(tr("Tools"));
  toolsMenu->addAction(tr("Conversion helper"),
                       this, SLOT(showConversionHelper()));
}

void MainWindow::loadBinary(const QString &file) {
  QProgressDialog progDiag(this);
  progDiag.setLabelText(tr("Detecting format.."));
  progDiag.setCancelButton(nullptr);
  progDiag.setRange(0, 0);
  progDiag.show();
  qApp->processEvents();

  auto fmt = Format::detect(file);
  if (fmt == nullptr) {
    QMessageBox::critical(this, tr("bmod"), tr("Unknown file - could not open!"));
    return;
  }
  
  qDebug() << "detected:" << Util::formatTypeString(fmt->getType());

  progDiag.setLabelText(tr("Reading and parsing binary.."));
  qApp->processEvents();
  if (!fmt->parse()) {
    QMessageBox::warning(this, tr("bmod"), tr("Could not parse file!"));
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
  binaryWidgets << binWidget;
  int idx = tabWidget->addTab(binWidget, QFileInfo(file).fileName());
  tabWidget->setCurrentIndex(idx);
}
