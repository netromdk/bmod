#include <QFile>
#include <QDebug>

#include "MachO.h"
#include "../Util.h"
#include "../Reader.h"

MachO::MachO(const QString &file)
  : Format(Kind::MachO), file{file}, littleEndian{true}, systemBits{32},
  cpuType{CpuType::X86}
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

  quint32 magic, cputype, cpusubtype, filetype, ncmds, sizeofcmds, flags;

  Reader r(f);
  bool ok;
  magic = r.getUInt32(&ok);
  if (!ok) return false;

  cputype = r.getUInt32(&ok);
  if (!ok) return false;

  cpusubtype = r.getUInt32(&ok);
  if (!ok) return false;

  filetype = r.getUInt32(&ok);
  if (!ok) return false;

  ncmds = r.getUInt32(&ok);
  if (!ok) return false;

  sizeofcmds = r.getUInt32(&ok);
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
  qDebug() << " system bits:" << systemBits;
  qDebug() << " little endian:" << littleEndian;

  qDebug() << "cputype:" << cputype;
  if (cputype == 7) { // CPU_TYPE_X86, CPU_TYPE_I386
    cpuType = CpuType::X86;
  }
  else if (cputype == 7 + 0x01000000) { // CPU_TYPE_X86 | CPU_ARCH_ABI64
    cpuType = CpuType::X86_64;
  }
  else if (cputype == 11) { // CPU_TYPE_HPPA
    cpuType = CpuType::HPPA;
  }
  else if (cputype == 12) { // CPU_TYPE_ARM
    cpuType = CpuType::ARM;
  }
  else if (cputype == 14) { // CPU_TYPE_SPARC
    cpuType = CpuType::SPARC;
  }
  else if (cputype == 15) { // CPU_TYPE_I860
    cpuType = CpuType::I860;
  }
  else if (cputype == 18) { // CPU_TYPE_POWERPC
    cpuType = CpuType::PowerPc;
  }
  else if (cputype == 18 + 0x01000000) { // CPU_TYPE_POWERPC | CPU_ARCH_ABI64
    cpuType = CpuType::PowerPc_64;
  }
  qDebug() << " cpu type:" << Util::cpuTypeString(cpuType);

  qDebug() << "cpusubtype:" << cpusubtype;
  qDebug() << "filetype:" << filetype;
  qDebug() << "ncmds:" << ncmds;
  qDebug() << "sizeofcmds:" << sizeofcmds;
  qDebug() << "flags:" << flags;

  return false;
}
