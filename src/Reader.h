#ifndef BMOD_READER_H
#define BMOD_READER_H

#include <QByteArray>

class QIODevice;

class Reader {
public:
  Reader(QIODevice &dev, bool littleEndian = true);

  bool isLittleEndian() const { return littleEndian; }
  void setLittleEndian(bool little) { littleEndian = little; }

  quint16 getUInt16(bool *ok = nullptr);
  quint32 getUInt32(bool *ok = nullptr);
  quint64 getUInt64(bool *ok = nullptr);

  QByteArray read(quint64 max);

  bool seek(quint64 pos);

private:
  template <typename T>
  T getUInt(bool *ok = nullptr);

  QIODevice &dev;
  bool littleEndian;
};

#endif // BMOD_READER_H
