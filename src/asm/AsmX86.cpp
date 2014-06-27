/**
 * References:
 *   http://www.read.seas.harvard.edu/~kohler/class/04f-aos/ref/i386.pdf
 *   http://ref.x86asm.net
 *
 * Uses the AT&T/GAS assembly syntax: Mnemonic Src Dst
 */

#include <QDebug>
#include <QBuffer>

#include <cmath>

#include "AsmX86.h"
#include "../Util.h"
#include "../Reader.h"
#include "../Section.h"

namespace {
  QString Instruction::toString(BinaryObjectPtr obj) const {
    QString str(mnemonic);

    // Annotate calls with symbols if present.
    if (call && dispDst) {
      str += " " + getDispString();
      const auto &symTable = obj->getSymbolTable();
      quint32 addr = disp + offset;
      QString name;
      if (symTable.getString(addr, name)) {
        str += " (" + name + ")";
      }
      return str;
    }

    bool comma{true};
    if (srcRegSet) {
      if (!str.endsWith(" ")) str += " ";
      if (immSrc) {
        str += getImmString() + ",";
      }
      if (dispSrc) {
        str += getDispString() + "(";
      }
      str += getRegString(srcReg, srcRegType,
                          // Using SREG because it's the greatest value.
                          dispSrc ? RegType::SREG : srcRegType);
      if (dispSrc) {
        str += ")";
      }
    }
    else if (sipSrc) {
      if (!str.endsWith(" ")) str += " ";
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
    else {
      comma = false;
    }

    if (dstRegSet) {
      if (!comma && !str.endsWith(" ")) {
        str += " ";
      }
      else {
        str += ",";
      }
      if (immDst) {
        str += getImmString() + ",";
      }
      if (dispDst) {
        str += getDispString() + "(";
      }
      str += getRegString(dstReg, dstRegType,
                          dispDst ? RegType::SREG : dstRegType);
      if (dispDst) {
        str += ")";
      }
    }
    else if (sipDst) {
      if (!comma && !str.endsWith(" ")) {
        str += " ";
      }
      else {
        str += ",";
      }
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
    return "0x" + Util::padString(QString::number(num, 16).toUpper(), len);
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
    // Handle special NOP sequences.
    if (handleNops(result)) {
      continue;
    }

    pos = reader->pos();
    ch = reader->getUChar(&ok);
    if (!ok) return false;

    nch = reader->peekUChar(&peek);

    // ADD (r/m16/32  r16/32) (reverse of 0x03)
    if (ch == 0x01 && peek) {
      Instruction inst;
      inst.mnemonic = "addl";
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);
      inst.reverse();
      addResult(inst, pos, result);
    }

    // ADD (r16/32  r/m16/32)
    else if (ch == 0x03 && peek) {
      Instruction inst;
      inst.mnemonic = "addl";
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);
      addResult(inst, pos, result);
    }

    // ADD (eAX  imm16/32)
    else if (ch == 0x05) {
      Instruction inst;
      inst.mnemonic = "addl";
      inst.imm = reader->getUInt32();
      inst.immBytes = 4;
      inst.immSrc = true;

      // Dst is always %eax.
      inst.dstReg = 0;
      inst.dstRegSet = true;
      inst.dstRegType = RegType::R32;
      inst.srcRegType = RegType::R32;

      addResult(inst, pos, result);
    }

    // OR (r/m16/32  r16/32)
    else if (ch == 0x09) {
      Instruction inst;
      inst.mnemonic = "orl";
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);
      inst.reverse();
      addResult(inst, pos, result);
    }

    // OR (eAX  imm16/32)
    else if (ch == 0x0D) {
      Instruction inst;
      inst.mnemonic = "orl";
      inst.imm = reader->getUInt32();
      inst.immBytes = 4;
      inst.immSrc = true;

      // Dst is always %eax.
      inst.dstReg = 0;
      inst.dstRegSet = true;
      inst.dstRegType = RegType::R32;
      inst.srcRegType = RegType::R32;

      addResult(inst, pos, result);
    }

