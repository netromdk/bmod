#ifndef BMOD_ASM_H
#define BMOD_ASM_H

#include "../Section.h"
#include "Disassembler.h"

class Asm {
public:
  virtual ~Asm() { }
  virtual bool disassemble(SectionPtr sec, Disassembly &result) =0;
};

#endif // BMOD_ASM_H
