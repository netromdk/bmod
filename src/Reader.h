#ifndef BMOD_READER_H
#define BMOD_READER_H

class QIODevice;

class Reader {
public:
  Reader(QIODevice &dev);

  quint32 getUInt32(bool *ok = nullptr);

private:
  QIODevice &dev;
};

#endif // BMOD_READER_H
