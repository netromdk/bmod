#ifndef BMOD_MACHO_FORMAT_H
#define BMOD_MACHO_FORMAT_H

#include "Format.h"

class Reader;

class MachO : public Format {
public:
  MachO(const QString &file);

  QString getName() const { return "Mach-O"; }
  QString getFile() const { return file; }

  bool detect();
  bool parse();

  QList<BinaryObjectPtr> getObjects() const { return objects; }

private:
  bool parseHeader(quint32 offset, quint32 size, Reader &reader);

  QString file;
  QList<BinaryObjectPtr> objects;
};

#endif // BMOD_MACHO_FORMAT_H
