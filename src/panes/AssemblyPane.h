#ifndef BMOD_ASSEMBLY_PANE_H
#define BMOD_ASSEMBLY_PANE_H

#include "Pane.h"
#include "../Section.h"
#include "../BinaryObject.h"

class AssemblyPane : public Pane {
public:
  AssemblyPane(BinaryObjectPtr obj, SectionPtr sec);

private:
  void createLayout();

  BinaryObjectPtr obj;
  SectionPtr sec;
};

#endif // BMOD_ASSEMBLY_PANE_H
