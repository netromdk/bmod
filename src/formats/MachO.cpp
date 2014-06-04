#include "MachO.h"

MachO::MachO(const QString &file) : Format(Format::MachO), file{file} { }

bool MachO::detect() {
  return false;
}
