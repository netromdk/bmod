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

  class Instruction {
  public:
    Instruction()
      : mod{0}, srcReg{0}, dstReg{0}, rm{0}, srcRegSet{false}, dstRegSet{false},
      srcRegType{RegType::R8}, dstRegType{RegType::R8}, scale{-1}, index{-1},
      base{-1}, srcDisp{0}, dstDisp{0}, srcImm{0}, dstImm{0}
    { }

    QString toString() const;

  private:
    QString getRegString(int reg, RegType type) const;

  public:
    QString mnemonic;
    unsigned char mod, srcReg, dstReg, rm; // mod-reg-r/m values
    bool srcRegSet, dstRegSet;
    RegType srcRegType, dstRegType;
    char scale, index, base; // SIP values
    qint32 srcDisp, dstDisp, // displacement
      srcImm, dstImm; // immediate data
  };
}

class AsmX86 : public Asm {
public:
  AsmX86(BinaryObjectPtr obj);
  bool disassemble(SectionPtr sec, Disassembly &result);

private:
  void addResult(const Instruction &inst, qint64 pos, Disassembly &result);
  void addResult(const QString &inst, qint64 pos, Disassembly &result);

  // Split byte into [2][3][3] bits.
  void splitByte(unsigned char num, unsigned char &mod, unsigned char &op1,
                 unsigned char &op2);

  // Get last 3 bits.
  unsigned char getR(unsigned char num);

  void processModRegRM(unsigned char ch, Instruction &inst);

  // Format and pad num as "0x.."
  QString formatHex(quint32 num, int len = 2);

  BinaryObjectPtr obj;
  ReaderPtr reader;
};

#endif // BMOD_ASM_X86_H
