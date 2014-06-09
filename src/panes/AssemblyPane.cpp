#include <QLabel>
#include <QVBoxLayout>

#include "AssemblyPane.h"
#include "../asm/Disassembler.h"
#include "../widgets/MachineCodeWidget.h"

AssemblyPane::AssemblyPane(BinaryObjectPtr obj, SectionPtr sec)
  : Pane(Kind::Assembly), obj{obj}, sec{sec}, shown{false}
{
  createLayout();
}

void AssemblyPane::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);
  if (!shown) {
    shown = true;
    setup();
  }
}

void AssemblyPane::createLayout() {
  asmLabel = new QLabel;

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(asmLabel);
  
  setLayout(layout);
}

void AssemblyPane::setup() {
  Disassembler dis(obj->getCpuType());
  QString asm_;
  if (dis.disassemble(sec->getData(), asm_)) {
    asmLabel->setText(asm_);
  }
  else {
    asmLabel->setText(tr("Could not disassemble machine code!"));
  }
}
