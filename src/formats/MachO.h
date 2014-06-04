#ifndef BMOD_MACHO_FORMAT_H
#define BMOD_MACHO_FORMAT_H

#include <QString>

#include "Format.h"

class MachO : public Format {
public:
  MachO(const QString &file);

  QString getName() const { return "Mach-O"; }
  
  bool detect();
  bool parse();

private:
  QString file;

  quint32 magic, cpuType, cpuSubType, fileType, ncmds, sizeOfCmds, flags;
  bool littleEndian;
  int systemBits;
};

#endif // BMOD_MACHO_FORMAT_H
