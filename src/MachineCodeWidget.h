#ifndef BMOD_MACHINE_CODE_WIDGET_H
#define BMOD_MACHINE_CODE_WIDGET_H

#include <QWidget>

#include "BinaryObject.h"

class QLabel;
class QTreeWidget;

class MachineCodeWidget : public QWidget {
public:
  MachineCodeWidget(BinaryObjectPtr obj);

protected:
  void showEvent(QShowEvent *event);

private:
  void createLayout();
  void setup();

  BinaryObjectPtr obj;
  
  bool shown;
  QLabel *label;
  QTreeWidget *treeWidget;
};

#endif // BMOD_MACHINE_CODE_WIDGET_H
