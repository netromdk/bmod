#ifndef BMOD_MACHO_FORMAT_H
#define BMOD_MACHO_FORMAT_H

#include <QString>

#include "Format.h"

class MachO : public Format {
public:
  MachO(const QString &file);

  QString getName() const { return "Mach-O"; }
  
  bool detect();

private:
  QString file;
};

#endif // BMOD_MACHO_FORMAT_H
