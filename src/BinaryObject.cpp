#include "BinaryObject.h"

QList<SectionPtr> BinaryObject::getSectionsByType(SectionType type) const {
  QList<SectionPtr> res;
  foreach (auto sec, sections) {
    if (sec->getType() == type) {
      res << sec;
    }
  }
  return res;
}

SectionPtr BinaryObject::getSection(SectionType type) const {
  foreach (auto sec, sections) {
    if (sec->getType() == type) {
      return sec;
    }
  }
  return nullptr;
}