    // AND (r16/32  r/m16/32)
    else if (ch == 0x23 && peek) {
      Instruction inst;
      inst.mnemonic = "andl";
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);
      addResult(inst, pos, result);
    }

    // AND (eAX  imm16/32)
    else if (ch == 0x25) {
      Instruction inst;
      inst.mnemonic = "andl";
      inst.imm = reader->getUInt32();
      inst.immBytes = 4;
      inst.immSrc = true;

      // Dst is always %eax.
      inst.dstReg = 0;
      inst.dstRegSet = true;
      inst.dstRegType = RegType::R32;
      inst.srcRegType = RegType::R32;

      addResult(inst, pos, result);
    }

    // SUB (r/m16/32  r16/32)
    else if (ch == 0x29 && peek) {
      Instruction inst;
      inst.mnemonic = "subl";
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);
      inst.reverse();
      addResult(inst, pos, result);
    }

    // SUB (eAX  imm16/32)
    else if (ch == 0x2D) {
      Instruction inst;
      inst.mnemonic = "subl";
      inst.imm = reader->getUInt32();
      inst.immBytes = 4;
      inst.immSrc = true;

      // Dst is always %eax.
      inst.dstReg = 0;
      inst.dstRegSet = true;
      inst.dstRegType = RegType::R32;
      inst.srcRegType = RegType::R32;

      addResult(inst, pos, result);
    }

    // CMP (r16/32 r/m16/32)
    else if (ch == 0x3B && peek) {
      Instruction inst;
      inst.mnemonic = "cmpl";
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);
      addResult(inst, pos, result);
    }

    // CMP (eAX  imm16/32)
    else if (ch == 0x3D) {
      Instruction inst;
      inst.mnemonic = "cmpl";
      inst.imm = reader->getUInt32();
      inst.immBytes = 4;
      inst.immSrc = true;

      // Dst is always %eax.
      inst.dstReg = 0;
      inst.dstRegSet = true;
      inst.dstRegType = RegType::R32;
      inst.srcRegType = RegType::R32;

      addResult(inst, pos, result);
    }

    // PUSH (r16/32)
    else if (ch >= 0x50 && ch <= 0x57) {
      Instruction inst;
      inst.mnemonic = "pushl";
      inst.srcReg = getR(ch);
      inst.srcRegType = RegType::R32;
      inst.srcRegSet = true;
      addResult(inst, pos, result);
    }

    // POP (r16/32)
    else if (ch >= 0x58 && ch <= 0x5F) {
      Instruction inst;
      inst.mnemonic = "popl";
      inst.srcReg = getR(ch);
      inst.srcRegType = RegType::R32;
      inst.srcRegSet = true;
      addResult(inst, pos, result);
    }

    // ADD, OR, ADC, SBB, AND, SUB, XOR, CMP
    // (r/m8  imm8)
    else if (ch == 0x80 && peek) {
      Instruction inst;
      inst.srcRegType = RegType::R8;
      inst.dstRegType = RegType::R8;
      processModRegRM(inst);

      inst.immSrc = true;
      processImm8(inst);

      // Don't display the 'dst' after the 'src'.
      inst.dstRegSet = false;

      if (inst.dstReg == 0) {
        inst.mnemonic = "addb";
      }
      else if (inst.dstReg == 1) {
        inst.mnemonic = "orb";
      }
      else if (inst.dstReg == 2) {
        inst.mnemonic = "adcb"; // Add with carry
      }
      else if (inst.dstReg == 3) {
        inst.mnemonic = "sbbb"; // Integer subtraction with borrow
      }
      else if (inst.dstReg == 4) {
        inst.mnemonic = "andb";
      }
      else if (inst.dstReg == 5) {
        inst.mnemonic = "subb";
      }
      else if (inst.dstReg == 6) {
        inst.mnemonic = "xorb";
      }
      else if (inst.dstReg == 7) {
        inst.mnemonic = "cmpb";
      }

      addResult(inst, pos, result);
    }

    // ADD, OR, ADC, SBB, AND, SUB, XOR, CMP
    // (r/m16/32	imm16/32)
    else if (ch == 0x81 && peek) {
      Instruction inst;
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);

      inst.immSrc = true;
      processImm32(inst);

      // Don't display the 'dst' after the 'src'.
      inst.dstRegSet = false;

      if (inst.dstReg == 0) {
        inst.mnemonic = "addl";
      }
      else if (inst.dstReg == 1) {
        inst.mnemonic = "orl";
      }
      else if (inst.dstReg == 2) {
        inst.mnemonic = "adcl"; // Add with carry
      }
      else if (inst.dstReg == 3) {
        inst.mnemonic = "sbbl"; // Integer subtraction with borrow
      }
      else if (inst.dstReg == 4) {
        inst.mnemonic = "andl";
      }
      else if (inst.dstReg == 5) {
        inst.mnemonic = "subl";
      }
      else if (inst.dstReg == 6) {
        inst.mnemonic = "xorl";
      }
      else if (inst.dstReg == 7) {
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

      inst.immSrc = true;
      processImm8(inst);

      // Don't display the 'dst' after the 'src'.
      inst.dstRegSet = false;

      if (inst.dstReg == 0) {
        inst.mnemonic = "addl";
      }
      else if (inst.dstReg == 1) {
        inst.mnemonic = "orl";
      }
      else if (inst.dstReg == 2) {
        inst.mnemonic = "adcl"; // Add with carry
      }
      else if (inst.dstReg == 3) {
        inst.mnemonic = "sbbl"; // Integer subtraction with borrow
      }
      else if (inst.dstReg == 4) {
        inst.mnemonic = "andl";
      }
      else if (inst.dstReg == 5) {
        inst.mnemonic = "subl";
      }
      else if (inst.dstReg == 6) {
        inst.mnemonic = "xorl";
      }
      else if (inst.dstReg == 7) {
        inst.mnemonic = "cmpl";
      }

      addResult(inst, pos, result);
    }

    // TEST (r/m16/32  r16/32)
    else if (ch == 0x85 && peek) {
      Instruction inst;
      inst.mnemonic = "testl";
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);
      inst.reverse();
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

    // TEST	(AL	imm8)
    else if (ch == 0xA8 && peek) {
      Instruction inst;
      inst.mnemonic = "testb";
      inst.disp = reader->getUChar();
      inst.dispBytes = 1;
      inst.dispSrc = true;

      // Dst is always %al.
      inst.dstReg = 0;
      inst.dstRegSet = true;
      inst.dstRegType = RegType::R8;
      inst.srcRegType = RegType::R8;

      addResult(inst, pos, result);
    }

    // MOV (r8  imm8)
    else if (ch >= 0xB0 && ch <= 0xB7) {
      Instruction inst;
      inst.mnemonic = "movb";
      inst.srcReg = getR(ch);
      inst.srcRegType = RegType::R8;
      inst.srcRegSet = true;

      inst.immSrc = true;
      processImm8(inst);

      addResult(inst, pos, result);
    }

    // MOV (r16/32  imm16/32)
    else if (ch >= 0xB8 && ch <= 0xBF) {
      Instruction inst;
      inst.mnemonic = "movl";
      inst.srcReg = getR(ch);
      inst.srcRegType = RegType::R32;
      inst.srcRegSet = true;

      inst.immSrc = true;
      processImm32(inst);

      addResult(inst, pos, result);
    }

    // ROL, ROR, RCL, RCR, SHL/SAL, SHR, SAL/SHL, SAR
    // (r/m16/32  imm8)
    else if (ch == 0xC1) {
      Instruction inst;
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);

      inst.immSrc = true;
      processImm8(inst);

      // Don't display the 'dst' after the 'src'.
      inst.dstRegSet = false;

      if (inst.dstReg == 0) {
        inst.mnemonic = "roll";
      }
      else if (inst.dstReg == 1) {
        inst.mnemonic = "rorl";
      }
      else if (inst.dstReg == 2) {
        inst.mnemonic = "rcll";
      }
      else if (inst.dstReg == 3) {
        inst.mnemonic = "rcrl";
      }
      else if (inst.dstReg == 4) {
        inst.mnemonic = "shll";
      }
      else if (inst.dstReg == 5) {
        inst.mnemonic = "shrl";
      }
      else if (inst.dstReg == 6) {
        inst.mnemonic = "sall";
      }
      else if (inst.dstReg == 7) {
        inst.mnemonic = "sarl";
      }

      addResult(inst, pos, result);
    }

    // RETN
    else if (ch == 0xC3) {
      addResult("ret", pos, result);
    }

    // MOV (r/m8  imm8)
    else if (ch == 0xC6 && peek) {
      Instruction inst;
      inst.mnemonic = "movb";
      inst.srcRegType = RegType::R8;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);
      inst.reverse();

      inst.immSrc = true;
      inst.srcRegSet = false;
      processImm8(inst);

      addResult(inst, pos, result);
    }

    // MOV (r/m16/32	imm16/32)
    else if (ch == 0xC7 && peek) {
      Instruction inst;
      inst.mnemonic = "movl";
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);
      inst.reverse();

      inst.immSrc = true;
      inst.srcRegSet = false;
      processImm32(inst);

      addResult(inst, pos, result);
    }

    // Call (relative function address)
    else if (ch == 0xE8) {
      Instruction inst;
      inst.mnemonic = "calll";
      inst.disp = reader->getUInt32();
      inst.dispBytes = 4;
      inst.dispDst = true;
      inst.offset = funcAddr + reader->pos();
      inst.call = true;
      addResult(inst, pos, result);
    }

    // JMP (rel16/32) (relative address)
    else if (ch == 0xE9) {
      Instruction inst;
      inst.mnemonic = "jmp";
      inst.disp = reader->getUInt32();
      inst.dispBytes = 4;
      inst.dispDst = true;
      inst.offset = funcAddr + reader->pos();
      addResult(inst, pos, result);
    }

    // INC, DEC, CALL, CALLF, JMP, JMPF, PUSH
    else if (ch == 0xFF && peek) {
      Instruction inst;
      inst.srcRegType = RegType::R32;
      inst.dstRegType = RegType::R32;
      processModRegRM(inst);

      // Don't display the 'dst' after the 'src'.
      inst.dstRegSet = false;

      if (inst.dstReg == 0) {
        inst.mnemonic = "inc ";
      }
      else if (inst.dstReg == 1) {
        inst.mnemonic = "dec ";
      }
      else if (inst.dstReg == 2) {
        inst.mnemonic = "call *";
        inst.call = true;
      }
      else if (inst.dstReg == 3) {
        inst.mnemonic = "callf ";
        inst.call = true;
      }
      else if (inst.dstReg == 4) {
        inst.mnemonic = "jmp ";
      }
      else if (inst.dstReg == 5) {
        inst.mnemonic = "jmpf ";
      }
      else if (inst.dstReg == 6) {
        inst.mnemonic = "pushl ";
      }

      addResult(inst, pos, result);
    }

    // Two-byte instructions.
    else if (ch == 0x0F && peek) {
      ch = nch;
      reader->getUChar(); // eat

      nch = reader->peekUChar(&peek);

      // NOP (r/m16/32)
      if (ch == 0x1F && peek) {
        Instruction inst;
        inst.mnemonic = "nopl";
        inst.srcRegType = RegType::R32;
        inst.dstRegType = RegType::R32;
        processModRegRM(inst);
        inst.dstRegSet = false;
        addResult(inst, pos, result);
      }

      // JNB (rel16/32), JAE (rel16/32), or JNC (rel16/32).
      else if (ch == 0x83) {
        Instruction inst;
        inst.mnemonic = "jae";
        inst.disp = reader->getUInt32();
        inst.dispBytes = 4;
        inst.dispDst = true;
        inst.offset = funcAddr + reader->pos();
        addResult(inst, pos, result);
      }

      // JZ (rel16/32) or JE (rel16/32), same functionality different
      // name. Relative function address.
      else if (ch == 0x84) {
        Instruction inst;
        inst.mnemonic = "je";
        inst.disp = reader->getUInt32();
        inst.dispBytes = 4;
        inst.dispDst = true;
        inst.offset = funcAddr + reader->pos();
        addResult(inst, pos, result);
      }

      // JNZ (rel16/32) or JNE (rel16/32), same functionality
      // different name. Relative function address.
      else if (ch == 0x85) {
        Instruction inst;
        inst.mnemonic = "jne";
        inst.disp = reader->getUInt32();
        inst.dispBytes = 4;
        inst.dispDst = true;
        inst.offset = funcAddr + reader->pos();
        addResult(inst, pos, result);
      }

      // JNL (rel16/32) or JGE (rel16/32).
      else if (ch == 0x8D) {
        Instruction inst;
        inst.mnemonic = "jge";
        inst.disp = reader->getUInt32();
        inst.dispBytes = 4;
        inst.dispDst = true;
        inst.offset = funcAddr + reader->pos();
        addResult(inst, pos, result);
      }

      // JLE (rel16/32) or JNG (rel16/32), same functionality
      // different name. Relative function address.
      else if (ch == 0x8E) {
        Instruction inst;
        inst.mnemonic = "jle";
        inst.disp = reader->getUInt32();
        inst.dispBytes = 4;
        inst.dispDst = true;
        inst.offset = funcAddr + reader->pos();
        addResult(inst, pos, result);
      }

      // JNLE (rel16/32) or JG (rel16/32).
      else if (ch == 0x8F) {
        Instruction inst;
        inst.mnemonic = "jg";
        inst.disp = reader->getUInt32();
        inst.dispBytes = 4;
        inst.dispDst = true;
        inst.offset = funcAddr + reader->pos();
        addResult(inst, pos, result);
      }

      // SETZ (r/m8) or SETE (r/m8)
      else if (ch == 0x94 && peek) {
        Instruction inst;
        inst.mnemonic = "sete";
        inst.srcRegType = RegType::R8;
        processModRegRM(inst);
        inst.dstRegSet = false;
        addResult(inst, pos, result);
      }

      // SETNZ (r/m8) or SETNE (r/m8).
      else if (ch == 0x95 && peek) {
        Instruction inst;
        inst.mnemonic = "setne";
        inst.srcRegType = RegType::R8;
        processModRegRM(inst);
        inst.dstRegSet = false;
        addResult(inst, pos, result);
      }

      // MOVZX (r16/32 r/m8)
      // Move with Zero-Extension
      else if (ch == 0xB6) {
        Instruction inst;
        inst.mnemonic = "movzbl";
        inst.srcRegType = RegType::R32;
        inst.dstRegType = RegType::R32;
        processModRegRM(inst);
        addResult(inst, pos, result);
      }

      // MOVSX (r16/32 r/m8)
      // Move with Sign-Extension
      else if (ch == 0xBE && peek) {
        Instruction inst;
        inst.mnemonic = "movsbl";
        inst.srcRegType = RegType::R32;
        inst.dstRegType = RegType::R32;
        processModRegRM(inst);
        addResult(inst, pos, result);
      }
    }

    // Unsupported
    else {
      addResult("Unsupported: " + QString::number(ch, 16).toUpper(),
                pos, result);
    }
  }

  return !result.asmLines.isEmpty();
}

