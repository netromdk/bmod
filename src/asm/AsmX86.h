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
      : srcReg{0}, dstReg{0}, srcRegSet{false}, dstRegSet{false},
      srcRegType{RegType::R8}, dstRegType{RegType::R8}, scale{0}, index{0},
      base{0}, sipSrc{false}, sipDst{false}, disp{0}, imm{0}, dispSrc{false},
      dispDst{false}, immSet{false}, dispBytes{1}, offset{0}
    { }

    QString toString() const;

  private:
    QString getRegString(int reg, RegType type) const;
    QString getSipString(RegType type) const;
    QString getDispString() const;
    QString formatHex(quint32 num, int len = 2) const;

  public:
    QString mnemonic;
    unsigned char srcReg, dstReg;
    bool srcRegSet, dstRegSet;
    RegType srcRegType, dstRegType;
    unsigned char scale, index, base; // SIP values
    bool sipSrc, sipDst;
    qint32 disp, imm;
    bool dispSrc, dispDst, immSet;
    char dispBytes;
    qint32 offset;
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

  void processModRegRM(Instruction &inst);
  void processSip(Instruction &inst);
  void processDisp8(Instruction &inst);
  void processDisp32(Instruction &inst);

  BinaryObjectPtr obj;
  ReaderPtr reader;
};

#endif // BMOD_ASM_X86_H
