#include <QByteArray>

#include "Asm.h"
#include "AsmX86.h"
#include "Disassembler.h"

Disassembler::Disassembler(CpuType type) : asm_{nullptr} {
  switch (type) {
  case CpuType::X86:
    asm_ = new AsmX86;
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

bool Disassembler::disassemble(const QByteArray &code, QString &result) {
  if (!asm_) return false;
  return asm_->disassemble(code, result);
}
