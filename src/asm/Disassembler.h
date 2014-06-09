#ifndef BMOD_DISASSEMBLER_H
#define BMOD_DISASSEMBLER_H

#include <QString>

#include "../CpuType.h"

class Asm;
class QByteArray;

class Disassembler {
public:
  Disassembler(CpuType type);
  ~Disassembler();

  QString disassemble(const QByteArray &code);

private:
  Asm *asm_;
};

#endif // BMOD_DISASSEMBLER_H
