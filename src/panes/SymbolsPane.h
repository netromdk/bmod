#ifndef BMOD_SYMBOLS_PANE_H
#define BMOD_SYMBOLS_PANE_H

#include "Pane.h"
#include "../Section.h"
#include "../BinaryObject.h"

class QLabel;
class TreeWidget;

class SymbolsPane : public Pane {
public:
  SymbolsPane(BinaryObjectPtr obj, SectionPtr sec);

protected:
  void showEvent(QShowEvent *event);

private:
  void createLayout();
  void setup();

  BinaryObjectPtr obj;
  SectionPtr sec;

  bool shown;
  QLabel *label;
  TreeWidget *treeWidget;
};

#endif // BMOD_SYMBOLS_PANE_H
