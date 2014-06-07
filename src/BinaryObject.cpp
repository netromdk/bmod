#include "BinaryObject.h"

SectionPtr BinaryObject::getSection(SectionType type) const {
  foreach (auto sec, sections) {
    if (sec->getType() == type) {
      return sec;
    }
  }
  return nullptr;
}
