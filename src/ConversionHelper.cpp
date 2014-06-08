#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QGroupBox>
#include <QPushButton>
#include <QGridLayout>
#include <QMessageBox>

#include "Util.h"
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

void ConversionHelper::onHexToText() {
  QString hex = hexEdit->toPlainText().trimmed().replace(" ", "");
  if (hex.isEmpty()) return;

  QString text = Util::hexToAscii(hex, 0, hex.size() / 2);
  if (text.isEmpty()) {
    QMessageBox::information(this, "bmod",
                             tr("Could not convert hex to text."));
    return;
  }
  textEdit->setText(text);
}

void ConversionHelper::onTextToHex() {
  QString text = textEdit->toPlainText().trimmed();
  if (text.isEmpty()) return;

  QString hex;
  foreach (auto c, text) {
    int ic = c.toLatin1();
    hex += Util::padString(QString::number(ic, 16).toUpper(), 2) + " ";
  }
  if (hex.isEmpty()) {
    QMessageBox::information(this, "bmod",
                             tr("Could not convert text to hex."));
    return;
  }
  hexEdit->setText(hex);
}

void ConversionHelper::createLayout() {
  auto *numLayout = new QGridLayout;
  numLayout->setContentsMargins(5, 5, 5, 5);
  numLayout->addWidget(new QLabel(tr("Octal")), 0, 0);
  numLayout->addWidget(new QLabel(tr("Decimal")), 1, 0);
  numLayout->addWidget(new QLabel(tr("Hexadecimal")), 2, 0);
  numLayout->addWidget(new QLabel(tr("ASCII")), 3, 0);

  for (int i = 0; i < 4; i++) {
    auto *edit = new QLineEdit;
    connect(edit, &QLineEdit::textEdited,
            this, &ConversionHelper::onTextEdited);

    if (i == 3) {
      edit->setMaxLength(1);
    }
    
    numLayout->addWidget(edit, i, 1);
    edits << edit;
  }

  auto *numGroup = new QGroupBox(tr("Numbers"));
  numGroup->setLayout(numLayout);

  hexEdit = new QTextEdit;
  hexEdit->setMaximumHeight(80);

  textEdit = new QTextEdit;
  textEdit->setMaximumHeight(80);

  auto *hexToText = new QPushButton(tr("Hex -> Text"));
  connect(hexToText, &QPushButton::clicked,
          this, &ConversionHelper::onHexToText);

  auto *textToHex = new QPushButton(tr("Text -> Hex"));
  connect(textToHex, &QPushButton::clicked,
          this, &ConversionHelper::onTextToHex);

  auto *buttonLayout = new QHBoxLayout;
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->addStretch();
  buttonLayout->addWidget(hexToText);
  buttonLayout->addWidget(textToHex);
  buttonLayout->addStretch();

  auto *textLayout = new QVBoxLayout;
  textLayout->setContentsMargins(5, 5, 5, 5);
  textLayout->addWidget(new QLabel(tr("Hex strings")));
  textLayout->addWidget(hexEdit);

  textLayout->addWidget(new QLabel(tr("Text")));
  textLayout->addWidget(textEdit);

  textLayout->addLayout(buttonLayout);

  auto *textGroup = new QGroupBox(tr("Text"));
  textGroup->setLayout(textLayout);

  auto *layout = new QVBoxLayout;
  layout->addWidget(numGroup);
  layout->addWidget(textGroup);
  
  setLayout(layout);
}
