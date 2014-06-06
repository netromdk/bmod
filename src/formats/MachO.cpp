#include <QFile>
#include <QDebug>

#include <cmath>

#include "MachO.h"
#include "../Util.h"
#include "../Reader.h"

MachO::MachO(const QString &file) : Format(Kind::MachO), file{file} { }

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
    magic == 0xFCAFDEEF || // 64-bit big endian
    magic == 0xCAFEBABE || // Universal binary little endian
    magic == 0xBEBAFECA; // Universal binary big endian
}

bool MachO::parse() {
  QFile f{file};
  if (!f.open(QIODevice::ReadOnly)) {
    return false;
  }

  Reader r(f);
  bool ok;
  quint32 magic = r.getUInt32(&ok);
  if (!ok) return false;

  // Check if this is a universal "fat" binary.
  if (magic == 0xCAFEBABE || magic == 0xBEBAFECA) {
    // Values are saved as big-endian so read as such.
    r.setLittleEndian(false);

    qDebug() << "magic:" << magic;

    quint32 nfat_arch = r.getUInt32(&ok);
    if (!ok) return false;
    qDebug() << "nfat_arch:" << nfat_arch;

    // Read "fat" headers.
    typedef QPair<quint32, quint32> puu;
    QList<puu> archs;
    for (quint32 i = 0; i < nfat_arch; i++) {
      qDebug() << "arch #" << i;

      quint32 cputype = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "cputype:" << cputype;

      quint32 cpusubtype = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "cpusubtype:" << cpusubtype;

      // File offset to this object file.
      quint32 offset = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "offset:" << offset;

      // Size of this object file.
      quint32 size = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "size:" << size;

      // Alignment as a power of 2.
      quint32 align = pow(2, r.getUInt32(&ok));
      if (!ok) return false;
      qDebug() << "align:" << align;

      qDebug();

      archs << puu(offset, size);
    }

    // Parse the actual binary objects.
    foreach (const auto &arch, archs) {
      if (!parseHeader(arch.first, arch.second, r)) {
        return false;
      }
    }
  }

  // Otherwise, just parse a single object file.
  else {
    return parseHeader(0, 0, r);
  }

  return true;
}

