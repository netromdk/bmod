#ifndef BMOD_MACHINE_CODE_WIDGET_H
#define BMOD_MACHINE_CODE_WIDGET_H

#include <QWidget>

#include "Section.h"
#include "BinaryObject.h"

class QLabel;
class QTreeWidget;

class MachineCodeWidget : public QWidget {
public:
  MachineCodeWidget(BinaryObjectPtr obj, SectionPtr sec);

protected:
  void showEvent(QShowEvent *event);

private:
  void createLayout();
  void setup();

  BinaryObjectPtr obj;
  SectionPtr sec;
  
  bool shown;
  QLabel *label;
  QTreeWidget *treeWidget;
};

#endif // BMOD_MACHINE_CODE_WIDGET_H
