#include <QVBoxLayout>

#include "AssemblyPane.h"
#include "../widgets/MachineCodeWidget.h"

AssemblyPane::AssemblyPane(BinaryObjectPtr obj, SectionPtr sec)
  : Pane(Kind::Assembly), obj{obj}, sec{sec}
{
  createLayout();
}

void AssemblyPane::createLayout() {
  /*
  auto *codeWidget = new MachineCodeWidget(obj, sec);

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(codeWidget);
  
  setLayout(layout);
  */
}
