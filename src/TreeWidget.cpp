#include <QKeyEvent>

#include "LineEdit.h"
#include "TreeWidget.h"

TreeWidget::TreeWidget(QWidget *parent) : QTreeWidget(parent) {
  searchEdit = new LineEdit(this);
  searchEdit->setVisible(false);
  searchEdit->setFixedWidth(200);
  connect(searchEdit, &LineEdit::focusLost, this, &TreeWidget::endSearch);
  connect(searchEdit, &LineEdit::returnPressed,
          this, &TreeWidget::onSearchReturnPressed);
}

void TreeWidget::keyPressEvent(QKeyEvent *event) {
  bool ctrl{false};
#ifdef MAC
  ctrl = event->modifiers() | Qt::MetaModifier;
#else
  ctrl = event->modifiers() | Qt::ControlModifier;
#endif
  if (ctrl && event->key() == Qt::Key_F) {
    doSearch();
  }
}

void TreeWidget::endSearch() {
  searchEdit->hide();
}

void TreeWidget::doSearch() {
  searchEdit->move(width() - searchEdit->width() - 5,
                   height() - searchEdit->height() - 3);
  searchEdit->show();
  searchEdit->setFocus();
}

void TreeWidget::onSearchReturnPressed() {
  QString query = searchEdit->text().trimmed();
  if (query == lastQuery) {
    nextSearchResult();
    return;
  }

  int cols = columnCount();
  searchResults.clear();
  for (int col = 0; col < cols; col++) {
    auto res = findItems(query, Qt::MatchContains, col);
    if (!res.isEmpty()) {
      searchResults[col] = res;
    }
  }

  if (searchResults.isEmpty()) {
    return;
  }

  lastQuery = query;
  curCol = searchResults.keys().first();
  curItem = 0;
  selectSearchResult(curCol, curItem);
}

void TreeWidget::selectSearchResult(int col, int item) {
  if (!searchResults.contains(col)) {
    return;
  }
  auto list = searchResults[col];
  if (item < 0 || item > list.size() - 1) {
    return;
  }
  auto res = list[item];

  // Select entry and not entire row.
  int row = indexOfTopLevelItem(res);
  auto index = model()->index(row, col);
  selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);  
}

void TreeWidget::nextSearchResult() {
  const auto &list = searchResults[curCol];
  int pos = curItem;
  pos++;
  if (pos > list.size() - 1) {
    curItem = 0;
    const auto &keys = searchResults.keys();
    int pos2 = keys.indexOf(curCol);
    pos2++;
    if (pos2 > keys.size() - 1) {
      curCol = keys[0];
    }
    else {
      curCol = keys[pos2];
    }
  }
  else {
    curItem = pos;
  }
  selectSearchResult(curCol, curItem);
}
