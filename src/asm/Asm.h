#ifndef BMOD_ASM_H
#define BMOD_ASM_H

#include <QString>
#include <QByteArray>

class Asm {
public:
  virtual ~Asm() { }
  virtual bool disassemble(const QByteArray &code, QString &result) =0;
};

#endif // BMOD_ASM_H
