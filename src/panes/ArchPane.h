#ifndef BMOD_ARCH_PANE_H
#define BMOD_ARCH_PANE_H

#include "Pane.h"
#include "../BinaryObject.h"
#include "../formats/FormatType.h"

class ArchPane : public Pane {
public:
  ArchPane(FormatType type, BinaryObjectPtr obj);

private:
  void createLayout();

  FormatType type;
  BinaryObjectPtr obj;
};

#endif // BMOD_ARCH_PANE_H
