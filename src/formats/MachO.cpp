#include <QFile>
#include <QDebug>

#include "MachO.h"
#include "../Util.h"
#include "../Reader.h"

MachO::MachO(const QString &file)
  : Format(Kind::MachO), file{file}, littleEndian{true}, systemBits{32},
  cpuType{CpuType::X86}, fileType{FileType::Execute}
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

  //qDebug() << "magic:" << magic;
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

  //qDebug() << "cputype:" << cputype;

  // Types in /usr/local/mach/machine.h
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

  // Subtract 64-bit mask.
  if (systemBits == 64) {
    cpusubtype -= 0x80000000;
  }

  //qDebug() << "cpusubtype:" << cpusubtype;
  if (cpusubtype == 3) { // CPU_SUBTYPE_386
    cpuSubType = CpuType::I386;
  }
  else if (cpusubtype == 4) { // CPU_SUBTYPE_486
    cpuSubType = CpuType::I486;
  }
  else if (cpusubtype == 4 + (8 << 4)) { // CPU_SUBTYPE_486SX
    cpuSubType = CpuType::I486_SX;
  }
  else if (cpusubtype == 5) { // CPU_SUBTYPE_PENT
    cpuSubType = CpuType::Pentium;
  }
  else if (cpusubtype == 6 + (1 << 4)) { // CPU_SUBTYPE_PENTPRO
    cpuSubType = CpuType::PentiumPro;
  }
  else if (cpusubtype == 6 + (3 << 4)) { // CPU_SUBTYPE_PENTII_M3
    cpuSubType = CpuType::PentiumII_M3;
  }
  else if (cpusubtype == 6 + (5 << 4)) { // CPU_SUBTYPE_PENTII_M5
    cpuSubType = CpuType::PentiumII_M5;
  }
  else if (cpusubtype == 7 + (6 << 4)) { // CPU_SUBTYPE_CELERON
    cpuSubType = CpuType::Celeron;
  }
  else if (cpusubtype == 7 + (7 << 4)) { // CPU_SUBTYPE_CELERON_MOBILE
    cpuSubType = CpuType::CeleronMobile;
  }
  else if (cpusubtype == 8) { // CPU_SUBTYPE_PENTIUM_3
    cpuSubType = CpuType::Pentium_3;
  }
  else if (cpusubtype == 8 + (1 << 4)) { // CPU_SUBTYPE_PENTIUM_3_M
    cpuSubType = CpuType::Pentium_3_M;
  }
  else if (cpusubtype == 8 + (2 << 4)) { // CPU_SUBTYPE_PENTIUM_3_XEON
    cpuSubType = CpuType::Pentium_3_Xeon;
  }
  else if (cpusubtype == 9) { // CPU_SUBTYPE_PENTIUM_M
    cpuSubType = CpuType::Pentium_M;
  }
  else if (cpusubtype == 10) { // CPU_SUBTYPE_PENTIUM_4
    cpuSubType = CpuType::Pentium_4;
  }
  else if (cpusubtype == 10 + (1 << 4)) { // CPU_SUBTYPE_PENTIUM_4_M
    cpuSubType = CpuType::Pentium_4_M;
  }
  else if (cpusubtype == 11) { // CPU_SUBTYPE_ITANIUM
    cpuSubType = CpuType::Itanium;
  }
  else if (cpusubtype == 11 + (1 << 4)) { // CPU_SUBTYPE_ITANIUM_2
    cpuSubType = CpuType::Itanium_2;
  }
  else if (cpusubtype == 12) { // CPU_SUBTYPE_XEON
    cpuSubType = CpuType::Xeon;
  }
  else if (cpusubtype == 12 + (1 << 4)) { // CPU_SUBTYPE_XEON_MP
    cpuSubType = CpuType::Xeon_MP;
  }

  //qDebug() << "filetype:" << filetype;
  if (filetype == 1) { // MH_OBJECT
    fileType = FileType::Object;
  }
  else if (filetype == 2) { // MH_EXECUTE
    fileType = FileType::Execute;
  }
  else if (filetype == 4) { // MH_CORE
    fileType = FileType::Core;
  }
  else if (filetype == 5) { // MH_PRELOAD
    fileType = FileType::Preload;
  }
  else if (filetype == 6) { // MH_DYLIB
    fileType = FileType::Dylib;
  }
  else if (filetype == 7) { // MH_DYLINKER
    fileType = FileType::Dylinker;
  }
  else if (filetype == 8) { // MH_BUNDLE
    fileType = FileType::Bundle;
  }

  /*
  qDebug() << "ncmds:" << ncmds;
  qDebug() << "sizeofcmds:" << sizeofcmds;
  qDebug() << "flags:" << flags;
  */

  // TODO: Load flags when necessary.

  return true;
}
