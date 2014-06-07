#ifndef BMOD_PROGRAM_PANE_H
#define BMOD_PROGRAM_PANE_H

#include "Pane.h"
#include "../Section.h"
#include "../BinaryObject.h"

class ProgramPane : public Pane {
public:
  ProgramPane(BinaryObjectPtr obj, SectionPtr sec);

private:
  void createLayout();

  BinaryObjectPtr obj;
  SectionPtr sec;
};

#endif // BMOD_PROGRAM_PANE_H
