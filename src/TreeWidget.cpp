#include <QDebug>
#include <QLabel>
#include <QKeyEvent>

#include "LineEdit.h"
#include "TreeWidget.h"

TreeWidget::TreeWidget(QWidget *parent)
  : QTreeWidget(parent), curCol{0}, curItem{0}, cur{0}, total{0}
{
  searchEdit = new LineEdit(this);
  searchEdit->setVisible(false);
  searchEdit->setFixedWidth(150);
  searchEdit->setFixedHeight(21);
  connect(searchEdit, &LineEdit::focusLost, this, &TreeWidget::endSearch);
  connect(searchEdit, &LineEdit::returnPressed,
          this, &TreeWidget::onSearchReturnPressed);

  searchLabel = new QLabel(this);
  searchLabel->setVisible(false);
  searchLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  searchLabel->setFixedHeight(searchEdit->height());
  searchLabel->setStyleSheet("QLabel { background-color: #FFFFFF; }");
}

void TreeWidget::keyPressEvent(QKeyEvent *event) {
  QTreeWidget::keyPressEvent(event);

  bool ctrl{false};
#ifdef MAC
  ctrl = event->modifiers() | Qt::MetaModifier;
#else
  ctrl = event->modifiers() | Qt::ControlModifier;
#endif
  if (ctrl && event->key() == Qt::Key_F) {
    doSearch();
  }
  else if (event->key() == Qt::Key_Escape) {
    endSearch();
  }
}

void TreeWidget::endSearch() {
  searchEdit->hide();
  searchLabel->hide();
  searchEdit->clear();
  searchLabel->clear();
  setFocus();
}

void TreeWidget::doSearch() {
  searchEdit->move(width() - searchEdit->width() - 1,
                   height() - searchEdit->height() - 1);
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
  total = 0;
  for (int col = 0; col < cols; col++) {
    auto res = findItems(query, Qt::MatchContains, col);
    if (!res.isEmpty()) {
      searchResults[col] = res;
      total += res.size();
    }
  }

  if (searchResults.isEmpty()) {
    return;
  }

  lastQuery = query;
  cur = 0;
  curCol = searchResults.keys().first();
  curItem = 0;
  selectSearchResult(curCol, curItem);
}

void TreeWidget::selectSearchResult(int col, int item) {
  if (!searchResults.contains(col)) {
    return;
  }

  const auto &list = searchResults[col];
  if (item < 0 || item > list.size() - 1) {
    return;
  }

  const auto &res = list[item];
  searchLabel->setText(tr("%1 of %2 matches  ").arg(cur + 1).arg(total));
  searchLabel->setFixedWidth(width() - searchEdit->width());
  searchLabel->move(1, searchEdit->pos().y());
  searchLabel->show();

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

  cur++;
  if (cur > total - 1) {
    cur = 0;
  }

  selectSearchResult(curCol, curItem);
}
