#ifndef BMOD_CONVERSION_HELPER_H
#define BMOD_CONVERSION_HELPER_H

#include <QList>
#include <QDialog>

class QLineEdit;
class QTextEdit;

class ConversionHelper : public QDialog {
  Q_OBJECT

public:
  ConversionHelper(QWidget *parent = nullptr);

private slots:
  void onTextEdited(const QString &text);
  void onHexToText();
  void onTextToHex();

private:
  void createLayout();

  QList<QLineEdit*> edits;
  QTextEdit *hexEdit, *textEdit;
};

#endif // BMOD_CONVERSION_HELPER_H
