#include <QDebug>

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
    qDebug() << "detected";
  }
  else {
    qDebug() << "unknown file";
  }
  return 0;
}
