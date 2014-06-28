#include <QLabel>
#include <QVBoxLayout>
#include <QApplication>
#include <QProgressDialog>

#include "../Util.h"
#include "SymbolsPane.h"
#include "../widgets/TreeWidget.h"

SymbolsPane::SymbolsPane(BinaryObjectPtr obj, SectionPtr sec, Type type)
  : Pane(Kind::Symbols), obj{obj}, sec{sec}, type{type}, shown{false}
{
  createLayout();
}

void SymbolsPane::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);
  if (!shown) {
    shown = true;
    setup();
  }
}

void SymbolsPane::createLayout() {
  label = new QLabel;

  treeWidget = new TreeWidget;
  treeWidget->setHeaderLabels(QStringList{tr("Index"), tr("Value"), tr("String")});
  treeWidget->setColumnWidth(0, 100);
  treeWidget->setColumnWidth(1, 100);
  treeWidget->setColumnWidth(2, 200);

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(label);
  layout->addWidget(treeWidget);
  
  setLayout(layout);
}

void SymbolsPane::setup() {
  treeWidget->clear();

  QProgressDialog progDiag(this);
  progDiag.setLabelText(tr("Processing symbols.."));
  progDiag.setCancelButton(nullptr);
  progDiag.setRange(0, 100);
  progDiag.show();
  qApp->processEvents();

  const auto &symTable =
    (type == Type::Symbols ? obj->getSymbolTable()
     : obj->getDynSymbolTable());
  foreach (const auto &symbol, symTable.getSymbols()) {
    auto *item = new QTreeWidgetItem;
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    quint32 idx = symbol.getIndex();
    item->setText(0, Util::padString(QString::number(idx, 16).toUpper(),
                                     obj->getSystemBits() / 8));
    item->setText(1, QString::number(symbol.getValue(), 16).toUpper());
    item->setText(2, symbol.getString());
    treeWidget->addTopLevelItem(item);
  }

  int padSize = obj->getSystemBits() / 8;
  qint64 len = sec->getData().size();
  quint64 addr = sec->getAddress();
  label->setText(tr("Section size: %1, address %2 to %3, %4 rows")
                 .arg(Util::formatSize(len))
                 .arg(Util::padString(QString::number(addr, 16).toUpper(),
                                      padSize))
                 .arg(Util::padString(QString::number(addr + len, 16).toUpper(),
                                      padSize))
                 .arg(treeWidget->topLevelItemCount()));
}
