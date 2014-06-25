#ifndef BMOD_PANE_H
#define BMOD_PANE_H

#include <QWidget>

class Pane : public QWidget {
  Q_OBJECT

public:
  enum class Kind {
    Arch,
    Program,
    Disassembly,
    Strings,
    Symbols,
    Generic
  };

signals:
  void modified();

protected:
  Pane(Kind kind) : kind{kind} { }  

public:
  Kind getKind() const { return kind; }

private:
  Kind kind;
};

#endif // BMOD_PANE_H
