/**
 * References:
 *   http://www.read.seas.harvard.edu/~kohler/class/04f-aos/ref/i386.pdf
 *   http://ref.x86asm.net
 */

#include <QDebug>
#include <QBuffer>

#include <cmath>

#include "AsmX86.h"
#include "../Util.h"
#include "../Reader.h"
#include "../Section.h"

namespace {
  QString Instruction::toString() const {
    QString str(mnemonic);

    bool comma{true};
    if (dstRegSet) {
      if (!str.endsWith(" ")) str += " ";
      if (dispDst) {
        str += getDispString() + "(";
      }
      else if (immDst) {
        str += getImmString() + ",";
      }
      str += getRegString(dstReg, dstRegType,
                          dispDst ? RegType::SREG : srcRegType);
      if (dispDst) {
        str += ")";
      }
    }
    else if (sipDst) {
      if (!str.endsWith(" ")) str += " ";
      if (dispDst) {
        str += getDispString();
      }
      str += getSipString(dstRegType);
    }
    else if (dispDst) {
      if (!str.endsWith(" ")) str += " ";
      str += getDispString();
    }
    else if (immDst) {
      if (!str.endsWith(" ")) str += " ";
      str += getImmString();
    }
    else {
      comma = false;
    }

    if (srcRegSet) {
      if (!comma && !str.endsWith(" ")) {
        str += " ";
      }
      else {
        str += ",";
      }
      if (dispSrc) {
        str += getDispString() + "(";
      }
      else if (immSrc) {
        str += getImmString() + ",";
      }
      str += getRegString(srcReg, srcRegType,
                          dispSrc ? RegType::SREG : dstRegType);
      if (dispSrc) {
        str += ")";
      }
    }
    else if (sipSrc) {
      if (!comma && !str.endsWith(" ")) {
        str += " ";
      }
      else {
        str += ",";
      }
      if (dispSrc) {
        str += getDispString();
      }
      str += getSipString(srcRegType);
    }
    else if (dispSrc) {
      if (!str.endsWith(" ")) str += " ";
      str += getDispString();
    }
    else if (immSrc) {
      if (!str.endsWith(" ")) str += " ";
      str += getImmString();
    }
    return str;
  }

  void Instruction::reverse() {
    qSwap<unsigned char>(srcReg, dstReg);
    qSwap<bool>(srcRegSet, dstRegSet);
    qSwap<RegType>(srcRegType, dstRegType);
    qSwap<bool>(sipSrc, sipDst);
    qSwap<bool>(dispSrc, dispDst);
    qSwap<bool>(immSrc, immDst);
  }

