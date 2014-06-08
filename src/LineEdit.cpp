#include "LineEdit.h"

LineEdit::LineEdit(QWidget *parent) : QLineEdit(parent) { }

void LineEdit::focusOutEvent(QFocusEvent *event) {
  emit focusLost();
}
