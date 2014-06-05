#ifndef BMOD_UTIL_H
#define BMOD_UTIL_H

#include <QString>

#include "CpuType.h"
#include "FileType.h"

class Util {
public:
  static QString cpuTypeString(CpuType type);
  static QString fileTypeString(FileType type);
};

#endif // BMOD_UTIL_H
