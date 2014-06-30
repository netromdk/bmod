#include <QDebug>
#include <QStandardPaths>

#include "Config.h"

Config::Config()
  : settings(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/bmod.ini",
             QSettings::IniFormat)
{
  qDebug() << "Using config:" << qPrintable(settings.fileName());
  load();
}

Config::~Config() {
  save();
}

void Config::load() {
  settings.beginReadArray("General");
  confirmCommit = settings.value("confirmCommit", true).toBool();
  settings.endArray();

  settings.beginReadArray("Backups");
  backupEnabled = settings.value("backupEnabled", true).toBool();
  backupAsk = settings.value("backupAsk", true).toBool();
  backupAmount = settings.value("backupAmount", 5).toInt();
  if (backupAmount < 0) backupAmount = 0;
  settings.endArray();
}

void Config::save() {
  settings.clear();

  settings.beginGroup("General");
  settings.setValue("confirmCommit", confirmCommit);
  settings.endGroup();

  settings.beginGroup("Backups");
  settings.setValue("backupEnabled", backupEnabled);
  settings.setValue("backupAsk", backupAsk);
  settings.setValue("backupAmount", backupAmount);
  settings.endGroup();

  settings.sync();
}
