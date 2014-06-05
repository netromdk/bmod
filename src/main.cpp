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
      qDebug() << "Little Endian:" << fmt->isLittleEndian();
      qDebug() << "System bits:" << fmt->getSystemBits();
      qDebug() << "CPU type:" << Util::cpuTypeString(fmt->getCpuType());
      qDebug() << "CPU sub type:" << Util::cpuTypeString(fmt->getCpuSubType());
      qDebug() << "File type:" << Util::fileTypeString(fmt->getFileType());
      qDebug() << "Sections:";
      foreach (const auto sec, fmt->getSections()) {
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
  else {
    qDebug() << "unknown file";
  }
  return 0;
}
