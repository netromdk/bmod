#include <QLabel>
#include <QVBoxLayout>
#include <QApplication>
#include <QProgressDialog>

#include "../Util.h"
#include "AssemblyPane.h"
#include "../asm/Disassembler.h"
#include "../widgets/TreeWidget.h"

AssemblyPane::AssemblyPane(BinaryObjectPtr obj, SectionPtr sec)
  : Pane(Kind::Assembly), obj{obj}, sec{sec}, shown{false}
{
  createLayout();
}

void AssemblyPane::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);
  if (!shown) {
    shown = true;
    setup();
  }
  else if (sec->isModified()) {
    QDateTime mod = sec->modifiedWhen();
    if (secModified.isNull() || mod != secModified) {
      secModified = mod;
      setup();
    }
  }
}

void AssemblyPane::createLayout() {
  label = new QLabel;

  treeWidget = new TreeWidget;
  treeWidget->setHeaderLabels(QStringList{tr("Address"), tr("Data"), tr("Assembly")});
  treeWidget->setColumnWidth(0, obj->getSystemBits() == 64 ? 110 : 70);
  treeWidget->setColumnWidth(1, 200);
  treeWidget->setColumnWidth(2, 200);

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(label);
  layout->addWidget(treeWidget);
  
  setLayout(layout);
}

void AssemblyPane::setup() {
  treeWidget->clear();

  QProgressDialog progDiag(this);
  progDiag.setLabelText(tr("Disassembling data.."));
  progDiag.setCancelButton(nullptr);
  progDiag.setRange(0, 100);
  progDiag.show();
  qApp->processEvents();

  quint64 addr = sec->getAddress();
  quint32 pos = 0, size = sec->getSize();
  const QByteArray &data = sec->getData();

  Disassembler dis(obj);
  Disassembly result;
  if (dis.disassemble(sec, result)) {
    int len = result.asmLines.size();
    label->setText(tr("%1 instructions").arg(len));
    for (int i = 0; i < len; i++) {
      const QString &line = result.asmLines[i];
      short bytes = result.bytesConsumed[i];

      auto *item = new QTreeWidgetItem;
      item->setText(0, Util::padString(QString::number(addr, 16).toUpper(),
                                       obj->getSystemBits() / 8));

      QString code;
      for (int j = pos; j < pos + bytes && j < size; j++) {
        QString hex =
          Util::padString(QString::number((unsigned char) data[j], 16), 2);
        code += hex + " ";
      }
      if (code.endsWith(" ")) {
        code.chop(1);
      }
      code = code.toUpper();
      item->setText(1, code);

      item->setText(2, line);
      treeWidget->addTopLevelItem(item);

      addr += bytes;
      pos += bytes;

      static int lastPerc{0};
      int perc = (float) i / (float) len * 100.0;
      if (perc > lastPerc || perc == 100) {
        lastPerc = perc;
        progDiag.setValue(perc);
        progDiag.setLabelText(tr("Disassembling data.. %1% (%2 of %3)")
                              .arg(perc)
                              .arg(Util::formatSize(pos))
                              .arg(Util::formatSize(len)));
        qApp->processEvents();
      }
    }
  }
  else {
    label->setText(tr("Could not disassemble machine code!"));
  }
}