bool MachO::parseHeader(quint32 offset, quint32 size, Reader &r) {
  BinaryObjectPtr binaryObject(new BinaryObject);

  r.seek(offset);
  r.setLittleEndian(true);

  bool ok;
  quint32 magic = r.getUInt32(&ok);
  if (!ok) return false;

  //qDebug() << "magic:" << magic;
  int systemBits{32};
  bool littleEndian{true};
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

  binaryObject->setSystemBits(systemBits);
  binaryObject->setLittleEndian(littleEndian);

  // Read info in the endianness of the file.
  r.setLittleEndian(littleEndian);

  quint32 cputype, cpusubtype, filetype, ncmds, sizeofcmds, flags;

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

  // Read reserved field.
  if (systemBits == 64) {
    r.getUInt32();
  }

  //qDebug() << "cputype:" << cputype;

  // Types in /usr/local/mach/machine.h
  CpuType cpuType{CpuType::X86};
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
    cpuType = CpuType::PowerPC;
  }
  else if (cputype == 18 + 0x01000000) { // CPU_TYPE_POWERPC | CPU_ARCH_ABI64
    cpuType = CpuType::PowerPC_64;
  }

  binaryObject->setCpuType(cpuType);

  // Subtract 64-bit mask.
  if (systemBits == 64) {
    cpusubtype -= 0x80000000;
  }

  //qDebug() << "cpusubtype:" << cpusubtype;
  CpuType cpuSubType{CpuType::I386};
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

  binaryObject->setCpuSubType(cpuSubType);

  //qDebug() << "filetype:" << filetype;
  FileType fileType{FileType::Object};
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

  binaryObject->setFileType(fileType);

  qDebug() << "ncmds:" << ncmds;
  qDebug() << "sizeofcmds:" << sizeofcmds;
  qDebug() << "flags:" << flags;

  // TODO: Load flags when necessary.

  // Parse load commands sequentially. Each consists of the type, size
  // and data.
  for (int i = 0; i < ncmds; i++) {
    qDebug() << "load cmd #" << i;

    quint32 type = r.getUInt32(&ok);
    if (!ok) return false;
    qDebug() << "type:" << type;

    quint32 cmdsize = r.getUInt32(&ok);
    if (!ok) return false;
    qDebug() << "size:" << cmdsize;

    // LC_SEGMENT or LC_SEGMENT_64
    if (type == 1 || type == 25) {
      qDebug() << "=== SEGMENT ===";
      QString name{r.read(16)};
      qDebug() << "name:" << name;

      // Memory address of this segment.
      quint64 vmaddr;
      if (systemBits == 32) {
        vmaddr = r.getUInt32(&ok);
        if (!ok) return false;
      }
      else {
        vmaddr = r.getUInt64(&ok);
        if (!ok) return false;
      }
      qDebug() << "vmaddr:" << vmaddr;

      // Memory size of this segment.
      quint64 vmsize;
      if (systemBits == 32) {
        vmsize = r.getUInt32(&ok);
        if (!ok) return false;
      }
      else {
        vmsize = r.getUInt64(&ok);
        if (!ok) return false;
      }
      qDebug() << "vmsize:" << vmsize;

      // File offset of this segment.
      quint64 fileoff;
      if (systemBits == 32) {
        fileoff = r.getUInt32(&ok);
        if (!ok) return false;
      }
      else {
        fileoff = r.getUInt64(&ok);
        if (!ok) return false;
      }
      qDebug() << "fileoff:" << fileoff;

      // Amount to map from the file.
      quint64 filesize;
      if (systemBits == 32) {
        filesize = r.getUInt32(&ok);
        if (!ok) return false;
      }
      else {
        filesize = r.getUInt64(&ok);
        if (!ok) return false;
      }
      qDebug() << "filesize:" << filesize;

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
          QString secname{r.read(16)};
          qDebug() << "secname:" << secname;

          QString segname{r.read(16)};
          qDebug() << "segname:" << segname;

          // Memory address of this section.
          quint64 addr;
          if (systemBits == 32) {
            addr = r.getUInt32(&ok);
            if (!ok) return false;
          }
          else {
            addr = r.getUInt64(&ok);
            if (!ok) return false;
          }
          qDebug() << "addr:" << addr;

          // Size in bytes of this section.
          quint64 secsize;
          if (systemBits == 32) {
            secsize = r.getUInt32(&ok);
            if (!ok) return false;
          }
          else {
            secsize = r.getUInt64(&ok);
            if (!ok) return false;
          }
          qDebug() << "secsize:" << secsize;

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

          // Store needed sections.
          if (segname == "__TEXT") {
            if (secname == "__text") {
              SectionPtr sec(new Section(SectionType::Text, addr, secsize,
                                         offset + secfileoff));
              binaryObject->addSection(sec);
            }
            else if (secname == "__cstring") {
              SectionPtr sec(new Section(SectionType::CString, addr, secsize,
                                         offset + secfileoff));
              binaryObject->addSection(sec);
            }
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

    // LC_SYMTAB
    else if (type == 2) {
      qDebug() << "=== SYMTAB ===";

      // Symbol table offset.
      quint32 symoff = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "symoff:" << symoff;

      // Number of symbol table entries.
      quint32 nsyms = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "nsyms:" << nsyms;

      // String table offset.
      quint32 stroff = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "stroff:" << stroff;

      // String table size in bytes.
      quint32 strsize = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "strsize:" << strsize;
    }

    // LC_DYSYMTAB
    else if (type == 0xB) {
      qDebug() << "=== DYSYMTAB ===";

      // Index to local symbols.
      quint32 ilocalsym = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "ilocalsym:" << ilocalsym;

      // Number of local symbols.
      quint32 nlocalsym = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "nlocalsym:" << nlocalsym;

      // Index to externally defined symbols.
      quint32 iextdefsym = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "iextdefsym:" << iextdefsym;

      // Number of externally defined symbols.
      quint32 nextdefsym = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "nextdefsym:" << nextdefsym;

      // Index to undefined defined symbols.
      quint32 iundefsym = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "iundefsym:" << iundefsym;

      // Number of undefined defined symbols.
      quint32 nundefsym = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "nundefsym:" << nundefsym;

      // File offset to table of contents.
      quint32 tocoff = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "tocoff:" << tocoff;

      // Number of entries in the table of contents.
      quint32 ntoc = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "ntoc:" << ntoc;

      // File offset to module table.
      quint32 modtaboff = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "modtaboff:" << modtaboff;

      // Number of module table entries.
      quint32 nmodtab = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "nmodtab:" << nmodtab;

      // File offset to referenced symbol table.
      quint32 extrefsymoff = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "extrefsymoff:" << extrefsymoff;

      // Number of referenced symbol table entries.
      quint32 nextrefsyms = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "nextrefsyms:" << nextrefsyms;

      // File offset to indirect symbol table.
      quint32 indirectrefsymoff = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "indirectrefsymoff:" << indirectrefsymoff;

      // Number of indirect symbol table entries.
      quint32 nindirectrefsyms = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "nindirectrefsyms:" << nindirectrefsyms;

      // File offset to external relocation entries.
      quint32 extreloff = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "extreloff:" << extreloff;

      // Number of external relocation entries.
      quint32 nextrel = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "nextrel:" << nextrel;

      // File offset to local relocation entries.
      quint32 locreloff = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "locreloff:" << locreloff;

      // Number of local relocation entries.
      quint32 nlocrel = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "nlocrel:" << nlocrel;
    }

    // LC_LOAD_DYLIB or LC_ID_DYLIB
    else if (type == 0xC || type == 0xD) {
      if (type == 0xC) {
        qDebug() << "=== LOAD DYLIB ===";
      }
      else {
        qDebug() << "=== ID DYLIB ===";
      }

      // Library path name offset.
      quint32 liboffset = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "liboffset:" << liboffset;

      quint32 timestamp = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "timestamp:" << timestamp;

      quint32 current_version = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "current_version:" << current_version;

      quint32 compatibility_version = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "compatibility_version:" << compatibility_version;

      // Library path name.
      QString libname{r.read(cmdsize - liboffset)};
      qDebug() << "lib name:" << libname;
    }

    // LC_LOAD_DYLINKER
    else if (type == 0xE) {
      qDebug() << "=== LOAD DYLINKER ===";

      // Dynamic linker's path name.
      quint32 noffset = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "noffset:" << noffset;

      QString dyname{r.read(cmdsize - noffset)};
      qDebug() << "dyld name:" << dyname;
    }

    // LC_UUID
    else if (type == 0x1B) {
      qDebug() << "=== UUID ===";

      const QByteArray uuid{r.read(16)};
      QString uuidStr;
      for (int h = 0; h < uuid.size(); h++) {
        uuidStr += QString::number((unsigned char) uuid[h], 16);
      }
      qDebug() << "uuid:" << uuidStr.toUpper();
    }

    // LC_VERSION_MIN_MACOSX
    else if (type == 0x24) {
      qDebug() << "=== VERSION MIN MACOSX ===";

      // Version (X.Y.Z is encoded in nibbles xxxx.yy.zz)
      quint32 version = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "version:" << version;

      // SDK version (X.Y.Z is encoded in nibbles xxxx.yy.zz)
      quint32 sdk = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "sdk:" << sdk;
    }

    // LC_SOURCE_VERSION
    else if (type == 0x2A) {
      qDebug() << "=== SOURCE VERSION ===";

      // Version (A.B.C.D.E packed as a24.b10.c10.d10.e10)
      quint64 version = r.getUInt64(&ok);
      if (!ok) return false;
      qDebug() << "version:" << version;
    }

    // LC_MAIN
    else if (type == (0x28 | 0x80000000)) {
      qDebug() << "=== MAIN ===";

      // File (__TEXT) offset of main()
      quint64 entryoff = r.getUInt64(&ok);
      if (!ok) return false;
      qDebug() << "entryoff:" << entryoff;

      // Initial stack size if not zero.
      quint64 stacksize = r.getUInt64(&ok);
      if (!ok) return false;
      qDebug() << "stacksize:" << stacksize;
    }

    // LC_FUNCTION_STARTS or LC_DYLIB_CODE_SIGN_DRS
    else if (type == 0x26 || type == 0x2B) {
      if (type == 0x26) {
        qDebug() << "=== FUNCTION STARTS ===";
      }
      else {
        qDebug() << "=== DYLIB CODE SIGN DRS ===";
      }

      // File offset to data in __LINKEDIT segment.
      quint32 dataoff = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "dataoff:" << dataoff;

      // File size of data in __LINKEDIT segment.
      quint32 datasize = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "datasize:" << datasize;
    }

    // LC_DATA_IN_CODE
    else if (type == 0x29) {
      qDebug() << "=== DATA IN CODE ===";

      // From mach_header to start of data range.
      quint32 hoffset = r.getUInt32(&ok);
      if (!ok) return false;
      qDebug() << "hoffset:" << hoffset;

      // Number of bytes in data range.
      quint16 length = r.getUInt16(&ok);
      if (!ok) return false;
      qDebug() << "length:" << length;

      // Dice kind value.
      quint16 kind = r.getUInt16(&ok);
      if (!ok) return false;
      qDebug() << "kind:" << kind;
    }

    // Temporary: Fail if unknown!
    else {
      qDebug() << "what is type:" << type;
      exit(0);
    }

    qDebug();
  }

  // Fill data of stored sections.
  foreach (auto sec, binaryObject->getSections()) {
    r.seek(sec->getOffset());
    sec->setData(r.read(sec->getSize()));
  }

  objects << binaryObject;
  return true;
}
