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

AsmX86::AsmX86(BinaryObjectPtr obj) : obj{obj}, reader{nullptr} { }

bool AsmX86::disassemble(SectionPtr sec, QString &result) {
  QBuffer buf;
  buf.setData(sec->getData());
  buf.open(QIODevice::ReadOnly);
  reader.reset(new Reader(buf));

  // Address of main()
  quint64 funcAddr = sec->getAddress();

  bool ok{true}, peek{false};
  unsigned char ch, nch, ch2, r, mod, first, second;
  quint32 num;
  while (!reader->atEnd()) {
    ch = reader->getUChar(&ok);
    if (!ok) return false;

    nch = reader->peekUChar(&peek);

    // PUSH
    if (ch >= 0x50 && ch <= 0x57) {
      r = ch - 0x50;
      result += "pushl " + getModRMByte(r, RegType::R32) + "\n";
    }

    // POP
    else if (ch >= 0x58 && ch <= 0x60) {
      r = ch - 0x58;
      result += "popl " + getModRMByte(r, RegType::R32) + "\n";
    }

    // ADD, OR, ADC, SBB, AND, SUB, XOR, CMP
    // (r/m16/32	imm8)
    else if (ch == 0x83 && peek) {
      reader->getUChar(); // eat
      splitByteModRM(nch, mod, first, second);

      ch2 = reader->getUChar(&ok);
      if (!ok) return false;

      if (first == 0) {
        result += "addl";
      }
      else if (first == 1) {
        result += "orl";
      }
      else if (first == 2) {
        result += "adcl"; // Add with carry
      }
      else if (first == 3) {
        result += "sbbl"; // Integer subtraction with borrow
      }
      else if (first == 4) {
        result += "andl";
      }
      else if (first == 5) {
        result += "subl";
      }
      else if (first == 6) {
        result += "xorl";
      }
      else if (first == 7) {
        result += "cmpl";
      }

      result += " $" + formatHex(ch2, 2) + "," +
        getModRMByte(second, RegType::R32) + "\n";
    }

    // MOV (r/m16/32	r16/32)
    else if (ch == 0x89 && peek) {
      reader->getUChar(); // eat
      splitByteModRM(nch, mod, first, second);
      result += "movl " + getModRMByte(nch, RegType::R32) + "\n";
    }

    // MOV (r16/32	imm16/32)
    else if (ch >= 0xB8 && ch <= 0xBF) {
      splitByteModRM(ch, mod, first, second);
      num = reader->getUInt32(&ok);
      if (!ok) return false;
      result += "movl $" + formatHex(num, 8) + "," +
        getModRMByte(second, RegType::R32) + "\n";
    }

    // LEA (r16/32	m) Load Effective Address
    else if (ch == 0x8D && peek) {
      reader->getUChar(); // eat
      splitByteModRM(nch, mod, first, second);
      num = reader->getUInt32(&ok);
      if (!ok) return false;
      result += "leal " + formatHex(num, 8) + "(" +
        getModRMByte(first, RegType::R32) + ")," +
        getModRMByte(second, RegType::R32) + "\n";
    }

    // RETN
    else if (ch == 0xC3) {
      result += "ret\n";
    }

    // MOV (r/m16/32	imm16/32)
    else if (ch == 0xC7 && peek) {
      reader->getUChar(); // eat
      splitByteModRM(nch, mod, first, second);
      ch2 = reader->getUChar(&ok);
      if (!ok) return false;
      num = reader->getUInt32(&ok);
      if (!ok) return false;
      result += "movl $" + formatHex(num, 8) + "," + formatHex(ch2, 2) + "(" +
        getModRMByte(second, RegType::R32) + ")\n";
    }

    // Call (relative function address)
    else if (ch == 0xE8) {
      num = reader->getUInt32(&ok);
      if (!ok) return false;
      result += "calll " + formatHex(funcAddr + reader->pos() + num, 8) + "\n";
    }

    // Unsupported
    else {
      result += "Unsupported: " + QString::number(ch, 16).toUpper() + "\n";
    }
  }

  return !result.isEmpty();
}

void AsmX86::splitByteModRM(unsigned char num, unsigned char &mod,
                            unsigned char &first, unsigned char &second) {
  mod = (num & 0xC0) >> 6; // 2 first bits
  first = (num & 0x38) >> 3; // 3 next bits
  second = num & 0x7; // 3 last bits  
}

QString AsmX86::getReg(RegType type, int num) {
  if (num < 0 || num > 7) {
    return QString();
  }
  static QString regs[6][8] = {{"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"},
                               {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"},
                               {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"},
                               {"mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"},
                               {"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"},
                               {"es", "cs", "ss", "ds", "fs", "gs", "reserved", "reserved"}};
  return regs[type][num];
}

QString AsmX86::getModRMByte(unsigned char num, RegType type) {
  if (num >= 0 && num <= 7) {
    return "%" + getReg(type, num);
  }

  unsigned char mod, first, second;
  splitByteModRM(num, mod, first, second);

  if (mod == 0) {
    // TODO!
    qDebug() << "unsupported mod=0";
  }

  // [reg]+disp8
  else if (mod == 1) {
    unsigned char num = reader->getUChar();
    return "%" + getReg(type, first) + "," + formatHex(num, 2) + "(%" +
      getReg(type, second) + ")";
  }

  // [reg]+disp32
  else if (mod == 2) {
    quint32 num = reader->getUInt32();
    return "%" + getReg(type, first) + "," + formatHex(num, 8) + "(%" +
      getReg(type, second) + ")";
  }

  else if (mod == 3) {
    return "%" + getReg(type, first) + ",%" + getReg(type, second);
  }

  return QString();
}

QString AsmX86::formatHex(quint32 num, int len) {
  return "0x" + Util::padString(QString::number(num, 16), len);
}
