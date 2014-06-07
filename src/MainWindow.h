#ifndef BMOD_MAIN_WINDOW_H
#define BMOD_MAIN_WINDOW_H

#include <QList>
#include <QMainWindow>

class QTabWidget;
class BinaryWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow();

private slots:
  void openBinary();

private:
  void createLayout();
  void createMenu();

  QTabWidget *tabWidget;
  QList<BinaryWidget*> binaryWidgets;
};

#endif // BMOD_MAIN_WINDOW_H
