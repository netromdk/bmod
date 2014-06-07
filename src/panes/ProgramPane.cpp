#include <QVBoxLayout>

#include "ProgramPane.h"
#include "../MachineCodeWidget.h"

ProgramPane::ProgramPane(BinaryObjectPtr obj, SectionPtr sec)
  : Pane(Kind::Program), obj{obj}, sec{sec}
{
  createLayout();
}

void ProgramPane::createLayout() {
  auto *codeWidget = new MachineCodeWidget(obj, sec);

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(codeWidget);
  
  setLayout(layout);
}
