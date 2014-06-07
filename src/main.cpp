#include <QStringList>
#include <QApplication>

#include "MainWindow.h"

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  QStringList files;
  for (int i = 1; i < argc; i++) {
    files << QString::fromUtf8(argv[i]);
  }

  MainWindow main(files);
  main.show();

  return app.exec();
}
