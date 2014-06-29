#ifndef BMOD_ASM_X86_H
#define BMOD_ASM_X86_H

#include "Asm.h"
#include "../Reader.h"
#include "../BinaryObject.h"

namespace {
  enum class RegType : int {
    R8,  // 8-bit register without REX prefix
    R8R, // 8-bit register with any REX prefix
    R16, // 16-bit register
    R32, // 32-bit register
    R64, // 64-bit register
    MM,  // MMX register
    XMM, // 128-bit register
    SREG // Segment register
  };

  enum class DataType : int {
    None,       // When it should not be shown, like for CALL and JMP.
    Byte,       // 8-bit
    Word,       // 16-bit
    Doubleword, // 32-bit
    Quadword    // 64-bit
  };

  class Instruction {
  public:
    Instruction()
      : dataType{DataType::Doubleword}, srcReg{0}, dstReg{0}, srcRegSet{false},
      dstRegSet{false}, srcRegType{RegType::R32}, dstRegType{RegType::R32},
      scale{0}, index{0}, base{0}, sipSrc{false}, sipDst{false}, disp{0},
      imm{0}, dispSrc{false}, dispDst{false}, immSrc{false}, immDst{false},
      dispBytes{1}, immBytes{1}, offset{0}, call{false}, rexW{false},
      rexR{false}, rexX{false}, rexB{false}
    { }

    QString toString(BinaryObjectPtr obj) const;
    void reverse();

  private:
    QString getRegString(int reg, RegType type,
                         RegType type2 = RegType::SREG) const;
    QString getSipString(RegType type) const;
    QString getDispString() const;
    QString getImmString() const;
    QString formatHex(quint64 num, int len = 2) const;

  public:
    QString mnemonic;
    DataType dataType;
    unsigned char srcReg, dstReg;
    bool srcRegSet, dstRegSet;
    RegType srcRegType, dstRegType;
    unsigned char scale, index, base; // SIP values
    bool sipSrc, sipDst;
    quint64 disp, imm;
    bool dispSrc, dispDst, immSrc, immDst;
    char dispBytes, immBytes;
    quint64 offset;
    bool call;
    bool rexW, rexR, rexX, rexB;
  };
}

class AsmX86 : public Asm {
public:
  AsmX86(BinaryObjectPtr obj);
  bool disassemble(SectionPtr sec, Disassembly &result);

private:
  bool handleNops(Disassembly &result);
  void addResult(const Instruction &inst, qint64 pos, Disassembly &result);
  void addResult(const QString &inst, qint64 pos, Disassembly &result);

  // Split byte into [2][3][3] bits.
  void splitByte(unsigned char num, unsigned char &mod, unsigned char &op1,
                 unsigned char &op2);

  // Get last 3 bits.
  unsigned char getR(unsigned char num);

  // Split REX values [0100WRXB]. (64-bit only)
  void splitRex(unsigned char num, bool &w, bool &r, bool &x, bool &b);

  void processModRegRM(Instruction &inst, bool noSip = false);
  void processSip(Instruction &inst);
  void processDisp8(Instruction &inst);
  void processDisp32(Instruction &inst);
  void processImm8(Instruction &inst);
  void processImm32(Instruction &inst);
  void processImm64(Instruction &inst);

  BinaryObjectPtr obj;
  ReaderPtr reader;
};

#endif // BMOD_ASM_X86_H
