#include <QDebug>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QApplication>
#include <QProgressDialog>

#include "../Util.h"
#include "ProgramPane.h"

ProgramPane::ProgramPane(BinaryObjectPtr obj)
  : Pane(Kind::Program), obj{obj}, shown{false}
{
  createLayout();
}

void ProgramPane::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);
  if (!shown) {
    shown = true;
    setup();
  }
}

void ProgramPane::createLayout() {
  treeWidget = new QTreeWidget;
  treeWidget->setHeaderLabels(QStringList{tr("Address"), tr("Data Low"),
        tr("Data High"), tr("ASCII")});
  treeWidget->setColumnWidth(0, obj->getSystemBits() == 64 ? 110 : 70);
  treeWidget->setColumnWidth(1, 200);
  treeWidget->setColumnWidth(2, 200);
  treeWidget->setColumnWidth(3, 110);

  // Set fixed-width font.
  treeWidget->setFont(QFont("Courier"));

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(treeWidget);
  
  setLayout(layout);
}

void ProgramPane::setup() {
  auto text = obj->getSection(SectionType::Text);
  if (text != nullptr) {
    quint64 addr = text->getAddress();
    const QByteArray &data = text->getData();
    int len = data.size(), rows = len / 16;
    if (len % 16 > 0) rows++;

    QProgressDialog progDiag(this);
    progDiag.setLabelText(tr("Processing data.."));
    progDiag.setCancelButton(nullptr);
    progDiag.setRange(0, 100);
    progDiag.show();
    qApp->processEvents();

    for (int row = 0, byte = 0; row < rows; row++) {
      auto *item = new QTreeWidgetItem;

      QString addrStr{QString::number(addr, 16).toUpper()};
      int pad = obj->getSystemBits() / 8, addrLen = addrStr.size();
      if (addrLen < pad) {
        addrStr = QString(pad - addrLen, (char) 48) + addrStr;
      }
      item->setText(0, addrStr);

      QString code, ascii;
      for (int cur = 0; cur < 16 && byte < len; cur++, byte++) {
        QString hex{QString::number((unsigned char) data[byte], 16)};
        if (hex.size() == 1) {
          hex = "0" + hex;
        }
        code += hex + " ";

        int ic = data[byte];
        ascii += (ic >= 32 && ic <= 126 ? (char) ic : '.');
      }
      if (code.endsWith(" ")) {
        code.chop(1);
      }
      code = code.toUpper();
      item->setText(1, code.mid(0, 8 * 3));
      item->setText(2, code.mid(8 * 3));
      item->setText(3, ascii);

      treeWidget->addTopLevelItem(item);
      addr += 16;

      static int lastPerc{0};
      int perc = (float) row / (float) rows * 100.0;
      if (perc > lastPerc || perc == 100) {
        lastPerc = perc;
        progDiag.setValue(perc);
        progDiag.setLabelText(tr("Processing data.. %1% (%2 of %3)")
                              .arg(perc)
                              .arg(Util::formatSize(byte))
                              .arg(Util::formatSize(len)));
        qApp->processEvents();
      }
    }
  }
}
