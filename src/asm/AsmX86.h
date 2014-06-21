#ifndef BMOD_ASM_X86_H
#define BMOD_ASM_X86_H

#include "Asm.h"
#include "../Reader.h"
#include "../BinaryObject.h"

namespace {
  enum class RegType : int {
    R8,  // 8-bit register
    R16, // 16-bit register
    R32, // 32-bit register
    MM,  // MMX register
    XMM, // 128-bit register
    SREG // Segment register
  };

  enum class Register : int {
    // 8-bit registers
    AL, CL, DL, BL, AH, CH, DH, BH,

    // 16-bit registers
    AX, CX, DX, BX, SP, BP, SI, DI,

    // 32-bit registers
    EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI,

    // MMX registers
    MM0, MM1, MM2, MM3, MM4, MM5, MM6, MM7,

    // 128-bit registers
    XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7,

    // Segment registers
    ES, CS, SS, DS, FS, GS
  };

  class Instruction {
  public:
    Instruction()
      : opcode{0}, mod{-1}, reg{-1}, rm{-1}, scale{-1}, index{-1}, base{-1},
      disp{0}, imm{0}
    { }

    QString toString() const;

    unsigned char opcode;
    char mod, reg, rm; // mod-reg-r/m values
    char scale, index, base; // SIP values
    qint32 disp, // displacement
      imm; // immediate data
  };
}

class AsmX86 : public Asm {
public:
  AsmX86(BinaryObjectPtr obj);
  bool disassemble(SectionPtr sec, Disassembly &result);

private:
  void addResult(const QString &line, qint64 pos, Disassembly &result);

  // Split byte into [2][3][3] bits.
  void splitByte(unsigned char num, unsigned char &mod, unsigned char &op1,
                 unsigned char &op2);

  // 0 <= num <= 7
  //QString getReg(RegType type, int num);

  // Format and pad num as "0x.."
  QString formatHex(quint32 num, int len = 2);

  BinaryObjectPtr obj;
  ReaderPtr reader;
};

#endif // BMOD_ASM_X86_H
