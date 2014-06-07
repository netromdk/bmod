#ifndef BMOD_PROGRAM_PANE_H
#define BMOD_PROGRAM_PANE_H

#include "Pane.h"
#include "../BinaryObject.h"

class ProgramPane : public Pane {
public:
  ProgramPane(BinaryObjectPtr obj);

private:
  void createLayout();

  BinaryObjectPtr obj;
};

#endif // BMOD_PROGRAM_PANE_H
