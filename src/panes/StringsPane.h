#ifndef BMOD_STRINGS_PANE_H
#define BMOD_STRINGS_PANE_H

#include "Pane.h"
#include "../BinaryObject.h"

class StringsPane : public Pane {
public:
  StringsPane(BinaryObjectPtr obj);

private:
  void createLayout();

  BinaryObjectPtr obj;
};

#endif // BMOD_STRINGS_PANE_H
