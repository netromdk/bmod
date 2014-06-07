#include <QListWidget>
#include <QHBoxLayout>
#include <QStackedLayout>

#include "Util.h"
#include "BinaryWidget.h"

#include "panes/Pane.h"
#include "panes/ArchPane.h"
#include "panes/ProgramPane.h"

BinaryWidget::BinaryWidget(FormatPtr fmt) : fmt{fmt} {
  createLayout();
  setup();
}

void BinaryWidget::createLayout() {
  listWidget = new QListWidget;
  listWidget->setFixedWidth(150);

  stackLayout = new QStackedLayout;
  
  auto *layout = new QHBoxLayout;
  layout->addWidget(listWidget);
  layout->addLayout(stackLayout);
  
  setLayout(layout);
}

void BinaryWidget::setup() {
  foreach (const auto obj, fmt->getObjects()) {
    auto *archPane = new ArchPane(obj);
    QString cpuStr = Util::cpuTypeString(obj->getCpuType()),
      cpuSubStr = Util::cpuTypeString(obj->getCpuSubType());
    addPane(tr("%1 (%2)").arg(cpuStr).arg(cpuSubStr), archPane);

    auto *progPane = new ProgramPane(obj);
    addPane(tr("Program"), progPane, 1);
  }
}

void BinaryWidget::addPane(const QString &title, Pane *pane, int level) {
  panes << pane;
  listWidget->addItem(QString(level * 4, ' ') + title);
  stackLayout->addWidget(pane); 
}
