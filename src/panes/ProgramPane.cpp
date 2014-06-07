#include <QDebug>
#include <QVBoxLayout>
#include <QTreeWidget>

#include "ProgramPane.h"

ProgramPane::ProgramPane(BinaryObjectPtr obj) : Pane(Kind::Program), obj{obj} {
  createLayout();
  setup();
}

void ProgramPane::createLayout() {
  treeWidget = new QTreeWidget;
  treeWidget->setHeaderLabels(QStringList{tr("Address"), tr("Data"), tr("ASCII")});
  treeWidget->setColumnWidth(0, obj->getSystemBits() == 64 ? 110 : 70);
  treeWidget->setColumnWidth(1, 370);
  treeWidget->setColumnWidth(2, 110);

  // Set fixed-width font.
  treeWidget->setFont(QFont("Courier"));

  auto *layout = new QVBoxLayout;
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

    for (int row = 0, byte = 0; row < rows; row++) {
      auto *item = new QTreeWidgetItem;
      item->setText(0, QString::number(addr, 16).toUpper());

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
      item->setText(1, code.toUpper());
      item->setText(2, ascii);

      treeWidget->addTopLevelItem(item);
      addr += 16;
    }
  }
}
