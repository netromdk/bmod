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

AsmX86::AsmX86(BinaryObjectPtr obj) : obj{obj} { }

bool AsmX86::disassemble(SectionPtr sec, QString &result) {
  QBuffer buf;
  buf.setData(sec->getData());
  buf.open(QIODevice::ReadOnly);
  Reader reader(buf);

  // Address of main()
  quint64 funcAddr = sec->getAddress();

  bool ok{true}, peek{false};
  unsigned char ch, nch, ch2, r, mod, first, second;
  quint32 num;
  while (!reader.atEnd()) {
    ch = reader.getUChar(&ok);
    if (!ok) return false;

    nch = reader.peekUChar(&peek);

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
    else if (ch == 0x83 && peek) {
      reader.getUChar(); // eat
      splitByteModRM(nch, mod, first, second);

      ch2 = reader.getUChar(&ok);
      if (!ok) return false;

      // SUB
      if (first == 5) {
        result += "subl $" + formatHex(ch2) + "," +
          getModRMByte(second, RegType::R32) + "\n";
      }
    }

    // MOV
    else if (ch >= 0x89 && ch <= 0x8A) {
      if (peek) {
        r = nch;
        reader.getUChar(); // Eat next.
        qDebug() << "peek" << QString::number(r, 16);
      }
      else {
        r = ch - 0x89;
        qDebug() << "peek r" << QString::number(r, 16);
      }
      result += "movl " + getModRMByte(r, RegType::R32) + "\n";
    }

    // Call (relative function address)
    else if (ch == 0xE8) {
      num = reader.getUInt32(&ok);
      if (!ok) return false;
      result += "calll " + formatHex(funcAddr + reader.pos() + num, 8) + "\n";
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

  qDebug() << num;
  unsigned char mod, first, second;
  splitByteModRM(num, mod, first, second);
  qDebug() << mod << first << second;
  if (mod == 3) {
    return "%" + getReg(type, first) + ",%" + getReg(type, second);
  }

  // TODO
  return QString();
}

QString AsmX86::formatHex(quint32 num, int len) {
  return "0x" + Util::padString(QString::number(num, 16), len);
}