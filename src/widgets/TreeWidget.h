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
  void resizeEvent(QResizeEvent *event);

private slots:
  void endSearch();
  void onSearchReturnPressed();
  void nextSearchResult();
  void prevSearchResult();

private:
  void doSearch();
  void selectSearchResult(int col, int item);

  QMap<int, QList<QTreeWidgetItem*>> searchResults;
  int curCol, curItem, cur, total;
  QString lastQuery;

  LineEdit *searchEdit;
  QLabel *searchLabel;
};

#endif // BMOD_TREE_WIDGET_H
