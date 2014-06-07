#ifndef BMOD_ARCH_PANE_H
#define BMOD_ARCH_PANE_H

#include "Pane.h"
#include "../BinaryObject.h"

class ArchPane : public Pane {
public:
  ArchPane(BinaryObjectPtr obj);

private:
  void createLayout();

  BinaryObjectPtr obj;
};

#endif // BMOD_ARCH_PANE_H
