#ifndef BMOD_SYMBOLS_PANE_H
#define BMOD_SYMBOLS_PANE_H

#include "Pane.h"
#include "../Section.h"
#include "../BinaryObject.h"

class QLabel;
class TreeWidget;

class SymbolsPane : public Pane {
public:
  enum class Type {
    Symbols,
    DynSymbols
  };

  SymbolsPane(BinaryObjectPtr obj, SectionPtr sec, Type type);

protected:
  void showEvent(QShowEvent *event);

private:
  void createLayout();
  void setup();

  BinaryObjectPtr obj;
  SectionPtr sec;
  Type type;

  bool shown;
  QLabel *label;
  TreeWidget *treeWidget;
};

#endif // BMOD_SYMBOLS_PANE_H
