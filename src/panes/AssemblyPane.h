#ifndef BMOD_ASSEMBLY_PANE_H
#define BMOD_ASSEMBLY_PANE_H

#include "Pane.h"
#include "../Section.h"
#include "../BinaryObject.h"

class QLabel;

class AssemblyPane : public Pane {
public:
  AssemblyPane(BinaryObjectPtr obj, SectionPtr sec);

protected:
  void showEvent(QShowEvent *event);

private:
  void createLayout();
  void setup();

  BinaryObjectPtr obj;
  SectionPtr sec;

  bool shown;
  QLabel *asmLabel;
};

#endif // BMOD_ASSEMBLY_PANE_H
