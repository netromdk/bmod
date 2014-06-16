#ifndef BMOD_DISASSEMBLY_PANE_H
#define BMOD_DISASSEMBLY_PANE_H

#include <QDateTime>
#include <QTreeWidgetItem>

#include "Pane.h"
#include "../Section.h"
#include "../BinaryObject.h"

class QLabel;
class TreeWidget;
class QPushButton;

class DisassemblyPane : public Pane {
  Q_OBJECT

public:
  DisassemblyPane(BinaryObjectPtr obj, SectionPtr sec);

  void showUpdateButton();

protected:
  void showEvent(QShowEvent *event);

private slots:
  void onUpdateClicked();

private:
  void createLayout();
  void setup();
  void setItemMarked(QTreeWidgetItem *item, int column);

  BinaryObjectPtr obj;
  SectionPtr sec;
  QDateTime secModified;

  bool shown;
  QLabel *label;
  QPushButton *updateBtn;
  TreeWidget *treeWidget;
};

#endif // BMOD_DISASSEMBLY_PANE_H
