#ifndef BMOD_UTIL_H
#define BMOD_UTIL_H

#include <QString>
#include <QByteArray>

#include "Section.h"
#include "CpuType.h"
#include "FileType.h"

class Util {
public:
  static QString cpuTypeString(CpuType type);
  static QString fileTypeString(FileType type);
  static QString sectionTypeString(SectionType type);
  static QString addrDataString(quint64 addr, QByteArray data);
};

#endif // BMOD_UTIL_H
