#ifndef BMOD_TREE_WIDGET_H
#define BMOD_TREE_WIDGET_H

#include <QMap>
#include <QList>
#include <QTreeWidget>

#include "../CpuType.h"

class QLabel;
class LineEdit;

class TreeWidget : public QTreeWidget {
  Q_OBJECT

public:
  TreeWidget(QWidget *parent = nullptr);

  void setCpuType(CpuType type) { cpuType = type; }
  void setMachineCodeColumns(const QList<int> columns);

protected:
  void keyPressEvent(QKeyEvent *event);
  void resizeEvent(QResizeEvent *event);

private slots:
  void doSearch();
  void endSearch();
  void onSearchLostFocus();
  void onSearchReturnPressed();
  void nextSearchResult();
  void prevSearchResult();
  void onSearchEdited(const QString &text);
  void onShowContextMenu(const QPoint &pos);
  void disassemble();
  void copyField();
  void copyRow();

private:
  void resetSearch();
  void selectSearchResult(int col, int item);
  void showSearchText(const QString &text);

  QList<int> machineCodeColumns;
  CpuType cpuType;
  QTreeWidgetItem *ctxItem;
  int ctxCol;

  QMap<int, QList<QTreeWidgetItem*>> searchResults;
  int curCol, curItem, cur, total;
  QString lastQuery;

  LineEdit *searchEdit;
  QLabel *searchLabel;
};

#endif // BMOD_TREE_WIDGET_H
