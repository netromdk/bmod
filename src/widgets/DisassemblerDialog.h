#ifndef BMOD_DISASSEMBLER_DIALOG_H
#define BMOD_DISASSEMBLER_DIALOG_H

#include <QDialog>

#include "../CpuType.h"

class QTextEdit;
class QComboBox;

class DisassemblerDialog : public QDialog {
  Q_OBJECT

public:
  DisassemblerDialog(QWidget *parent = nullptr,
                     CpuType cpuType = CpuType::X86);

private slots:
  void onConvert();

private:
  void createLayout();

  CpuType cpuType;

  QTextEdit *machineText, *asmText;
  QComboBox *cpuTypeBox;
};

#endif // BMOD_DISASSEMBLER_DIALOG_H
