#ifndef BMOD_MAIN_WINDOW_H
#define BMOD_MAIN_WINDOW_H

#include <QList>
#include <QMainWindow>

#include "Config.h"

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
  void closeEvent(QCloseEvent *event);

private slots:
  void openBinary();
  void saveBinary();
  void closeBinary();
  void showPreferences();
  void showConversionHelper();
  void showDisassembler();
  void onRecentFile();
  void onBinaryObjectModified();

private:
  void readSettings();
  void writeSettings();
  void createLayout();
  void createMenu();

  void loadBinary(QString file);
  void saveBackup(const QString &file);

  Config config;
  bool shown, modified;
  QStringList recentFiles, startupFiles;
  QByteArray geometry;

  QTabWidget *tabWidget;
  QList<BinaryWidget*> binaryWidgets;
};

#endif // BMOD_MAIN_WINDOW_H
