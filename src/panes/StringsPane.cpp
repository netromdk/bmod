#include <QDebug>
#include <QSplitter>
#include <QVBoxLayout>
#include <QTreeWidget>

#include "../Util.h"
#include "StringsPane.h"
#include "../MachineCodeWidget.h"

StringsPane::StringsPane(BinaryObjectPtr obj)
  : Pane(Kind::Strings), obj{obj}, shown{false}
{
  createLayout();
}

void StringsPane::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);
  if (!shown) {
    shown = true;
    setup();
  }
}

void StringsPane::createLayout() {
  auto *codeWidget = new MachineCodeWidget(obj, SectionType::CString);

  treeWidget = new QTreeWidget;
  treeWidget->setHeaderLabels(QStringList{tr("Address"), tr("String"),
        tr("Length"), tr("Data")});
  treeWidget->setColumnWidth(0, obj->getSystemBits() == 64 ? 110 : 70);
  treeWidget->setColumnWidth(1, 200);
  treeWidget->setColumnWidth(2, 50);
  treeWidget->setColumnWidth(3, 200);

  // Set fixed-width font.
  treeWidget->setFont(QFont("Courier"));

  auto *splitter = new QSplitter(Qt::Vertical);
  splitter->addWidget(codeWidget);
  splitter->addWidget(treeWidget);

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(splitter);
  
  setLayout(layout);
}

void StringsPane::setup() {
  SectionPtr sec = obj->getSection(SectionType::CString);
  if (sec == nullptr) {
    return;
  }

  quint64 addr = sec->getAddress();
  const QByteArray &data = sec->getData();
  if (data.isEmpty()) {
    return;
  }

  QByteArray cur;
  int len = data.size();
  for (int i = 0; i < len; i++) {
    char c = data[i];
    cur += c;
    if (c == 0) {
      auto *item = new QTreeWidgetItem;
      item->setText(0, Util::padString(QString::number(addr, 16).toUpper(),
                                       obj->getSystemBits() / 8));

      QString str = QString::fromUtf8(cur);
      item->setText(1, str);
      item->setText(2, QString::number(str.size()));

      QString dataStr;
      for (int j = 0; j < cur.size(); j++) {
        QString hex =
          Util::padString(QString::number((unsigned char) cur[j], 16), 2);
        dataStr += hex;
      }
      item->setText(3, dataStr.toUpper());

      treeWidget->addTopLevelItem(item);
      addr += cur.size();
      cur.clear();
    }
  }
}
