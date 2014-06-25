#include <QVBoxLayout>

#include "GenericPane.h"
#include "../widgets/MachineCodeWidget.h"

GenericPane::GenericPane(BinaryObjectPtr obj, SectionPtr sec)
  : Pane(Kind::Generic), obj{obj}, sec{sec}
{
  createLayout();
}

void GenericPane::createLayout() {
  auto *codeWidget = new MachineCodeWidget(obj, sec);
  connect(codeWidget, SIGNAL(modified()), this, SIGNAL(modified()));

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(codeWidget);
  
  setLayout(layout);
}
