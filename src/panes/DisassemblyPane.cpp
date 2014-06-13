#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QApplication>
#include <QProgressDialog>
#include <QStyledItemDelegate>

#include "../Util.h"
#include "DisassemblyPane.h"
#include "../asm/Disassembler.h"
#include "../widgets/TreeWidget.h"

namespace {
  class ItemDelegate : public QStyledItemDelegate {
  public:
    ItemDelegate(QTreeWidget *tree, SectionPtr sec) : tree{tree}, sec{sec} { }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const {
      int col = index.column();
      if (col != 1) {
        return nullptr;
      }

      QString text = index.data().toString().trimmed();
      qDebug() << text;
      if (text.isEmpty()) {
        return nullptr;
      }

      QString mask;
      int blocks = text.split(" ").size();
      for (int i = 0; i < blocks; i++) {
        mask += "HH ";
      }
      if (mask.endsWith(" ")) {
        mask.chop(1);
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
        QString newStr = edit->text().toUpper();
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

          // Change region.
          quint64 addr = item->text(0).toULongLong(nullptr, 16);
          quint64 pos = addr - sec->getAddress();
          QByteArray data = Util::hexToData(newStr.replace(" ", ""));
          sec->setSubData(data, pos);

          // TODO: update the disassembly
        }
      }
    }

  private:
    QTreeWidget *tree;
    SectionPtr sec;
  };
}

DisassemblyPane::DisassemblyPane(BinaryObjectPtr obj, SectionPtr sec)
  : Pane(Kind::Disassembly), obj{obj}, sec{sec}, shown{false}
{
  createLayout();
}

void DisassemblyPane::showEvent(QShowEvent *event) {
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

void DisassemblyPane::createLayout() {
  label = new QLabel;

  treeWidget = new TreeWidget;
  treeWidget->setHeaderLabels(QStringList{tr("Address"), tr("Data"), tr("Disassembly")});
  treeWidget->setColumnWidth(0, obj->getSystemBits() == 64 ? 110 : 70);
  treeWidget->setColumnWidth(1, 200);
  treeWidget->setColumnWidth(2, 200);
  treeWidget->setItemDelegate(new ItemDelegate(treeWidget, sec));

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(label);
  layout->addWidget(treeWidget);
  
  setLayout(layout);
}

void DisassemblyPane::setup() {
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
      item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
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

    treeWidget->setFocus();
  }
  else {
    label->setText(tr("Could not disassemble machine code!"));
  }
}
