#ifndef BMOD_MACHO_FORMAT_H
#define BMOD_MACHO_FORMAT_H

#include "Format.h"

class MachO : public Format {
public:
  MachO(const QString &file);

  bool detect();
  bool parse();

  QString getName() const { return "Mach-O"; }
  bool isLittleEndian() const { return littleEndian; }
  int getSystemBits() const { return systemBits; }
  CpuType getCpuType() const { return cpuType; }
  CpuType getCpuSubType() const { return cpuSubType; }
  FileType getFileType() const { return fileType; }
  const QList<SectionPtr> &getSections() const { return sections; }

private:
  QString file;

  bool littleEndian;
  int systemBits;
  CpuType cpuType, cpuSubType;
  FileType fileType;
  QList<SectionPtr> sections;
};

#endif // BMOD_MACHO_FORMAT_H
