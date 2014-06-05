#include "MachO.h"
#include "Format.h"

FormatPtr Format::detect(const QString &file) {
  // Mach-O
  FormatPtr res(new MachO(file));
  if (res->detect()) {
    return res;
  }

  return nullptr;
}
