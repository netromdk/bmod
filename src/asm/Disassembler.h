#ifndef BMOD_DISASSEMBLER_H
#define BMOD_DISASSEMBLER_H

#include <QString>
#include <QStringList>

#include "../BinaryObject.h"

class Asm;
class QByteArray;

struct Disassembly {
  // Machine code to assembly language.
  QStringList asmLines;

  // Bytes consumed per ASM line produced.
  QList<short> bytesConsumed;
};

class Disassembler {
public:
  Disassembler(BinaryObjectPtr obj);
  ~Disassembler();

  bool disassemble(SectionPtr sec, Disassembly &result);
  bool disassemble(const QByteArray &data, Disassembly &result,
                   quint64 offset = 0);
  bool disassemble(const QString &data, Disassembly &result,
                   quint64 offset = 0);

private:
  Asm *asm_;
};

#endif // BMOD_DISASSEMBLER_H
