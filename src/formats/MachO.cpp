#include <QFile>
#include <QDebug>
#include <QByteArray>

#include "MachO.h"

MachO::MachO(const QString &file) : Format(Kind::MachO), file{file} { }

bool MachO::detect() {
  QFile f{file};
  if (!f.open(QIODevice::ReadOnly)) {
    qWarning() << "Could not open file for writing:" << file;
    return false;
  }

  constexpr int num = sizeof(quint32);
  QByteArray buf = f.read(num);
  if (buf.size() < num) {
    return false;
  }

  quint32 magic{0};
  for (int i = 0; i < num; i++) {
    magic += ((quint32) (unsigned char) buf[i]) << i * 8;
  }

  // 32-bit is the first and 64-bit is the second.
  return (magic == 0xFEEDFACE || magic == 0xFEEDFACF);
}
