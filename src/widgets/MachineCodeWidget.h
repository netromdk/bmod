#ifndef BMOD_MACHINE_CODE_WIDGET_H
#define BMOD_MACHINE_CODE_WIDGET_H

#include <QWidget>
#include <QDateTime>

#include "../Section.h"
#include "../BinaryObject.h"

class QLabel;
class TreeWidget;
class QTreeWidgetItem;

class MachineCodeWidget : public QWidget {
  Q_OBJECT

public:
  MachineCodeWidget(BinaryObjectPtr obj, SectionPtr sec);

signals:
  void modified();

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

#endif // BMOD_MACHINE_CODE_WIDGET_H
