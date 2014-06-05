#include <QIODevice>
#include <QByteArray>

#include "Reader.h"

Reader::Reader(QIODevice &dev) : dev{dev} { }
  
quint32 Reader::getUInt32(bool *ok) {
  constexpr int num = sizeof(quint32);
  QByteArray buf = dev.read(num);
  if (buf.size() < num) {
    if (ok) *ok = false;
    return 0;
  }
  quint32 res{0};
  for (int i = 0; i < num; i++) {
    res += ((quint32) (unsigned char) buf[i]) << i * 8;
  }
  if (ok) *ok = true;
  return res;
}

quint64 Reader::getUInt64(bool *ok) {
  constexpr int num = sizeof(quint64);
  QByteArray buf = dev.read(num);
  if (buf.size() < num) {
    if (ok) *ok = false;
    return 0;
  }
  quint64 res{0};
  for (int i = 0; i < num; i++) {
    res += ((quint64) (unsigned char) buf[i]) << i * 8;
  }
  if (ok) *ok = true;
  return res;
}
