#ifndef BMOD_TREE_WIDGET_H
#define BMOD_TREE_WIDGET_H

#include <QMap>
#include <QList>
#include <QTreeWidget>

class QLabel;
class LineEdit;

class TreeWidget : public QTreeWidget {
public:
  TreeWidget(QWidget *parent = nullptr);

protected:
  void keyPressEvent(QKeyEvent *event);

private slots:
  void endSearch();
  void onSearchReturnPressed();

private:
  void doSearch();
  void selectSearchResult(int col, int item);
  void nextSearchResult();

  QMap<int, QList<QTreeWidgetItem*>> searchResults;
  int curCol, curItem, cur, total;
  QString lastQuery;

  LineEdit *searchEdit;
  QLabel *searchLabel;
};

#endif // BMOD_TREE_WIDGET_H
