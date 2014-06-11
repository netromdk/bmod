#ifndef BMOD_DISASSEMBLER_H
#define BMOD_DISASSEMBLER_H

#include <QString>

#include "../BinaryObject.h"

class Asm;
class QByteArray;

class Disassembler {
public:
  Disassembler(BinaryObjectPtr obj);
  ~Disassembler();

  bool disassemble(SectionPtr sec, QString &result);

private:
  Asm *asm_;
};

#endif // BMOD_DISASSEMBLER_H
