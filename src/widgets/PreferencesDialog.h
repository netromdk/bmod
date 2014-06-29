#ifndef BMOD_PREFERENCES_DIALOG_H
#define BMOD_PREFERENCES_DIALOG_H

#include <QDialog>

class Config;
class QLabel;
class QTabWidget;

class PreferencesDialog : public QDialog {
  Q_OBJECT

public:
  PreferencesDialog(Config &config);

private slots:
  void onBackupsToggled(bool on);
  void onBackupAskChanged(int state);
  void onBackupAmountChanged(int amount);

private:
  void createLayout();
  void createTabs();

  Config &config;

  QTabWidget *tabWidget;
  QLabel *backupAmountInfo;
};

#endif // BMOD_PREFERENCES_DIALOG_H
