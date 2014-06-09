#include <QDebug>
#include <QLabel>
#include <QSplitter>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QApplication>
#include <QProgressDialog>
#include <QStyledItemDelegate>

#include "../Util.h"
#include "StringsPane.h"
#include "../MachineCodeWidget.h"

namespace {
  class ItemDelegate : public QStyledItemDelegate {
  public:
    ItemDelegate(QTreeWidget *tree, SectionPtr sec) : tree{tree}, sec{sec} { }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const {
      int col = index.column();
      // TODO: Also make it possible to edit the strings directly
      if (col != 3) {
        return nullptr;
      }

      QString text = index.data().toString().trimmed();
      if (text.isEmpty()) {
        return nullptr;
      }

      QString mask;
      int blocks = text.size() / 2 - 1;
      for (int i = 0; i < blocks; i++) {
        mask += "HH";
      }

      auto *edit = new QLineEdit(parent);
      edit->setInputMask(mask);
      edit->setText(text);
      return edit;
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const {
      auto *edit = qobject_cast<QLineEdit*>(editor);
      if (edit) {
        QString oldStr = model->data(index).toString().trimmed();
        QString newStr = edit->text().toUpper() + "00";
        if (newStr == oldStr) {
          return;
        }
        model->setData(index, newStr);
        auto *item = tree->topLevelItem(index.row());
        if (item) {
          int col = index.column();
          auto font = item->font(col);
          font.setBold(true);
          item->setFont(col, font);
          item->setForeground(col, Qt::red);

          // Update string representation.
          item->setText(1, Util::hexToString(newStr)
                        .replace("\n", "\\n")
                        .replace("\t", "\\t")
                        .replace("\r", "\\r"));

          // Change region.
          quint64 addr = item->text(0).toULongLong(nullptr, 16);
          quint64 pos = addr - sec->getAddress();
          QByteArray data = Util::hexToData(newStr);
          sec->setSubData(data, pos);
        }
      }
    }

  private:
    QTreeWidget *tree;
    SectionPtr sec;
  };
}

StringsPane::StringsPane(BinaryObjectPtr obj, SectionPtr sec)
  : Pane(Kind::Strings), obj{obj}, sec{sec}, shown{false}
{
  createLayout();
}

void StringsPane::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);
  if (!shown && sec->getType() == SectionType::CString) {
    shown = true;
    setup();
    treeWidget->setFocus();
  }
}

void StringsPane::createLayout() {
  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);

  if (sec->getType() != SectionType::CString) {
    auto *codeWidget = new MachineCodeWidget(obj, sec);
    layout->addWidget(codeWidget);
  }
  else {
    label = new QLabel;

    treeWidget = new QTreeWidget;
    treeWidget->setHeaderLabels(QStringList{tr("Address"), tr("String"),
          tr("Length"), tr("Data")});
    treeWidget->setColumnWidth(0, obj->getSystemBits() == 64 ? 110 : 70);
    treeWidget->setColumnWidth(1, 200);
    treeWidget->setColumnWidth(2, 50);
    treeWidget->setColumnWidth(3, 200);
    treeWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    treeWidget->setEditTriggers(QAbstractItemView::DoubleClicked);
    treeWidget->setItemDelegate(new ItemDelegate(treeWidget, sec));

    // Set fixed-width font.
    treeWidget->setFont(QFont("Courier"));

    layout->addWidget(label);
    layout->addWidget(treeWidget);
  }
  
  setLayout(layout);
}

void StringsPane::setup() {
  quint64 addr = sec->getAddress();
  const QByteArray &data = sec->getData();
  if (data.isEmpty()) {
    return;
  }

  int len = data.size();
  int padSize = obj->getSystemBits() / 8;
  label->setText(tr("Section size: %1, address %2 to %3")
                 .arg(Util::formatSize(len))
                 .arg(Util::padString(QString::number(addr, 16).toUpper(),
                                      padSize))
                 .arg(Util::padString(QString::number(addr + len, 16).toUpper(),
                                      padSize)));

  QProgressDialog progDiag(this);
  progDiag.setLabelText(tr("Processing strings.."));
  progDiag.setCancelButton(nullptr);
  progDiag.setRange(0, 100);
  progDiag.show();
  qApp->processEvents();

  QByteArray cur;
  for (int i = 0; i < len; i++) {
    char c = data[i];
    cur += c;
    if (c == 0) {
      auto *item = new QTreeWidgetItem;
      item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      item->setText(0, Util::padString(QString::number(addr, 16).toUpper(),
                                       obj->getSystemBits() / 8));

      QString str = QString::fromUtf8(cur);
      str = str.replace("\n", "\\n").replace("\t", "\\t").replace("\r", "\\r");

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

      static int lastPerc{0};
      int perc = (float) i / (float) len * 100.0;
      if (perc > lastPerc || perc == 100) {
        lastPerc = perc;
        progDiag.setValue(perc);
        progDiag.setLabelText(tr("Processing strings.. %1% (%2 of %3)")
                              .arg(perc)
                              .arg(Util::formatSize(i))
                              .arg(Util::formatSize(len)));
        qApp->processEvents();
      }
    }
  }
}
