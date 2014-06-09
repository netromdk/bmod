#ifndef BMOD_STRINGS_PANE_H
#define BMOD_STRINGS_PANE_H

#include <QDateTime>
#include <QTreeWidgetItem>

#include "Pane.h"
#include "../Section.h"
#include "../BinaryObject.h"

class QLabel;
class TreeWidget;

class StringsPane : public Pane {
public:
  StringsPane(BinaryObjectPtr obj, SectionPtr sec);

protected:
  void showEvent(QShowEvent *event);

private:
  void createLayout();
  void setup();
  void setItemMarked(QTreeWidgetItem *item, int column);

  BinaryObjectPtr obj;
  SectionPtr sec;
  QDateTime secModified;

  bool shown;
  QLabel *label;
  TreeWidget *treeWidget;
};

#endif // BMOD_STRINGS_PANE_H
