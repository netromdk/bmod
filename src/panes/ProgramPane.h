#ifndef BMOD_PROGRAM_PANE_H
#define BMOD_PROGRAM_PANE_H

#include "Pane.h"
#include "../BinaryObject.h"

class QTreeWidget;

class ProgramPane : public Pane {
public:
  ProgramPane(BinaryObjectPtr obj);

private:
  void createLayout();
  void setup();

  BinaryObjectPtr obj;

  QTreeWidget *treeWidget;
};

#endif // BMOD_PROGRAM_PANE_H
