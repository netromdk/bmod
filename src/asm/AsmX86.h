#ifndef BMOD_ASM_X86_H
#define BMOD_ASM_X86_H

#include "Asm.h"

class AsmX86 : public Asm {
public:
  bool disassemble(const QByteArray &code, QString &result);
};

#endif // BMOD_ASM_X86_H
