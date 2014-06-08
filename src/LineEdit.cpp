#include "LineEdit.h"

LineEdit::LineEdit(QWidget *parent) : QLineEdit(parent) { }

void LineEdit::focusOutEvent(QFocusEvent *event) {
  QLineEdit::focusOutEvent(event);
  emit focusLost();
}
