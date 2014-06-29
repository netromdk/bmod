#ifndef BMOD_PREFERENCES_DIALOG_H
#define BMOD_PREFERENCES_DIALOG_H

#include <QDialog>

class Config;
class QTabWidget;

class PreferencesDialog : public QDialog {
  Q_OBJECT

public:
  PreferencesDialog(Config &config);

private slots:
  void onBackupsToggled(bool on);

private:
  void createLayout();
  void createTabs();

  Config &config;

  QTabWidget *tabWidget;
};

#endif // BMOD_PREFERENCES_DIALOG_H
