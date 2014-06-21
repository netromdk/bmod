#ifndef BMOD_ASM_X86_H
#define BMOD_ASM_X86_H

#include "Asm.h"
#include "../Reader.h"
#include "../BinaryObject.h"

namespace {
  enum RegType {
    R8,  // 8-bit register
    R16, // 16-bit register
    R32, // 32-bit register
    MM,  // MMX register
    XMM, // 128-bit register
    SREG // Segment register
  };
}

class AsmX86 : public Asm {
public:
  AsmX86(BinaryObjectPtr obj);
  bool disassemble(SectionPtr sec, Disassembly &result);

private:
  void addResult(const QString &line, qint64 pos, Disassembly &result);

  // Split byte into [2][3][3] bits.
  void splitByteModRM(unsigned char num, unsigned char &mod,
                      unsigned char &op1, unsigned char &op2);

  // 0 <= num <= 7
  QString getReg(RegType type, int num);

  /**
   * num is either a general-purpose register (r) or a memory address
   * (m). swap means to swap the order of the [op1] and [op2] part of
   * num and any displacement.
   *
   * 'imm' is for using an intermediate value instead of a register,
   * the value can be 0 or 1, but -1 means unused.
   */
  QString getModRMByte(unsigned char num, RegType type1, RegType type2,
                       bool swap = false, int imm = -1);

  // Format and pad num as "0x.."
  QString formatHex(quint32 num, int len = 2);

  BinaryObjectPtr obj;
  ReaderPtr reader;
};

#endif // BMOD_ASM_X86_H
