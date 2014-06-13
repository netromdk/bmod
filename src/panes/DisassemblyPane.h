#ifndef BMOD_DISASSEMBLY_PANE_H
#define BMOD_DISASSEMBLY_PANE_H

#include <QDateTime>

#include "Pane.h"
#include "../Section.h"
#include "../BinaryObject.h"

class QLabel;
class TreeWidget;

class DisassemblyPane : public Pane {
public:
  DisassemblyPane(BinaryObjectPtr obj, SectionPtr sec);

protected:
  void showEvent(QShowEvent *event);

private:
  void createLayout();
  void setup();

  BinaryObjectPtr obj;
  SectionPtr sec;
  QDateTime secModified;

  bool shown;
  QLabel *label;
  TreeWidget *treeWidget;
};

#endif // BMOD_DISASSEMBLY_PANE_H
