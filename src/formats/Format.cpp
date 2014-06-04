#include <QString>

#include "MachO.h"
#include "Format.h"

std::unique_ptr<Format> Format::detect(const QString &file) {
  // Mach-O
  std::unique_ptr<Format> res(new MachO(file));
  if (res->detect()) {
    return res;
  }

  return nullptr;
}
