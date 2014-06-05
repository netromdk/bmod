#include <QFile>
#include <QDebug>

#include <cmath>

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

  // Read reserved field.
  if (systemBits == 64) {
    r.getUInt32();
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
  qDebug() << "ncmds:" << ncmds;
  /*
  qDebug() << "sizeofcmds:" << sizeofcmds;
  qDebug() << "flags:" << flags;
  */

  // TODO: Load flags when necessary.

  // Parse load commands sequentially. Each consists of the type, size
  // and data.
  for (int i = 0; i < ncmds; i++) {
    qDebug() << "load cmd #" << i;

    quint32 type = r.getUInt32(&ok);
    if (!ok) return false;
    qDebug() << "type:" << type;

    quint32 size = r.getUInt32(&ok);
    if (!ok) return false;
    qDebug() << "size:" << size;

    // LC_SEGMENT or LC_SEGMENT_64
    if (type == 1 || type == 25) {
      qDebug() << "=== SEGMENT ===";
      QString name{f.read(16)};
      qDebug() << "name:" << name;

      // Memory address of this segment.
      if (systemBits == 32) {
        quint32 vmaddr = r.getUInt32(&ok);
        if (!ok) return false;
        qDebug() << "vmaddr:" << vmaddr;
      }
      else {
        quint64 vmaddr = r.getUInt64(&ok);
        if (!ok) return false;
        qDebug() << "vmaddr:" << vmaddr;
      }

      // Memory size of this segment.
      if (systemBits == 32) {
        quint32 vmsize = r.getUInt32(&ok);
        if (!ok) return false;
        qDebug() << "vmsize:" << vmsize;
      }
      else {
        quint64 vmsize = r.getUInt64(&ok);
        if (!ok) return false;
        qDebug() << "vmsize:" << vmsize;
      }

      // File offset of this segment.
      if (systemBits == 32) {
        quint32 fileoff = r.getUInt32(&ok);
        if (!ok) return false;
        qDebug() << "fileoff:" << fileoff;
      }
      else {
        quint64 fileoff = r.getUInt64(&ok);
        if (!ok) return false;
        qDebug() << "fileoff:" << fileoff;
      }

      // Amount to map from the file.
      if (systemBits == 32) {
        quint32 filesize = r.getUInt32(&ok);
        if (!ok) return false;
        qDebug() << "filesize:" << filesize;
      }
      else {
        quint64 filesize = r.getUInt64(&ok);
        if (!ok) return false;
        qDebug() << "filesize:" << filesize;
      }

      // Maximum VM protection.
      quint32 maxprot = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "maxprot:" << maxprot;

      // Initial VM protection.
      quint32 initprot = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "initprot:" << initprot;

      // Number of sections in segment.
      quint32 nsects = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "nsects:" << nsects;

      // Flags.
      quint32 segflags = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "segflags:" << segflags;

      // Read sections.
      if (nsects > 0) {
        qDebug() << endl << "== SECTIONS ==";
        for (int j = 0; j < nsects; j++) {
          QString secname{f.read(16)};
          qDebug() << "secname:" << secname;

          QString segname{f.read(16)};
          qDebug() << "segname:" << segname;

          // Memory address of this section.
          if (systemBits == 32) {
            quint32 addr = r.getUInt32(&ok);
            if (!ok) return false;
            qDebug() << "addr:" << addr;
          }
          else {
            quint64 addr = r.getUInt64(&ok);
            if (!ok) return false;
            qDebug() << "addr:" << addr;
          }

          // Size in bytes of this section.
          if (systemBits == 32) {
            quint32 secsize = r.getUInt32(&ok);
            if (!ok) return false;
            qDebug() << "secsize:" << secsize;
          }
          else {
            quint64 secsize = r.getUInt64(&ok);
            if (!ok) return false;
            qDebug() << "secsize:" << secsize;
          }

          // File offset of this section.
          quint32 secfileoff = r.getUInt32(&ok);
          if (!ok) return false;
          qDebug() << "secfileoff:" << secfileoff;

          // Section alignment (power of 2).
          quint32 align = pow(2, r.getUInt32(&ok));
          if (!ok) return false;
          qDebug() << "align:" << align;

          // File offset of relocation entries.
          quint32 reloff = r.getUInt32(&ok);
          if (!ok) return false;
          qDebug() << "reloff:" << reloff;

          // Number of relocation entries.
          quint32 nreloc = r.getUInt32(&ok);
          if (!ok) return false;
          qDebug() << "nreloc:" << nreloc;

          // Flags.
          quint32 secflags = r.getUInt32(&ok);
          if (!ok) return false;
          qDebug() << "secflags:" << secflags;

          // Reserved fields.
          r.getUInt32();
          r.getUInt32();
          if (systemBits == 64) {
            r.getUInt32();
          }

          qDebug();
        }
      }
    }

    // LC_DYLD_INFO or LC_DYLD_INFO_ONLY
    else if (type == 0x22 || type == (0x22 | 0x80000000)) {
      qDebug() << "=== DYLD INFO ===";

      // File offset to rebase info.
      quint32 rebase_off = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "rebase_off:" << rebase_off;

      // Size of rebase info.
      quint32 rebase_size = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "rebase_size:" << rebase_size;

      // File offset to binding info.
      quint32 bind_off = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "bind_off:" << bind_off;

      // Size of binding info.
      quint32 bind_size = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "bind_size:" << bind_size;

      // File offset to weak binding info.
      quint32 weak_bind_off = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "weak_bind_off:" << weak_bind_off;

      // Size of weak binding info.
      quint32 weak_bind_size = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "weak_bind_size:" << weak_bind_size;

      // File offset to lazy binding info.
      quint32 lazy_bind_off = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "lazy_bind_off:" << lazy_bind_off;

      // Size of lazy binding info.
      quint32 lazy_bind_size = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "lazy_bind_size:" << lazy_bind_size;

      // File offset to export info.
      quint32 export_off = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "export_off:" << export_off;

      // Size of export info.
      quint32 export_size = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "export_size:" << export_size;
    }

    qDebug();
  }

  return true;
}
