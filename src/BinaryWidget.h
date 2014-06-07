#ifndef BMOD_BINARY_WIDGET_H
#define BMOD_BINARY_WIDGET_H

#include <QList>
#include <QWidget>

#include "formats/Format.h"

class Pane;
class QListWidget;
class QStackedLayout;

class BinaryWidget : public QWidget {
public:
  BinaryWidget(FormatPtr fmt);

private:
  void createLayout();
  void setup();
  void addPane(const QString &title, Pane *pane, int level = 0);
  
  FormatPtr fmt;

  QListWidget *listWidget;
  QStackedLayout *stackLayout;
  QList<Pane*> panes;
};

#endif // BMOD_BINARY_WIDGET_H
