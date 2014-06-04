#ifndef BMOD_MACHO_FORMAT_H
#define BMOD_MACHO_FORMAT_H

#include <QString>

#include "Format.h"
#include "../CpuType.h"

class MachO : public Format {
public:
  MachO(const QString &file);

  QString getName() const { return "Mach-O"; }
  
  bool detect();
  bool parse();

private:
  QString file;

  bool littleEndian;
  int systemBits;
  CpuType cpuType, cpuSubType;
};

#endif // BMOD_MACHO_FORMAT_H
