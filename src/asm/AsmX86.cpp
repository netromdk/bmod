/**
 * X86 and X86_64 disassembling.
 *
 * Uses the AT&T/GAS assembly syntax: Mnemonic Src Dst
 *
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
  QString Instruction::toString(BinaryObjectPtr obj) const {
    QString str(mnemonic);
    switch (dataType) {
    case DataType::None:
      // Nothing.
      break;

    case DataType::Byte:
      str += "b";
      break;

    case DataType::Word:
      str += "w";
      break;

    case DataType::Doubleword:
      str += "l";
      break;

    case DataType::Quadword:
      str += "q";
      break;
    }

    // Annotate calls with symbols if present.
    if (call && dispDst) {
      str += " " + getDispString();
      quint64 addr = disp + offset;
      const auto &symTable = obj->getSymbolTable();
      const auto &dynsymTable = obj->getDynSymbolTable();
      QString name;
      if (symTable.getString(addr, name) || dynsymTable.getString(addr, name)) {
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
    if (reg < 0 || reg > 17) {
      return QString();
    }
    static QString regs[8][17] =
      { // R8 without REX prefix
        {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh",
         "inva", "inva", "inva", "inva", "inva", "inva", "inva", "inva", "inva"},

        // R8R with any REX prefix
        {"al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil",
         "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b", // REX.R=1
         "inva"},

        // R16
        {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di",
         "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w", // REX.R=1
         "inva"},

        // R32
        {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
         "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d", // REX.R=1
         "eip"},

        // R64
        {"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
         "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", // REX.R=1
         "rip"},

        // MM
        {"mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7",
         "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7", // REX.R=1
         "inva"},

        // XMM
        {"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
         "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", // REX.R=1
         "inva"},

        // SREG
        {"es", "cs", "ss", "ds", "fs", "gs", "resv", "resv",
         "es", "cs", "ss", "ds", "fs", "gs", "resv", "resv", // REX.R=1
         "inva"}};
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
  
  QString Instruction::formatHex(quint64 num, int len) const {
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
  Instruction inst;
  const bool _64 = (obj->getSystemBits() == 64);
  while (!reader->atEnd()) {
    // Handle special NOP sequences.
    if (handleNops(result)) {
      continue;
    }

    pos = reader->pos();
    ch = reader->getUChar(&ok);
    if (!ok) return false;

    nch = reader->peekUChar(&peek);

    // Instruction to fill.
    inst = Instruction();
    inst.dataType = DataType::Doubleword;
    inst.srcRegType = inst.dstRegType = (_64 ? RegType::R64 : RegType::R32);

    // REX mode (64-bit ONLY!).
    if (_64 && ch >= 0x40 && ch <= 0x4F) {
      // W=1 => 64-bit
      // R=1 => Extension to Mod-R/M: reg
      // X=1 => Extension to SIB index
      // B=1 => Extension to Mod-R/M: R/M oR SIB base
      splitRex(ch, inst.rexW, inst.rexR, inst.rexX, inst.rexB);
      if (inst.rexW) {
        inst.dataType = DataType::Quadword;
      }

      // Setup for next.
      ch = reader->getUChar(&ok);
      if (!ok) return false;

      nch = reader->peekUChar(&peek);
    }

    // ADD (r/m16/32  r16/32) (reverse of 0x03)
    if (ch == 0x01 && peek) {
      inst.mnemonic = "add";
      processModRegRM(inst);
      inst.reverse();
      addResult(inst, pos, result);
    }

    // ADD (r16/32  r/m16/32)
    else if (ch == 0x03 && peek) {
      inst.mnemonic = "add";
      processModRegRM(inst);
      addResult(inst, pos, result);
    }

    // ADD (eAX  imm16/32)
    else if (ch == 0x05) {
      inst.mnemonic = "add";
      inst.imm = reader->getUInt32();
      inst.immBytes = 4;
      inst.immSrc = true;

      // Dst is always %eax.
      inst.dstReg = 0;
      inst.dstRegSet = true;

      addResult(inst, pos, result);
    }

    // PUSH (imm8)
    else if (ch == 0x6A && peek) {
      inst.mnemonic = "push";
      if (_64) inst.dataType = DataType::Quadword;
      inst.immDst = true;
      inst.dstRegSet = false;
      processImm8(inst);
      addResult(inst, pos, result);
    }

    // OR (r/m16/32  r16/32)
    else if (ch == 0x09) {
      inst.mnemonic = "or";
      processModRegRM(inst);
      inst.reverse();
      addResult(inst, pos, result);
    }

    // OR (eAX  imm16/32)
    else if (ch == 0x0D) {
      inst.mnemonic = "or";
      inst.imm = reader->getUInt32();
      inst.immBytes = 4;
      inst.immSrc = true;

      // Dst is always %eax.
      inst.dstReg = 0;
      inst.dstRegSet = true;

      addResult(inst, pos, result);
    }

    // AND (r16/32  r/m16/32)
    else if (ch == 0x23 && peek) {
      inst.mnemonic = "and";
      processModRegRM(inst);
      addResult(inst, pos, result);
    }

    // AND (AL  imm8)
    else if (ch == 0x24) {
      inst.mnemonic = "and";
      inst.dataType = DataType::Byte;
      inst.srcRegType = inst.dstRegType = RegType::R8;
      inst.imm = reader->getUChar();
      inst.immBytes = 1;
      inst.immSrc = true;

      // Dst is always %al.
      inst.dstReg = 0;
      inst.dstRegSet = true;

      addResult(inst, pos, result);
    }

    // AND (eAX  imm16/32)
    else if (ch == 0x25) {
      inst.mnemonic = "and";
      inst.imm = reader->getUInt32();
      inst.immBytes = 4;
      inst.immSrc = true;

      // Dst is always %eax.
      inst.dstReg = 0;
      inst.dstRegSet = true;

      addResult(inst, pos, result);
    }

    // SUB (r/m16/32  r16/32)
    else if (ch == 0x29 && peek) {
      inst.mnemonic = "sub";
      processModRegRM(inst);
      inst.reverse();
      addResult(inst, pos, result);
    }

    // SUB (eAX  imm16/32)
    else if (ch == 0x2D) {
      inst.mnemonic = "sub";
      inst.imm = reader->getUInt32();
      inst.immBytes = 4;
      inst.immSrc = true;

      // Dst is always %eax.
      inst.dstReg = 0;
      inst.dstRegSet = true;

      addResult(inst, pos, result);
    }

    // XOR (r/m16/32/64  r16/32/64)
    else if (ch == 0x31 && peek) {
      inst.mnemonic = "xor";
      processModRegRM(inst);
      addResult(inst, pos, result);
    }

    // CMP (r16/32 r/m16/32)
    else if (ch == 0x3B && peek) {
      inst.mnemonic = "cmp";
      processModRegRM(inst);
      addResult(inst, pos, result);
    }

    // CMP (eAX  imm16/32)
    else if (ch == 0x3D) {
      inst.mnemonic = "cmp";
      inst.imm = reader->getUInt32();
      inst.immBytes = 4;
      inst.immSrc = true;

      // Dst is always %eax.
      inst.dstReg = 0;
      inst.dstRegSet = true;

      addResult(inst, pos, result);
    }

    // PUSH (r16/32)
    else if (ch >= 0x50 && ch <= 0x57) {
      inst.mnemonic = "push";
      if (_64) inst.dataType = DataType::Quadword;
      inst.srcReg = getR(ch);
      inst.srcRegSet = true;
      addResult(inst, pos, result);
    }

    // POP (r16/32)
    else if (ch >= 0x58 && ch <= 0x5F) {
      inst.mnemonic = "pop";
      if (_64) inst.dataType = DataType::Quadword;
      inst.srcReg = getR(ch);
      inst.srcRegSet = true;
      addResult(inst, pos, result);
    }

    // MOVSXD (r32/64  r/m32)
    // Move with Sign-Extension
    else if (ch == 0x63 && peek) {
      inst.mnemonic = "movsl";
      processModRegRM(inst);
      addResult(inst, pos, result);
    }

    // JNZ (rel8) or JNE (rel8)
    // Short jump
    else if (ch == 0x75 && peek) {
      inst.mnemonic = "jne";
      inst.disp = pos - (255 - (int) reader->getUChar()) + 1;
      inst.dispBytes = 1;
      inst.dispDst = true;
      inst.offset = funcAddr;
      inst.dataType = DataType::None;
      addResult(inst, pos, result);
    }

    // ADD, OR, ADC, SBB, AND, SUB, XOR, CMP
    // (r/m8  imm8)
    else if (ch == 0x80 && peek) {
      inst.dataType = DataType::Byte;
      inst.srcRegType = inst.dstRegType = RegType::R8;
      processModRegRM(inst);

      inst.immSrc = true;
      processImm8(inst);

      // Don't display the 'dst' after the 'src'.
      inst.dstRegSet = false;

      if (inst.dstReg == 0) {
        inst.mnemonic = "add";
      }
      else if (inst.dstReg == 1) {
        inst.mnemonic = "or";
      }
      else if (inst.dstReg == 2) {
        inst.mnemonic = "adc"; // Add with carry
      }
      else if (inst.dstReg == 3) {
        inst.mnemonic = "sbb"; // Integer subtraction with borrow
      }
      else if (inst.dstReg == 4) {
        inst.mnemonic = "and";
      }
      else if (inst.dstReg == 5) {
        inst.mnemonic = "sub";
      }
      else if (inst.dstReg == 6) {
        inst.mnemonic = "xor";
      }
      else if (inst.dstReg == 7) {
        inst.mnemonic = "cmp";
      }

      addResult(inst, pos, result);
    }

    // ADD, OR, ADC, SBB, AND, SUB, XOR, CMP
    // (r/m16/32	imm16/32)
    else if (ch == 0x81 && peek) {
      processModRegRM(inst);

      inst.immSrc = true;
      processImm32(inst);

      // Don't display the 'dst' after the 'src'.
      inst.dstRegSet = false;

      if (inst.dstReg == 0) {
        inst.mnemonic = "add";
      }
      else if (inst.dstReg == 1) {
        inst.mnemonic = "or";
      }
      else if (inst.dstReg == 2) {
        inst.mnemonic = "adc"; // Add with carry
      }
      else if (inst.dstReg == 3) {
        inst.mnemonic = "sbb"; // Integer subtraction with borrow
      }
      else if (inst.dstReg == 4) {
        inst.mnemonic = "and";
      }
      else if (inst.dstReg == 5) {
        inst.mnemonic = "sub";
      }
      else if (inst.dstReg == 6) {
        inst.mnemonic = "xor";
      }
      else if (inst.dstReg == 7) {
        inst.mnemonic = "cmp";
      }

      addResult(inst, pos, result);
    }

    // ADD, OR, ADC, SBB, AND, SUB, XOR, CMP
    // (r/m16/32	imm8)
    else if (ch == 0x83 && peek) {
      processModRegRM(inst);

      inst.immSrc = true;
      processImm8(inst);

      // Don't display the 'dst' after the 'src'.
      inst.dstRegSet = false;

      if (inst.dstReg == 0) {
        inst.mnemonic = "add";
      }
      else if (inst.dstReg == 1) {
        inst.mnemonic = "or";
      }
      else if (inst.dstReg == 2) {
        inst.mnemonic = "adc"; // Add with carry
      }
      else if (inst.dstReg == 3) {
        inst.mnemonic = "sbb"; // Integer subtraction with borrow
      }
      else if (inst.dstReg == 4) {
        inst.mnemonic = "and";
      }
      else if (inst.dstReg == 5) {
        inst.mnemonic = "sub";
      }
      else if (inst.dstReg == 6) {
        inst.mnemonic = "xor";
      }
      else if (inst.dstReg == 7) {
        inst.mnemonic = "cmp";
      }

      addResult(inst, pos, result);
    }

    // TEST (r/m16/32  r16/32)
    else if (ch == 0x85 && peek) {
      inst.mnemonic = "test";
      processModRegRM(inst);
      inst.reverse();
      addResult(inst, pos, result);
    }

    // MOV (r/m8	r8) (reverse of 0x8A)
    else if (ch == 0x88 && peek) {
      inst.mnemonic = "mov";
      inst.dataType = DataType::Byte;
      inst.srcRegType = inst.dstRegType = RegType::R8;
      processModRegRM(inst);
      inst.reverse();
      addResult(inst, pos, result);
    }

    // MOV (r8  r/m8)
    else if (ch == 0x8A && peek) {
      inst.mnemonic = "mov";
      inst.dataType = DataType::Byte;
      inst.srcRegType = inst.dstRegType = RegType::R8;
      processModRegRM(inst);
      addResult(inst, pos, result);
    }

    // MOV (r16/32	r/m16/32)
    else if (ch == 0x8B && peek) {
      inst.mnemonic = "mov";
      processModRegRM(inst);
      addResult(inst, pos, result);
    }

    // LEA (r16/32	m) Load Effective Address
    else if (ch == 0x8D && peek) {
      inst.mnemonic = "lea";
      processModRegRM(inst);
      addResult(inst, pos, result);
    }

    // MOV (r/m16/32  r16/32) (reverse of 0x8B)
    else if (ch == 0x89 && peek) {
      inst.mnemonic = "mov";
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
      inst.mnemonic = "mov";
      inst.dataType = DataType::Byte;
      inst.srcReg = getR(ch);
      inst.srcRegType = RegType::R8;
      inst.srcRegSet = true;

      inst.immSrc = true;
      processImm8(inst);

      addResult(inst, pos, result);
    }

    // MOV (r16/32  imm16/32)
    else if (ch >= 0xB8 && ch <= 0xBF) {
      inst.mnemonic = "mov";
      inst.srcReg = getR(ch);
      inst.srcRegSet = true;

      inst.immSrc = true;
      if (inst.dataType == DataType::Quadword) {
        processImm64(inst);
      }
      else {
        processImm32(inst);
      }

      addResult(inst, pos, result);
    }

    // ROL, ROR, RCL, RCR, SHL/SAL, SHR, SAL/SHL, SAR
    // (r/m16/32  imm8)
    else if (ch == 0xC1) {
      processModRegRM(inst);

      inst.immSrc = true;
      processImm8(inst);

      // Don't display the 'dst' after the 'src'.
      inst.dstRegSet = false;

      if (inst.dstReg == 0) {
        inst.mnemonic = "rol";
      }
      else if (inst.dstReg == 1) {
        inst.mnemonic = "ror";
      }
      else if (inst.dstReg == 2) {
        inst.mnemonic = "rcl";
      }
      else if (inst.dstReg == 3) {
        inst.mnemonic = "rcr";
      }
      else if (inst.dstReg == 4) {
        inst.mnemonic = "shl";
      }
      else if (inst.dstReg == 5) {
        inst.mnemonic = "shr";
      }
      else if (inst.dstReg == 6) {
        inst.mnemonic = "sal";
      }
      else if (inst.dstReg == 7) {
        inst.mnemonic = "sar";
      }

      addResult(inst, pos, result);
    }

    // RETN
    else if (ch == 0xC3) {
      addResult("ret", pos, result);
    }

    // MOV (r/m8  imm8)
    else if (ch == 0xC6 && peek) {
      inst.mnemonic = "mov";
      inst.dataType = DataType::Byte;
      inst.dstRegType = RegType::R8;
      processModRegRM(inst);
      inst.reverse();

      inst.immSrc = true;
      inst.srcRegSet = false;
      processImm8(inst);

      addResult(inst, pos, result);
    }

    // MOV (r/m16/32	imm16/32)
    else if (ch == 0xC7 && peek) {
      inst.mnemonic = "mov";
      processModRegRM(inst);
      inst.reverse();

      inst.immSrc = true;
      inst.srcRegSet = false;
      processImm32(inst);

      addResult(inst, pos, result);
    }

    // Call (relative function address)
    else if (ch == 0xE8) {
      inst.mnemonic = "call";
      inst.disp = reader->getUInt32();
      inst.dispBytes = 4;
      inst.dispDst = true;
      inst.offset = funcAddr + reader->pos();
      inst.call = true;
      if (_64) inst.dataType = DataType::Quadword;
      addResult(inst, pos, result);
    }

    // JMP (rel16/32) (relative address)
    else if (ch == 0xE9) {
      inst.mnemonic = "jmp";
      inst.dataType = DataType::None;
      inst.disp = reader->getUInt32();
      inst.dispBytes = 4;
      inst.dispDst = true;
      inst.offset = funcAddr + reader->pos();
      addResult(inst, pos, result);
    }

    // JMP (rel8)
    else if (ch == 0xEB && peek) {
      inst.mnemonic = "jmp";
      inst.dataType = DataType::None;
      inst.disp = reader->getUChar();
      inst.dispBytes = 1;
      inst.dispDst = true;
      inst.offset = funcAddr + reader->pos();
      addResult(inst, pos, result);
    }

    // HLT
    else if (ch == 0xF4) {
      addResult("hlt", pos, result);
    }

    // INC, DEC, CALL, CALLF, JMP, JMPF, PUSH
    else if (ch == 0xFF && peek) {
      processModRegRM(inst);

      // Don't display the 'dst' after the 'src'.
      inst.dstRegSet = false;

      if (inst.dstReg == 0) {
        inst.mnemonic = "inc";
        inst.dataType = DataType::None;
      }
      else if (inst.dstReg == 1) {
        inst.mnemonic = "dec";
        inst.dataType = DataType::None;
      }
      else if (inst.dstReg == 2) {
        inst.mnemonic = "call *";
        inst.call = true;
        inst.dataType = DataType::None;
      }
      else if (inst.dstReg == 3) {
        inst.mnemonic = "callf";
        inst.call = true;
        inst.dataType = DataType::None;
      }
      else if (inst.dstReg == 4) {
        inst.mnemonic = "jmp";
        inst.dataType = DataType::None;
      }
      else if (inst.dstReg == 5) {
        inst.mnemonic = "jmpf";
        inst.dataType = DataType::None;
      }
      else if (inst.dstReg == 6) {
        inst.mnemonic = "push";
        if (_64) inst.dataType = DataType::Quadword;
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
        inst.mnemonic = "nop";
        processModRegRM(inst);
        inst.dstRegSet = false;
        addResult(inst, pos, result);
      }

      // JNB (rel16/32), JAE (rel16/32), or JNC (rel16/32).
      else if (ch == 0x83) {
        inst.mnemonic = "jae";
        inst.disp = reader->getUInt32();
        inst.dispBytes = 4;
        inst.dispDst = true;
        inst.offset = funcAddr + reader->pos();
        inst.dataType = DataType::None;
        addResult(inst, pos, result);
      }

      // JZ (rel16/32) or JE (rel16/32), same functionality different
      // name. Relative function address.
      else if (ch == 0x84) {
        inst.mnemonic = "je";
        inst.disp = reader->getUInt32();
        inst.dispBytes = 4;
        inst.dispDst = true;
        inst.offset = funcAddr + reader->pos();
        inst.dataType = DataType::None;
        addResult(inst, pos, result);
      }

      // JNZ (rel16/32) or JNE (rel16/32), same functionality
      // different name. Relative function address.
      else if (ch == 0x85) {
        inst.mnemonic = "jne";
        inst.disp = reader->getUInt32();
        inst.dispBytes = 4;
        inst.dispDst = true;
        inst.offset = funcAddr + reader->pos();
        inst.dataType = DataType::None;
        addResult(inst, pos, result);
      }

      // JNL (rel16/32) or JGE (rel16/32).
      else if (ch == 0x8D) {
        inst.mnemonic = "jge";
        inst.disp = reader->getUInt32();
        inst.dispBytes = 4;
        inst.dispDst = true;
        inst.offset = funcAddr + reader->pos();
        inst.dataType = DataType::None;
        addResult(inst, pos, result);
      }

      // JLE (rel16/32) or JNG (rel16/32), same functionality
      // different name. Relative function address.
      else if (ch == 0x8E) {
        inst.mnemonic = "jle";
        inst.disp = reader->getUInt32();
        inst.dispBytes = 4;
        inst.dispDst = true;
        inst.offset = funcAddr + reader->pos();
        inst.dataType = DataType::None;
        addResult(inst, pos, result);
      }

      // JNLE (rel16/32) or JG (rel16/32).
      else if (ch == 0x8F) {
        inst.mnemonic = "jg";
        inst.disp = reader->getUInt32();
        inst.dispBytes = 4;
        inst.dispDst = true;
        inst.offset = funcAddr + reader->pos();
        inst.dataType = DataType::None;
        addResult(inst, pos, result);
      }

      // SETZ (r/m8) or SETE (r/m8)
      else if (ch == 0x94 && peek) {
        inst.mnemonic = "sete";
        inst.srcRegType = RegType::R8;
        processModRegRM(inst);
        inst.dstRegSet = false;
        addResult(inst, pos, result);
      }

      // SETNZ (r/m8) or SETNE (r/m8).
      else if (ch == 0x95 && peek) {
        inst.mnemonic = "setne";
        inst.srcRegType = RegType::R8;
        processModRegRM(inst);
        inst.dstRegSet = false;
        addResult(inst, pos, result);
      }

      // MOVZX (r16/32 r/m8)
      // Move with Zero-Extension
      else if (ch == 0xB6) {
        inst.mnemonic = "movzb";
        processModRegRM(inst);
        addResult(inst, pos, result);
      }

      // MOVSX (r16/32 r/m8)
      // Move with Sign-Extension
      else if (ch == 0xBE && peek) {
        inst.mnemonic = "movsb";
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

void AsmX86::splitRex(unsigned char num, bool &w, bool &r, bool &x, bool &b) {
  w = ((num & 0x8) >> 3 == 1);
  r = ((num & 0x4) >> 2 == 1);
  x = ((num & 0x2) >> 1 == 1);
  b = ((num & 0x1) == 1);
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
      inst.dstReg = 16; // RIP/EIP
      inst.dstRegSet = true;
    }
    else {
      inst.dstReg = op1 + (inst.rexR ? 8 : 0);
      inst.dstRegSet = true;
    }
    if (op2 == 4) {
      inst.sipSrc = true;
      processSip(inst);
    }
    else if (op2 == 5) {
      inst.dispSrc = true;
      inst.srcReg = 16; // RIP/EIP
      inst.srcRegSet = true;
    }
    else {
      inst.srcReg = op2 + (inst.rexB ? 8 : 0);
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
      inst.dstReg = op1 + (inst.rexR ? 8 : 0);
      inst.dstRegSet = true;
    }
    else {
      inst.sipDst = true;
      processSip(inst);
    }
    if (op2 != 4) {
      inst.srcReg = op2 + (inst.rexB ? 8 : 0);
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
      inst.dstReg = op1 + (inst.rexR ? 8 : 0);
      inst.dstRegSet = true;
    }
    else {
      inst.sipDst = true;
      processSip(inst);
    }
    if (op2 != 4) {
      inst.srcReg = op2 + (inst.rexB ? 8 : 0);
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
    inst.dstReg = op1 + (inst.rexR ? 8 : 0);
    inst.dstRegSet = true;
    inst.srcReg = op2 + (inst.rexB ? 8 : 0);
    inst.srcRegSet = true;
  }

  // Any REX prefix means R8 => R8R.
  if (inst.rexW || inst.rexR || inst.rexX || inst.rexB) {
    if (inst.dstRegSet && inst.dstRegType == RegType::R8) {
      inst.dstRegType = RegType::R8R;
    }
    if (inst.srcRegSet && inst.srcRegType == RegType::R8) {
      inst.srcRegType = RegType::R8R;
    }
  }
}

void AsmX86::processSip(Instruction &inst) {
  unsigned char sip = reader->getUChar();
  splitByte(sip, inst.scale, inst.index, inst.base);
  if (inst.rexB) {
    inst.base += 8;
  }
  if (inst.rexX && inst.index != 4) {
    inst.index += 8;
  }
}

void AsmX86::processDisp8(Instruction &inst) {
  inst.disp = reader->getUChar();
  inst.dispBytes = 1;
}

void AsmX86::processDisp32(Instruction &inst) {
  inst.disp = reader->getUInt32();
  inst.dispBytes = 4;
}

void AsmX86::processImm8(Instruction &inst) {
  inst.imm = reader->getUChar();
  inst.immBytes = 1;
}

void AsmX86::processImm32(Instruction &inst) {
  inst.imm = reader->getUInt32();
  inst.immBytes = 4;
}

void AsmX86::processImm64(Instruction &inst) {
  inst.imm = reader->getUInt64();
  inst.immBytes = 8;
}
