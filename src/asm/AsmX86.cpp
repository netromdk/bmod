/**
 * References:
 *   http://www.read.seas.harvard.edu/~kohler/class/04f-aos/ref/i386.pdf
 *   http://ref.x86asm.net
 */

#include <QDebug>
#include <QBuffer>

#include "AsmX86.h"
#include "../Util.h"
#include "../Reader.h"
#include "../Section.h"

namespace {
  QString Instruction::toString() const {
    QString str(mnemonic);
    if (dstRegSet) {
      if (!str.endsWith(" ")) str += " ";
      str += getRegString(dstReg, dstRegType);
    }
    if (srcRegSet) {
      if (!dstRegSet && !str.endsWith(" ")) {
        str += " ";
      }
      else {
        str += ",";
      }
      str += getRegString(srcReg, srcRegType);
    }
    return str;
  }

  QString Instruction::getRegString(int reg, RegType type) const {
    if (reg < 0 || reg > 7) {
      return QString();
    }
    static QString regs[6][8] = {{"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"},
                                 {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"},
                                 {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"},
                                 {"mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"},
                                 {"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"},
                                 {"es", "cs", "ss", "ds", "fs", "gs", "reserved", "reserved"}};
    return "%" + regs[(int) type][reg];
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
  unsigned char ch, nch, ch2, r, mod, op1, op2;
  quint32 num{0};
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

    // MOV (r16/32	r/m16/32) (reverse of 0x89)
    else if (ch == 0x8B && peek) {
      reader->getUChar(); // eat
      Instruction inst;
      inst.mnemonic = "movl";
      processModRegRM(nch, inst);
      inst.dstRegType = RegType::R32;
      inst.srcRegType = RegType::R32;
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

void AsmX86::processModRegRM(unsigned char ch, Instruction &inst) {
  splitByte(ch, inst.mod, inst.dstReg, inst.srcReg);
  inst.dstRegSet = true;
  inst.srcRegSet = true;
}

QString AsmX86::formatHex(quint32 num, int len) {
  return "0x" + Util::padString(QString::number(num, 16), len);
}
