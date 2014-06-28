#include "BinaryObject.h"

BinaryObject::BinaryObject(CpuType cpuType, CpuType cpuSubType,
                           bool littleEndian, int systemBits, FileType fileType)
  : cpuType{cpuType}, cpuSubType{cpuSubType}, littleEndian{littleEndian},
  systemBits{systemBits}, fileType{fileType}
{
  if (cpuType == CpuType::X86_64) {
    this->systemBits = 64;
  }
}

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
