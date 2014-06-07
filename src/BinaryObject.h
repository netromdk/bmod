#ifndef BMOD_BINARY_OBJECT_H
#define BMOD_BINARY_OBJECT_H

#include <QList>

#include <memory>

#include "Section.h"
#include "CpuType.h"
#include "FileType.h"

class BinaryObject;
typedef std::shared_ptr<BinaryObject> BinaryObjectPtr;

class BinaryObject {
public:
  BinaryObject() : littleEndian{true}, systemBits{32}, cpuType{CpuType::X86},
    fileType{FileType::Execute}
  { }
  
  bool isLittleEndian() const { return littleEndian; }
  void setLittleEndian(bool little) { littleEndian = little; }

  int getSystemBits() const { return systemBits; }
  void setSystemBits(int bits) { systemBits = bits; }
  
  CpuType getCpuType() const { return cpuType; }
  void setCpuType(CpuType type) { cpuType = type; }
  
  CpuType getCpuSubType() const { return cpuSubType; }
  void setCpuSubType(CpuType type) { cpuSubType = type; }
  
  FileType getFileType() const { return fileType; }
  void setFileType(FileType type) { fileType = type; }

  QList<SectionPtr> getSections() const { return sections; }
  SectionPtr getSection(SectionType type) const;
  void addSection(SectionPtr ptr) { sections << ptr; }

private:
  bool littleEndian;
  int systemBits;
  CpuType cpuType, cpuSubType;
  FileType fileType;
  QList<SectionPtr> sections;
};

#endif // BMOD_BINARY_OBJECT_H