bool AsmX86::handleNops(Disassembly &result) {
  qint64 pos = reader->pos();

  // Eat any 0x66's but leave one for the matching beneath.
  while (reader->peekList({0x66, 0x66})) {
    reader->read(1);
  }

  if (reader->peekList({0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00})) {
    reader->read(10);
    addResult("nopw %cs:0L(%eax,%eax,1)", pos, result);
  }
  else if (reader->peekList({0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00})) {
    reader->read(9);
    addResult("nopw 0L(%eax,%eax,1)", pos, result);
  }
  else if (reader->peekList({0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00})) {
    reader->read(8);
    addResult("nopl 0L(%eax,%eax,1)", pos, result);
  }
  else if (reader->peekList({0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00})) {
    reader->read(7);
    addResult("nopl 0L(%eax)", pos, result);
  }
  else if (reader->peekList({0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00})) {
    reader->read(6);
    addResult("nopw 0x0(%eax,%eax,1)", pos, result);
  }
  else if (reader->peekList({0x0f, 0x1f, 0x44, 0x00, 0x00})) {
    reader->read(5);
    addResult("nopl 0x0(%eax,%eax,1)", pos, result);
  }
  else if (reader->peekList({0x0f, 0x1f, 0x00})) {
    reader->read(3);
    addResult("nopl 0x0(%eax)", pos, result);
  }
  else if (reader->peekList({0x66, 0x90})) {
    reader->read(2);
    addResult("xchg %ax,%ax", pos, result);
  }
  else {
    return false;
  }
  return true;
}

