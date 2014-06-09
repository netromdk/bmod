#ifndef BMOD_BINARY_WIDGET_H
#define BMOD_BINARY_WIDGET_H

#include <QWidget>

#include "../formats/Format.h"

class Pane;
class QListWidget;
class QStackedLayout;

class BinaryWidget : public QWidget {
  Q_OBJECT

public:
  BinaryWidget(FormatPtr fmt);

  QString getFile() const { return fmt->getFile(); }

  void commit();

private slots:
  void onModeChanged(int row);

private:
  void createLayout();
  void setup();
  void addPane(const QString &title, Pane *pane, int level = 0);
  
  FormatPtr fmt;

  QListWidget *listWidget;
  QStackedLayout *stackLayout;
};

#endif // BMOD_BINARY_WIDGET_H
