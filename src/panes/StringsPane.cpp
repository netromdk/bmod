#include <QVBoxLayout>

#include "StringsPane.h"
#include "../MachineCodeWidget.h"

StringsPane::StringsPane(BinaryObjectPtr obj)
  : Pane(Kind::Strings), obj{obj}
{
  createLayout();
}

void StringsPane::createLayout() {
  auto *codeWidget = new MachineCodeWidget(obj, SectionType::CString);

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(codeWidget);
  
  setLayout(layout);
}
