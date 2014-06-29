#include <QLabel>
#include <QGroupBox>
#include <QTabWidget>
#include <QVBoxLayout>

#include "../Config.h"
#include "PreferencesDialog.h"

PreferencesDialog::PreferencesDialog(Config &config) : config{config} {
  createLayout();
  createTabs();
}

void PreferencesDialog::onBackupsToggled(bool on) {
  config.setBackupEnabled(on);
}

void PreferencesDialog::createLayout() {
  tabWidget = new QTabWidget;

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(5, 5, 5, 5);
  layout->addWidget(tabWidget);

  setLayout(layout);
}

void PreferencesDialog::createTabs() {
  auto *backupWidget =
    new QGroupBox(tr("Enable backups when committing changes to binaries."));
  backupWidget->setCheckable(true);
  backupWidget->setChecked(config.getBackupEnabled());
  connect(backupWidget, &QGroupBox::toggled,
          this, &PreferencesDialog::onBackupsToggled);

  auto *backupLbl =
    new QLabel(tr("Backups are saved in the same folder as the originating "
                  "binary file but with a post-fix of the form \".bakN\", where "
                  "\"N\" is the backup number."));
  backupLbl->setWordWrap(true);

  auto *backupLayout = new QVBoxLayout;
  backupLayout->addWidget(backupLbl);
  backupLayout->addStretch();
  backupWidget->setLayout(backupLayout);
  
  tabWidget->addTab(backupWidget, tr("Backup"));
}
