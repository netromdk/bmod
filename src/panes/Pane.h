#ifndef BMOD_PANE_H
#define BMOD_PANE_H

#include <QWidget>

class Pane : public QWidget {
public:
  enum class Kind {
    Arch,
    Program,
    Assembly,
    Strings,
    Generic
  };

protected:
  Pane(Kind kind) : kind{kind} { }  

public:
  Kind getKind() const { return kind; }

private:
  Kind kind;
};

#endif // BMOD_PANE_H
