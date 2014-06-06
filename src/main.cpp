#include <QDebug>

#include "Util.h"
#include "formats/Format.h"

int main(int argc, char **argv) {
  // Temporary!
  if (argc != 2) {
    qDebug() << "No file given";
    return 0;
  }

  QString file{argv[1]};
  qDebug() << "File:" << file;

  auto fmt = Format::detect(file);
  if (fmt != nullptr) {
    qDebug() << "detected:" << fmt->getName();
    if (fmt->parse()) {
      foreach (const auto bin, fmt->getObjects()) {
        qDebug() << "Little Endian:" << bin->isLittleEndian();
        qDebug() << "System bits:" << bin->getSystemBits();
        qDebug() << "CPU type:" << Util::cpuTypeString(bin->getCpuType());
        qDebug() << "CPU sub type:" << Util::cpuTypeString(bin->getCpuSubType());
        qDebug() << "File type:" << Util::fileTypeString(bin->getFileType());
        qDebug() << "Sections:";
        foreach (const auto sec, bin->getSections()) {
          qDebug();
          qDebug() << "Type:" << Util::sectionTypeString(sec->getType());
          qDebug() << "Address:" << sec->getAddress();
          qDebug() << "Size:" << sec->getSize();
          qDebug() << "Offset:" << sec->getOffset();
          qDebug() << "Data:";
          QString addrView{Util::addrDataString(sec->getAddress(), sec->getData())};
          qDebug() << qPrintable(addrView);
        }
      }
    }
  }
  else {
    qDebug() << "unknown file";
  }
  return 0;
}
