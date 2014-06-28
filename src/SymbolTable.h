#ifndef BMOD_SYMBOL_TABLE_H
#define BMOD_SYMBOL_TABLE_H

#include <QList>
#include <QString>

class SymbolEntry {
public:
  SymbolEntry(quint32 index, quint64 value,
              const QString &strValue = QString())
    : index{index}, value{value}, strValue{strValue}
  { }

  quint32 getIndex() const { return index; }

  void setValue(quint64 value) { this->value = value; }
  quint64 getValue() const { return value; }

  void setString(const QString &str) { strValue = str; }
  const QString &getString() const { return strValue; }

private:
  quint32 index; // of string table
  quint64 value; // of symbol
  QString strValue; // String table value
};

class SymbolTable {
public:
  void addSymbol(const SymbolEntry &entry);
  QList<SymbolEntry> &getSymbols() { return entries; }  
  const QList<SymbolEntry> &getSymbols() const { return entries; }

  bool getString(quint64 value, QString &str) const;

private:
  QList<SymbolEntry> entries;
};

#endif // BMOD_SYMBOL_TABLE_H
