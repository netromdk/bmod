#ifndef BMOD_READER_H
#define BMOD_READER_H

#include <QByteArray>

#include <memory>

class QIODevice;

class Reader;
typedef std::unique_ptr<Reader> ReaderPtr;

class Reader {
public:
  Reader(QIODevice &dev, bool littleEndian = true);

  bool isLittleEndian() const { return littleEndian; }
  void setLittleEndian(bool little) { littleEndian = little; }

  quint16 getUInt16(bool *ok = nullptr);
  quint32 getUInt32(bool *ok = nullptr);
  quint64 getUInt64(bool *ok = nullptr);

  char getChar(bool *ok = nullptr);
  unsigned char getUChar(bool *ok = nullptr);
  char peekChar(bool *ok = nullptr);
  unsigned char peekUChar(bool *ok = nullptr);

  QByteArray read(qint64 max);

  qint64 pos() const;
  bool seek(qint64 pos);
  bool atEnd() const;

  bool peekList(std::initializer_list<unsigned char> list);

private:
  template <typename T>
  T getUInt(bool *ok = nullptr);

  QIODevice &dev;
  bool littleEndian;
};

#endif // BMOD_READER_H
