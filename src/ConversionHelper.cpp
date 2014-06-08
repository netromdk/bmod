#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>

#include "ConversionHelper.h"

ConversionHelper::ConversionHelper(QWidget *parent) : QDialog(parent) {
  setWindowTitle(tr("Conversion Helper"));
  createLayout();
}

void ConversionHelper::onTextEdited(const QString &text) {
  QLineEdit *edit = qobject_cast<QLineEdit*>(sender());
  if (!edit) return;

  quint64 val{0};
  bool ok{true};

  for (int i = 0; i < edits.size(); i++) {
    QLineEdit *e = edits[i];
    if (e != edit) continue;

    // Oct
    if (i == 0) {
      val = text.toULongLong(&ok, 8);
    }

    // Dec
    else if (i == 1) {
      val = text.toULongLong(&ok, 10);
    }

    // Hex
    else if (i == 2) {
      val = text.toULongLong(&ok, 16);
    }

    // ACSII
    else if (i == 3) {
      if (text.size() == 1) {
        val = (int) (char) text[0].toLatin1();
      }
    }
  }

  for (int i = 0; i < edits.size(); i++) {
    QLineEdit *e = edits[i];
    if (e == edit) continue;

    if (!ok) {
      e->setText("");
      continue;
    }

    // Oct
    if (i == 0) {
      e->setText(QString::number(val, 8));
    }

    // Dec
    else if (i == 1) {
      e->setText(QString::number(val));
    }

    // Hex
    else if (i == 2) {
      e->setText(QString::number(val, 16).toUpper());
    }

    // ACSII
    else if (i == 3 && val > 0) {
      QString s = (val >= 32 && val <= 126 ? QString((char) val) : "");
      e->setText(s);
    }
  }
}

void ConversionHelper::createLayout() {
  auto *layout = new QGridLayout;
  layout->setContentsMargins(5, 5, 5, 5);
  layout->addWidget(new QLabel(tr("Octal")), 0, 0);
  layout->addWidget(new QLabel(tr("Decimal")), 1, 0);
  layout->addWidget(new QLabel(tr("Hexadecimal")), 2, 0);
  layout->addWidget(new QLabel(tr("ASCII")), 3, 0);

  for (int i = 0; i < 4; i++) {
    auto *edit = new QLineEdit;
    connect(edit, &QLineEdit::textEdited,
            this, &ConversionHelper::onTextEdited);

    if (i == 3) {
      edit->setMaxLength(1);
    }
    
    layout->addWidget(edit, i, 1);
    edits << edit;
  }
  
  setLayout(layout);
}
