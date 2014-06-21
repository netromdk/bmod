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
    return "not implemented yet";
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
      // 1. set instruction type (PUSH)
      // 2. find r value and set register
      // 3. convert to string value and add result
    }


    // Unsupported
    else {
      addResult("Unsupported: " + QString::number(ch, 16).toUpper(),
                pos, result);
    }
  }

  return !result.asmLines.isEmpty();
}

void AsmX86::addResult(const QString &line, qint64 pos, Disassembly &result) {
  result.asmLines << line;
  result.bytesConsumed << reader->pos() - pos;
}

void AsmX86::splitByte(unsigned char num, unsigned char &mod, unsigned char &op1,
                       unsigned char &op2) {
  mod = (num & 0xC0) >> 6; // 2 first bits
  op1 = (num & 0x38) >> 3; // 3 next bits
  op2 = num & 0x7; // 3 last bits  
}

/*
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
  return "%" + regs[(int) type][num];
}
*/

QString AsmX86::formatHex(quint32 num, int len) {
  return "0x" + Util::padString(QString::number(num, 16), len);
}
