#include <QByteArray>

#include "Asm.h"
#include "AsmX86.h"
#include "Disassembler.h"

Disassembler::Disassembler(CpuType type) : asm_{nullptr} {
  switch (type) {
  default:
  case CpuType::X86:
    asm_ = new AsmX86;
    break;
  }
}

Disassembler::~Disassembler() {
  if (asm_) {
    delete asm_;
    asm_ = nullptr;
  }
}

QString Disassembler::disassemble(const QByteArray &code) {
  return asm_->disassemble(code);
}
