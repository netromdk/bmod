#include "SymbolTable.h"

void SymbolTable::addSymbol(const SymbolEntry &entry) {
  entries << entry;
}

bool SymbolTable::getString(quint64 value, QString &str) const {
  foreach (const auto &entry, entries) {
    if (entry.getValue() == value) {
      str = entry.getString();
      return true;
    }
  }
  return false;
}
