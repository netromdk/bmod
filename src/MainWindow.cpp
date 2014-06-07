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

#include "Util.h"
#include "MainWindow.h"
#include "BinaryWidget.h"
#include "formats/Format.h"

MainWindow::MainWindow(const QStringList &files) {
  setWindowTitle("bmod");
  createLayout();
  createMenu();

  // Load specified files or open file dialog.
  if (files.isEmpty()) {
    openBinary();
  }
  else {
    foreach (const auto &file, files) {
      loadBinary(file);
    }
  }
}

MainWindow::~MainWindow() {
  QSettings settings;
  settings.setValue("MainWindow_geometry", saveGeometry());
}

void MainWindow::showEvent(QShowEvent *event) {
  QMainWindow::showEvent(event);

  QSettings settings;
  QVariant val = settings.value("MainWindow_geometry");
  if (val.isNull()) {
    resize(900, 500);
    Util::centerWidget(this);
  }
  else {
    restoreGeometry(val.toByteArray());
  }
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