  QString Instruction::getRegString(int reg, RegType type,
                                    RegType type2) const {
    if (reg < 0 || reg > 7) {
      return QString();
    }
    static QString regs[6][8] = {{"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"},
                                 {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"},
                                 {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"},
                                 {"mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"},
                                 {"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"},
                                 {"es", "cs", "ss", "ds", "fs", "gs", "reserved", "reserved"}};
    QString res = "%" + regs[(int) type][reg];

    // If opposite type is less than then mark as address reference.
    if ((int) type > (int) type2) {
      res = "(" + res + ")";
    }
    return res;
  }

  QString Instruction::getSipString(RegType type) const {
    QString sip{"("};
    sip += getRegString(base, type);
    if (index != 4) {
      sip += "," + getRegString(index, type) +
        "," + QString::number(pow(2, scale));
    }
    return sip + ")";
  }

  QString Instruction::getDispString() const {
    return formatHex(disp + offset, dispBytes * 2);
  }

  QString Instruction::getImmString() const {
    return "$" + formatHex(imm + offset, immBytes * 2);
  }
  
  QString Instruction::formatHex(quint32 num, int len) const {
    return "0x" + Util::padString(QString::number(num, 16), len);
  }
}

AsmX86::AsmX86(BinaryObjectPtr obj) : obj{obj}, reader{nullptr} { }

bool AsmX86::disassemble(SectionPtr sec, Disassembly &result) {
  QBuffer buf;
  buf.setData(sec->getData());
  buf.open(QIODevice::ReadOnly);
  reader.reset(new Reader(buf));

  // Address of main()
  quint64 funcAddr = sec->getAddress();

  bool ok{true}, peek{false};
  unsigned char ch, nch;
  qint64 pos{0};
  while (!reader->atEnd()) {
    pos = reader->pos();
    ch = reader->getUChar(&ok);
    if (!ok) return false;

    nch = reader->peekUChar(&peek);

    // PUSH
    if (ch >= 0x50 && ch <= 0x57) {
      Instruction inst;
      inst.mnemonic = "pushl";
      inst.dstReg = getR(ch);
      inst.dstRegType = RegType::R32;
      inst.dstRegSet = true;
      addResult(inst, pos, result);
    }

    // POP
    else if (ch >= 0x58 && ch <= 0x60) {
      Instruction inst;
      inst.mnemonic = "popl";
      inst.dstReg = getR(ch);
      inst.dstRegType = RegType::R32;
      inst.dstRegSet = true;
      addResult(inst, pos, result);
    }

    // ADD, OR, ADC, SBB, AND, SUB, XOR, CMP
    // (r/m16/32	imm16/32)
    else if (ch == 0x81 && peek) {
      Instruction inst;
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);

      inst.immDst = true;
      processImm32(inst);

      // Don't display the 'src' after the 'dst'.
      inst.srcRegSet = false;

      if (inst.srcReg == 0) {
        inst.mnemonic = "addl";
      }
      else if (inst.srcReg == 1) {
        inst.mnemonic = "orl";
      }
      else if (inst.srcReg == 2) {
        inst.mnemonic = "adcl"; // Add with carry
      }
      else if (inst.srcReg == 3) {
        inst.mnemonic = "sbbl"; // Integer subtraction with borrow
      }
      else if (inst.srcReg == 4) {
        inst.mnemonic = "andl";
      }
      else if (inst.srcReg == 5) {
        inst.mnemonic = "subl";
      }
      else if (inst.srcReg == 6) {
        inst.mnemonic = "xorl";
      }
      else if (inst.srcReg == 7) {
        inst.mnemonic = "cmpl";
      }

      addResult(inst, pos, result);
    }

    // ADD, OR, ADC, SBB, AND, SUB, XOR, CMP
    // (r/m16/32	imm8)
    else if (ch == 0x83 && peek) {
      Instruction inst;
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);

      inst.immDst = true;
      processImm8(inst);

      // Don't display the 'src' after the 'dst'.
      inst.srcRegSet = false;

      if (inst.srcReg == 0) {
        inst.mnemonic = "addl";
      }
      else if (inst.srcReg == 1) {
        inst.mnemonic = "orl";
      }
      else if (inst.srcReg == 2) {
        inst.mnemonic = "adcl"; // Add with carry
      }
      else if (inst.srcReg == 3) {
        inst.mnemonic = "sbbl"; // Integer subtraction with borrow
      }
      else if (inst.srcReg == 4) {
        inst.mnemonic = "andl";
      }
      else if (inst.srcReg == 5) {
        inst.mnemonic = "subl";
      }
      else if (inst.srcReg == 6) {
        inst.mnemonic = "xorl";
      }
      else if (inst.srcReg == 7) {
        inst.mnemonic = "cmpl";
      }

      addResult(inst, pos, result);
    }

    // MOV (r/m8	r8) (reverse of 0x8A)
    else if (ch == 0x88 && peek) {
      Instruction inst;
      inst.mnemonic = "movb";
      inst.srcRegType = RegType::R8;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);
      inst.reverse();
      addResult(inst, pos, result);
    }

    // MOV (r8  r/m8)
    else if (ch == 0x8A && peek) {
      Instruction inst;
      inst.mnemonic = "movb";
      inst.srcRegType = RegType::R8;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);
      addResult(inst, pos, result);
    }

    // MOV (r16/32	r/m16/32)
    else if (ch == 0x8B && peek) {
      Instruction inst;
      inst.mnemonic = "movl";
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);
      addResult(inst, pos, result);
    }

    // LEA (r16/32	m) Load Effective Address
    else if (ch == 0x8D && peek) {
      Instruction inst;
      inst.mnemonic = "leal";
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);
      addResult(inst, pos, result);
      // TODO: was reversed before(?)
    }

    // MOV (r/m16/32  r16/32) (reverse of 0x8B)
    else if (ch == 0x89 && peek) {
      Instruction inst;
      inst.mnemonic = "movl";
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);
      inst.reverse();
      addResult(inst, pos, result);
    }

    // NOP
    else if (ch == 0x90) {
      addResult("nop", pos, result);
    }

    // RETN
    else if (ch == 0xC3) {
      addResult("ret", pos, result);
    }

    // MOV (r/m16/32	imm16/32)
    else if (ch == 0xC7 && peek) {
      Instruction inst;
      inst.mnemonic = "movl";
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);
      inst.reverse();

      inst.immDst = true;
      inst.dstRegSet = false;
      processImm32(inst);

      addResult(inst, pos, result);
    }

    // Call (relative function address)
    else if (ch == 0xE8) {
      Instruction inst;
      inst.mnemonic = "calll";
      inst.disp = reader->getUInt32();
      inst.dispBytes = 4;
      inst.dispSrc = true;
      inst.offset = funcAddr + reader->pos();
      addResult(inst, pos, result);
    }

    // JMP (rel16/32) (relative address)
    else if (ch == 0xE9) {
      Instruction inst;
      inst.mnemonic = "jmp";
      inst.disp = reader->getUInt32();
      inst.dispBytes = 4;
      inst.dispSrc = true;
      inst.offset = funcAddr + reader->pos();
      addResult(inst, pos, result);
    }

    // INC, DEC, CALL, CALLF, JMP, JMPF, PUSH
    else if (ch == 0xFF && peek) {
      Instruction inst;
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);

      // Don't display the 'src' after the 'dst'.
      inst.srcRegSet = false;

      if (inst.srcReg == 0) {
        inst.mnemonic = "inc ";
      }
      else if (inst.srcReg == 1) {
        inst.mnemonic = "dec ";
      }
      else if (inst.srcReg == 2) {
        inst.mnemonic = "call *";
      }
      else if (inst.srcReg == 3) {
        inst.mnemonic = "callf ";
      }
      else if (inst.srcReg == 4) {
        inst.mnemonic = "jmp ";
      }
      else if (inst.srcReg == 5) {
        inst.mnemonic = "jmpf ";
      }
      else if (inst.srcReg == 6) {
        inst.mnemonic = "pushl ";
      }

      addResult(inst, pos, result);
    }

    // Unsupported
    else {
      addResult("Unsupported: " + QString::number(ch, 16).toUpper(),
                pos, result);
    }
  }

  return !result.asmLines.isEmpty();
}


