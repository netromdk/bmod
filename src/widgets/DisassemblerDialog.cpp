#include <QLabel>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>

#include "../Util.h"
#include "../BinaryObject.h"
#include "DisassemblerDialog.h"
#include "../asm/Disassembler.h"

DisassemblerDialog::DisassemblerDialog(QWidget *parent, CpuType cpuType)
  : QDialog{parent}, cpuType{cpuType}
{
  setWindowTitle(tr("Disassembler"));
  createLayout();
  resize(400, 400);
  Util::centerWidget(this);
}

void DisassemblerDialog::onConvert() {
  QString text = machineText->toPlainText();
  if (text.isEmpty()) {
    machineText->setFocus();
    QMessageBox::warning(this, "bmod", tr("Write some machine code!"));
    return;
  }

  auto obj = BinaryObjectPtr(new BinaryObject);
  obj->setCpuType(cpuType);

  QByteArray data =
    Util::hexToData(text.simplified().trimmed().replace(" ", ""));
  int size = data.size();
  auto sec = SectionPtr(new Section(SectionType::Text, "", 0, size));
  sec->setData(data);

  Disassembler dis(obj);
  Disassembly result;
  if (dis.disassemble(sec, result)) {
    asmText->setText(result.asmLines.join("\n"));
  }
  else {
    machineText->setFocus();
    QMessageBox::warning(this, "bmod",
                         tr("Could not disassemble machine code!"));
  }
}

void DisassemblerDialog::createLayout() {
  machineText = new QTextEdit;
  machineText->setTabChangesFocus(true);

  asmText = new QTextEdit;
  asmText->setReadOnly(true);
  asmText->setTabChangesFocus(true);

  cpuTypeBox = new QComboBox;

  // TODO: For now there is only one entry so changing it has no
  // effect anyway.
  cpuTypeBox->addItem(tr("X86"), (int) CpuType::X86);

  int idx = cpuTypeBox->findData((int) cpuType);
  if (idx != -1) {
    cpuTypeBox->setCurrentIndex(idx);
  }

  auto *convertBtn = new QPushButton(tr("Disassemble"));
  connect(convertBtn, &QPushButton::clicked,
          this, &DisassemblerDialog::onConvert);

  auto *bottomLayout = new QHBoxLayout;
  bottomLayout->addWidget(new QLabel(tr("CPU:")));
  bottomLayout->addWidget(cpuTypeBox);
  bottomLayout->addStretch();
  bottomLayout->addWidget(convertBtn);
  
  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(5, 5, 5, 5);
  layout->addWidget(new QLabel(tr("Machine code:")));
  layout->addWidget(machineText);
  layout->addWidget(new QLabel(tr("Disassembly:")));
  layout->addWidget(asmText);
  layout->addLayout(bottomLayout);
  
  setLayout(layout);
}
