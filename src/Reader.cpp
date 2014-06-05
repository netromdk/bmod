#include <QIODevice>
#include <QByteArray>

#include "Reader.h"

Reader::Reader(QIODevice &dev) : dev{dev} { }

quint16 Reader::getUInt16(bool *ok) {
  return getUInt<quint16>(ok);
}

quint32 Reader::getUInt32(bool *ok) {
  return getUInt<quint32>(ok);
}

quint64 Reader::getUInt64(bool *ok) {
  return getUInt<quint64>(ok);
}

template <typename T>
T Reader::getUInt(bool *ok) {
  constexpr int num = sizeof(T);
  QByteArray buf = dev.read(num);
  if (buf.size() < num) {
    if (ok) *ok = false;
    return 0;
  }
  T res{0};
  for (int i = 0; i < num; i++) {
    res += ((T) (unsigned char) buf[i]) << i * 8;
  }
  if (ok) *ok = true;
  return res;
}
