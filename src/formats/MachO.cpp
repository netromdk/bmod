#include "MachO.h"

MachO::MachO(const QString &file) : Format(Kind::MachO), file{file} { }

bool MachO::detect() {
  return false;
}
