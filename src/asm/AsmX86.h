#ifndef BMOD_ASM_X86_H
#define BMOD_ASM_X86_H

#include "Asm.h"

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
  bool disassemble(const QByteArray &code, QString &result);

private:
  // Split byte into [2][3][3] bits.
  void splitByteModRM(unsigned char num, unsigned char &mod,
                      unsigned char &first, unsigned char &second);

  // 0 <= num <= 7
  QString getReg(RegType type, int num);

  /**
   * num is either a general-purpose register (r) or a memory address
   * (m).
   */
  QString getModRMByte(unsigned char num, RegType type);

  // Format num as "$0x.."
  QString formatHex(quint32 num, int len = 2);
};

#endif // BMOD_ASM_X86_H
