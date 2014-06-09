#ifndef BMOD_GENERIC_PANE_H
#define BMOD_GENERIC_PANE_H

#include "Pane.h"
#include "../Section.h"
#include "../BinaryObject.h"

class GenericPane : public Pane {
public:
  GenericPane(BinaryObjectPtr obj, SectionPtr sec);

private:
  void createLayout();

  BinaryObjectPtr obj;
  SectionPtr sec;
};

#endif // BMOD_GENERIC_PANE_H
