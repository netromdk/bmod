#ifndef BMOD_MACHINE_CODE_WIDGET_H
#define BMOD_MACHINE_CODE_WIDGET_H

#include <QWidget>

#include "Section.h"
#include "BinaryObject.h"

class QLabel;
class TreeWidget;

class MachineCodeWidget : public QWidget {
  Q_OBJECT

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
  TreeWidget *treeWidget;
};

#endif // BMOD_MACHINE_CODE_WIDGET_H
