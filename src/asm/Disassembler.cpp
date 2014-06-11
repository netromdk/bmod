#include <QByteArray>

#include "Asm.h"
#include "AsmX86.h"
#include "Disassembler.h"

Disassembler::Disassembler(BinaryObjectPtr obj) : asm_{nullptr} {
  switch (obj->getCpuType()) {
  case CpuType::X86:
    asm_ = new AsmX86(obj);
    break;

  default: break;
  }
}

Disassembler::~Disassembler() {
  if (asm_) {
    delete asm_;
    asm_ = nullptr;
  }
}

bool Disassembler::disassemble(SectionPtr sec, QString &result) {
  if (!asm_) return false;
  return asm_->disassemble(sec, result);
}
