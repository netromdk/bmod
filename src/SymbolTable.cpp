#include "SymbolTable.h"

void SymbolTable::addSymbol(const SymbolEntry &entry) {
  entries << entry;
}

bool SymbolTable::getString(quint64 value, QString &str) const {
  foreach (const auto &entry, entries) {
    if (entry.getValue() == value) {
      const auto &s  = entry.getString();
      if (s.isEmpty()) continue;
      str = s;
      return true;
    }
  }
  return false;
}
