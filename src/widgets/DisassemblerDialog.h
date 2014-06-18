#ifndef BMOD_DISASSEMBLER_DIALOG_H
#define BMOD_DISASSEMBLER_DIALOG_H

#include <QDialog>

#include "../CpuType.h"

class QTextEdit;
class QSplitter;
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
  void setAsmVisible(bool visible = true);

  CpuType cpuType;

  QTextEdit *machineText, *asmText;
  QSplitter *splitter;
  QComboBox *cpuTypeBox;
};

#endif // BMOD_DISASSEMBLER_DIALOG_H
