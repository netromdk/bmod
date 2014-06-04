#include <QFile>
#include <QDebug>

#include "MachO.h"
#include "../Reader.h"

MachO::MachO(const QString &file)
  : Format(Kind::MachO), file{file}, magic{0}, cpuType{0}, cpuSubType{0},
  fileType{0}, ncmds{0}, sizeOfCmds{0}, flags{0}, littleEndian{true},
  systemBits{32}
{ }

bool MachO::detect() {
  QFile f{file};
  if (!f.open(QIODevice::ReadOnly)) {
    return false;
  }

  Reader r(f);
  bool ok;
  quint32 magic = r.getUInt32(&ok);
  if (!ok) return false;

  return magic == 0xFEEDFACE || // 32-bit little endian
    magic == 0xFEEDFACF || // 64-bit little endian
    magic == 0xECAFDEEF || // 32-bit big endian
    magic == 0xFCAFDEEF; // 64-bit big endian
}

bool MachO::parse() {
  QFile f{file};
  if (!f.open(QIODevice::ReadOnly)) {
    return false;
  }

  Reader r(f);
  bool ok;
  magic = r.getUInt32(&ok);
  if (!ok) return false;

  cpuType = r.getUInt32(&ok);
  if (!ok) return false;

  cpuSubType = r.getUInt32(&ok);
  if (!ok) return false;

  fileType = r.getUInt32(&ok);
  if (!ok) return false;

  ncmds = r.getUInt32(&ok);
  if (!ok) return false;

  sizeOfCmds = r.getUInt32(&ok);
  if (!ok) return false;

  flags = r.getUInt32(&ok);
  if (!ok) return false;

  qDebug() << "magic:" << magic;
  if (magic == 0xFEEDFACE) {
    systemBits = 32;
    littleEndian = true;
  }
  else if (magic == 0xFEEDFACF) {
    systemBits = 64;
    littleEndian = true;
  }
  else if (magic == 0xECAFDEEF) {
    systemBits = 32;
    littleEndian = false;
  }
  else if (magic == 0xFCAFDEEF) {
    systemBits = 64;
    littleEndian = false;
  }
  qDebug() << "system bits:" << systemBits;
  qDebug() << "little endian:" << littleEndian;

  qDebug() << "cputype:" << cpuType;
  qDebug() << "cpusubtype:" << cpuSubType;
  qDebug() << "filetype:" << fileType;
  qDebug() << "ncmds:" << ncmds;
  qDebug() << "sizeofcmds:" << sizeOfCmds;
  qDebug() << "flags:" << flags;

  return false;
}
