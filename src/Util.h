#ifndef BMOD_UTIL_H
#define BMOD_UTIL_H

#include <QString>

#include "CpuType.h"

class Util {
public:
  static QString cpuTypeString(CpuType type);
};

#endif // BMOD_UTIL_H
