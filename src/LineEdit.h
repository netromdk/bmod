#ifndef BMOD_LINE_EDIT_H
#define BMOD_LINE_EDIT_H

#include <QLineEdit>

class LineEdit : public QLineEdit {
  Q_OBJECT

public:
  LineEdit(QWidget *parent = nullptr);

signals:
  void focusLost();
  void keyUp();
  void keyDown();

protected:
  void focusOutEvent(QFocusEvent *event);
  void keyPressEvent(QKeyEvent *event);
};

#endif // BMOD_LINE_EDIT_H