void AsmX86::addResult(const Instruction &inst, qint64 pos,
                       Disassembly &result) {
  addResult(inst.toString(obj), pos, result);
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
      inst.sipDst = true;
      processSip(inst);
    }
    else if (op1 == 5) {
      inst.dispDst = true;
    }
    else {
      inst.dstReg = op1;
      inst.dstRegSet = true;
    }
    if (op2 == 4) {
      inst.sipSrc = true;
      processSip(inst);
    }
    else if (op2 == 5) {
      inst.dispSrc = true;
    }
    else {
      inst.srcReg = op2;
      inst.srcRegSet = true;
    }
    if (inst.dispDst) {
      processDisp32(inst);
    }
    if (inst.dispSrc) {
      processDisp32(inst);
    }
  }

  // [reg]+disp8
  else if (mod == 1) {
    if (op1 != 4) {
      inst.dstReg = op1;
      inst.dstRegSet = true;
    }
    else {
      inst.sipDst = true;
      processSip(inst);
    }
    if (op2 != 4) {
      inst.srcReg = op2;
      inst.srcRegSet = true;
    }
    else {
      inst.sipSrc = true;
      processSip(inst);
    }
    inst.dispSrc = true;
    processDisp8(inst);
  }

  // [reg]+disp32
  else if (mod == 2) {
    if (op1 != 4) {
      inst.dstReg = op1;
      inst.dstRegSet = true;
    }
    else {
      inst.sipDst = true;
      processSip(inst);
    }
    if (op2 != 4) {
      inst.srcReg = op2;
      inst.srcRegSet = true;
    }
    else {
      inst.sipSrc = true;
      processSip(inst);
    }
    inst.dispSrc = true;
    processDisp32(inst);
  }

  // [reg]
  else if (mod == 3) {
    inst.dstReg = op1;
    inst.dstRegSet = true;
    inst.srcReg = op2;
    inst.srcRegSet = true;
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
