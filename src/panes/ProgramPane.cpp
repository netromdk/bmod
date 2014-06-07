#include <QVBoxLayout>

#include "ProgramPane.h"
#include "../MachineCodeWidget.h"

ProgramPane::ProgramPane(BinaryObjectPtr obj)
  : Pane(Kind::Program), obj{obj}
{
  createLayout();
}

void ProgramPane::createLayout() {
  auto *codeWidget = new MachineCodeWidget(obj);

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(codeWidget);
  
  setLayout(layout);
}
