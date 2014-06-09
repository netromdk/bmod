#ifndef BMOD_ASM_H
#define BMOD_ASM_H

#include <QString>
#include <QByteArray>

class Asm {
public:
  virtual ~Asm() { }
  virtual QString disassemble(const QByteArray &code) =0;
};

#endif // BMOD_ASM_H
