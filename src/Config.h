#ifndef BMOD_CONFIG_H
#define BMOD_CONFIG_H

#include <QSettings>

class Config {
public:
  Config();
  ~Config();

  void load();
  void save();

  bool getBackupEnabled() const { return backupEnabled; }
  void setBackupEnabled(bool enabled) { backupEnabled = enabled; }

private:
  QSettings settings;
  
  bool backupEnabled;
};

#endif // BMOD_CONFIG_H
