#ifndef BMOD_ASM_H
#define BMOD_ASM_H

#include <QString>

#include "../Section.h"

class Asm {
public:
  virtual ~Asm() { }
  virtual bool disassemble(SectionPtr sec, QString &result) =0;
};

#endif // BMOD_ASM_H