void AsmX86::addResult(const Instruction &inst, qint64 pos,
                       Disassembly &result) {
  addResult(inst.toString(), pos, result);
}

void AsmX86::addResult(const QString &line, qint64 pos,
                       Disassembly &result) {
  result.asmLines << line;
  result.bytesConsumed << reader->pos() - pos;
}

void AsmX86::splitByte(unsigned char num, unsigned char &mod, unsigned char &op1,
                       unsigned char &op2) {
  mod = (num & 0xC0) >> 6; // 2 first bits
  op1 = (num & 0x38) >> 3; // 3 next bits
  op2 = num & 0x7; // 3 last bits  
}

unsigned char AsmX86::getR(unsigned char num) {
  return num & 0x7; // 3 last bits  
}

void AsmX86::processModRegRM(Instruction &inst) {
  unsigned char ch = reader->getUChar();

  unsigned char mod, op1, op2;
  splitByte(ch, mod, op1, op2);

  // [reg]
  if (mod == 0) {
    if (op1 == 4) {
      inst.sipSrc = true;
      processSip(inst);
    }
    else if (op1 == 5) {
      inst.dispSrc = true;
    }
    else {
      inst.srcReg = op1;
      inst.srcRegSet = true;
    }
    if (op2 == 4) {
      inst.sipDst = true;
      processSip(inst);
    }
    else if (op2 == 5) {
      inst.dispDst = true;
    }
    else {
      inst.dstReg = op2;
      inst.dstRegSet = true;
    }
    if (inst.dispSrc) {
      processDisp32(inst);
    }
    if (inst.dispDst) {
      processDisp32(inst);
    }
  }

  // [reg]+disp8
  else if (mod == 1) {
    if (op1 != 4) {
      inst.srcReg = op1;
      inst.srcRegSet = true;
    }
    else {
      inst.sipSrc = true;
      processSip(inst);
    }
    if (op2 != 4) {
      inst.dstReg = op2;
      inst.dstRegSet = true;
    }
    else {
      inst.sipDst = true;
      processSip(inst);
    }
    inst.dispDst = true;
    processDisp8(inst);
  }

  // [reg]+disp32
  else if (mod == 2) {
    if (op1 != 4) {
      inst.srcReg = op1;
      inst.srcRegSet = true;
    }
    else {
      inst.sipSrc = true;
      processSip(inst);
    }
    if (op2 != 4) {
      inst.dstReg = op2;
      inst.dstRegSet = true;
    }
    else {
      inst.sipDst = true;
      processSip(inst);
    }
    inst.dispDst = true;
    processDisp32(inst);
  }

  // [reg]
  else if (mod == 3) {
    inst.srcReg = op1;
    inst.srcRegSet = true;
    inst.dstReg = op2;
    inst.dstRegSet = true;
  }
}

void AsmX86::processSip(Instruction &inst) {
  unsigned char sip = reader->getUChar();
  splitByte(sip, inst.scale, inst.index, inst.base);
  /*
  qDebug() << "sip, scale=" << inst.scale
           << "index=" << inst.index
           << "base=" << inst.base;
  qDebug() << "src:" << inst.sipSrc;
  qDebug() << "dst:" << inst.sipDst;
  */
}

void AsmX86::processDisp8(Instruction &inst) {
  inst.disp = reader->getUChar();
  inst.dispBytes = 1;
  //qDebug() << "disp8:" << inst.disp;
}

void AsmX86::processDisp32(Instruction &inst) {
  inst.disp = reader->getUInt32();
  inst.dispBytes = 4;
  //qDebug() << "disp32:" << inst.disp;
}

void AsmX86::processImm8(Instruction &inst) {
  inst.imm = reader->getUChar();
  inst.immBytes = 1;
  //qDebug() << "imm8:" << inst.imm;
}

void AsmX86::processImm32(Instruction &inst) {
  inst.imm = reader->getUInt32();
  inst.immBytes = 4;
  //qDebug() << "imm8:" << inst.imm;
}
