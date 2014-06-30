#ifndef BMOD_CONFIG_H
#define BMOD_CONFIG_H

#include <QSettings>

class Config {
public:
  Config();
  ~Config();

  void load();
  void save();

  bool getConfirmCommit() const { return confirmCommit; }
  void setConfirmCommit(bool confirm) { confirmCommit = confirm; }

  bool getConfirmQuit() const { return confirmQuit; }
  void setConfirmQuit(bool confirm) { confirmQuit = confirm; }

  bool getBackupEnabled() const { return backupEnabled; }
  void setBackupEnabled(bool enabled) { backupEnabled = enabled; }

  bool getBackupAsk() const { return backupAsk; }
  void setBackupAsk(bool ask) { this->backupAsk = ask; }

  int getBackupAmount() const { return backupAmount; }
  void setBackupAmount(int amount) { backupAmount = amount; }

private:
  QSettings settings;

  // General
  bool confirmCommit, confirmQuit;

  // Backup
  bool backupEnabled, backupAsk;
  int backupAmount;
};

#endif // BMOD_CONFIG_H
