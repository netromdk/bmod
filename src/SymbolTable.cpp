#include "SymbolTable.h"

void SymbolTable::addSymbol(const SymbolEntry &entry) {
  entries << entry;
}
