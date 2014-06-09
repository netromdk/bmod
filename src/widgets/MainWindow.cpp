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

  setWindowTitle("bmod");
  createLayout();
  createMenu();
}

MainWindow::~MainWindow() {
  QSettings settings;
  settings.setValue("MainWindow_geometry", saveGeometry());
}

void MainWindow::showEvent(QShowEvent *event) {
  QMainWindow::showEvent(event);

  if (shown) return;
  shown = true;

  QSettings settings;
  QVariant val = settings.value("MainWindow_geometry");
  if (val.isNull()) {
    resize(900, 500);
    Util::centerWidget(this);
  }
  else {
    restoreGeometry(val.toByteArray());
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

  auto *binWidget = new BinaryWidget(fmt);
  binaryWidgets << binWidget;
  int idx = tabWidget->addTab(binWidget, QFileInfo(file).fileName());
  tabWidget->setCurrentIndex(idx);
}
