#include <QDir>
#include <QDebug>
#include <QMenuBar>
#include <QFileInfo>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QProgressDialog>

#include "MainWindow.h"
#include "BinaryWidget.h"
#include "formats/Format.h"

MainWindow::MainWindow() {
  setWindowTitle("bmod");
  createLayout();
  createMenu();
  resize(600, 400);

  // If no file was specified on CLI then show open-file-dialog.
  openBinary();
}

void MainWindow::openBinary() {
  QString file =
    QFileDialog::getOpenFileName(this, tr("Open Binary"),
                                 QDir::homePath(),
                                 tr("Mach-O (* *.dylib *.bundle *.o)"));
  if (file.isEmpty()) return;

  loadBinary(file);
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
  QAction *openBin = fileMenu->addAction("Open binary");
  connect(openBin, &QAction::triggered, this, &MainWindow::openBinary);
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
  
  qDebug() << "detected:" << fmt->getName();

  progDiag.setLabelText(tr("Reading and parsing binary.."));
  qApp->processEvents();
  if (!fmt->parse()) {
    QMessageBox::warning(this, tr("bmod"), tr("Could not parse file!"));
    return;
  }

  auto *binWidget = new BinaryWidget(fmt);
  binaryWidgets << binWidget;
  tabWidget->addTab(binWidget, QFileInfo(file).fileName());
}
