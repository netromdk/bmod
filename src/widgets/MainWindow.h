#ifndef BMOD_MAIN_WINDOW_H
#define BMOD_MAIN_WINDOW_H

#include <QList>
#include <QMainWindow>

class QTabWidget;
class QStringList;
class BinaryWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(const QStringList &files = QStringList());
  ~MainWindow();

protected:
  void showEvent(QShowEvent *event);

private slots:
  void openBinary();
  void saveBinary();
  void closeBinary();
  void showConversionHelper();
  void onRecentFile();

private:
  void readSettings();
  void writeSettings();
  void createLayout();
  void createMenu();

  void loadBinary(const QString &file);

  bool shown;
  QStringList recentFiles, startupFiles;
  QByteArray geometry;

  QTabWidget *tabWidget;
  QList<BinaryWidget*> binaryWidgets;
};

#endif // BMOD_MAIN_WINDOW_H
