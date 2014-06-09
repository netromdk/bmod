#ifndef BMOD_ASM_X86_H
#define BMOD_ASM_X86_H

#include "Asm.h"

class AsmX86 : public Asm {
public:
  QString disassemble(const QByteArray &code);
};

#endif // BMOD_ASM_X86_H
