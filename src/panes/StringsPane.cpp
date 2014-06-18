#include <QDebug>
#include <QLabel>
#include <QSplitter>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QApplication>
#include <QProgressDialog>
#include <QStyledItemDelegate>

#include "../Util.h"
#include "StringsPane.h"
#include "../widgets/TreeWidget.h"

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
          Util::setTreeItemMarked(item, index.column());

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

void StringsPane::createLayout() {
  label = new QLabel;

  treeWidget = new TreeWidget;
  treeWidget->setHeaderLabels(QStringList{tr("Address"), tr("String"),
        tr("Length"), tr("Data")});
  treeWidget->setColumnWidth(0, obj->getSystemBits() == 64 ? 110 : 70);
  treeWidget->setColumnWidth(1, 200);
  treeWidget->setColumnWidth(2, 50);
  treeWidget->setColumnWidth(3, 200);
  treeWidget->setItemDelegate(new ItemDelegate(treeWidget, sec));
  treeWidget->setMachineCodeColumns(QList<int>{3});
  treeWidget->setCpuType(obj->getCpuType());

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(label);
  layout->addWidget(treeWidget);
  
  setLayout(layout);
}

void StringsPane::setup() {
  treeWidget->clear();

  quint64 addr = sec->getAddress();
  const QByteArray &data = sec->getData();
  if (data.isEmpty()) {
    return;
  }

  int len = data.size();

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

  // Mark items as modified if a region states it.
  const auto &modRegs = sec->getModifiedRegions();
  int rows = treeWidget->topLevelItemCount();
  quint64 offset =
    treeWidget->topLevelItem(0)->text(0).toULongLong(nullptr, 16);
  for (int row = 0; row < rows; row++) {
    auto *item = treeWidget->topLevelItem(row);
    addr = item->text(0).toULongLong(nullptr, 16) - offset;
    int size = item->text(2).toInt() + 1; // account for \0
    foreach (const auto &reg, modRegs) {
      if (reg.first >= addr && reg.first < addr + size) {
        Util::setTreeItemMarked(item, 3);
        int excess = (reg.first + reg.second) - (addr + size);
        if (excess > 0) {
          for (int row2 = row + 1; row2 < rows; row2++) {
            auto *item2 = treeWidget->topLevelItem(row2);
            if (item2) {
              int size2 = item2->text(2).toInt() + 1; // account for \0
              Util::setTreeItemMarked(item2, 3);
              excess -= size2;
              if (excess <= 0) break;
            }
            else break;
          }
        }
      }
    }
  }

  int padSize = obj->getSystemBits() / 8;
  addr = sec->getAddress();
  label->setText(tr("Section size: %1, address %2 to %3, %4 rows")
                 .arg(Util::formatSize(len))
                 .arg(Util::padString(QString::number(addr, 16).toUpper(),
                                      padSize))
                 .arg(Util::padString(QString::number(addr + len, 16).toUpper(),
                                      padSize))
                 .arg(treeWidget->topLevelItemCount()));

  treeWidget->setFocus();
}
