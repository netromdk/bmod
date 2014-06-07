#ifndef BMOD_PROGRAM_PANE_H
#define BMOD_PROGRAM_PANE_H

#include "Pane.h"
#include "../BinaryObject.h"

class QLabel;
class QTreeWidget;

class ProgramPane : public Pane {
public:
  ProgramPane(BinaryObjectPtr obj);

protected:
  void showEvent(QShowEvent *event);

private:
  void createLayout();
  void setup();

  BinaryObjectPtr obj;

  bool shown;
  QLabel *label;
  QTreeWidget *treeWidget;
};

#endif // BMOD_PROGRAM_PANE_H
