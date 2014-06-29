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

  bool getBackupAsk() const { return backupAsk; }
  void setBackupAsk(bool ask) { this->backupAsk = ask; }

private:
  QSettings settings;
  
  bool backupEnabled, backupAsk;
};

#endif // BMOD_CONFIG_H
