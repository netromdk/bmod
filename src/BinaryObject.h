#ifndef BMOD_BINARY_OBJECT_H
#define BMOD_BINARY_OBJECT_H

#include <QList>

#include <memory>

#include "Section.h"
#include "CpuType.h"
#include "FileType.h"
#include "SymbolTable.h"

class BinaryObject;
typedef std::shared_ptr<BinaryObject> BinaryObjectPtr;

class BinaryObject {
public:
  BinaryObject(CpuType cpuType = CpuType::X86, CpuType cpuSubType = CpuType::I386,
               bool littleEndian = true, int systemBits = 32,
               FileType fileType = FileType::Execute)
    : cpuType{cpuType}, cpuSubType{cpuSubType}, littleEndian{littleEndian},
    systemBits{systemBits}, fileType{fileType}
  { }
  
  CpuType getCpuType() const { return cpuType; }
  void setCpuType(CpuType type) { cpuType = type; }
  
  CpuType getCpuSubType() const { return cpuSubType; }
  void setCpuSubType(CpuType type) { cpuSubType = type; }

  bool isLittleEndian() const { return littleEndian; }
  void setLittleEndian(bool little) { littleEndian = little; }

  int getSystemBits() const { return systemBits; }
  void setSystemBits(int bits) { systemBits = bits; }
  
  FileType getFileType() const { return fileType; }
  void setFileType(FileType type) { fileType = type; }

  QList<SectionPtr> getSections() const { return sections; }
  QList<SectionPtr> getSectionsByType(SectionType type) const;
  SectionPtr getSection(SectionType type) const;
  void addSection(SectionPtr ptr) { sections << ptr; }

  void setSymbolTable(const SymbolTable &tbl) { symTable = tbl; }
  const SymbolTable &getSymbolTable() const { return symTable; }

private:
  CpuType cpuType, cpuSubType;
  bool littleEndian;
  int systemBits;
  FileType fileType;
  QList<SectionPtr> sections;
  SymbolTable symTable;
};

#endif // BMOD_BINARY_OBJECT_H
