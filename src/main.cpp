#include <QDebug>

#include "formats/MachO.h"

int main(int argc, char **argv) {
  // Temporary!
  if (argc != 2) {
    qDebug() << "No file given";
    return 0;
  }

  QString file{argv[1]};
  qDebug() << "File:" << file;

  MachO fmt(file);
  qDebug() << "MachO:" << fmt.detect();
  return 0;
}
