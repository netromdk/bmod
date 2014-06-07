#include <QStringList>
#include <QApplication>

#include "Version.h"
#include "MainWindow.h"

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  QCoreApplication::setApplicationName("bmod");
  QCoreApplication::setApplicationVersion(versionString());

  QStringList files;
  for (int i = 1; i < argc; i++) {
    files << QString::fromUtf8(argv[i]);
  }

  MainWindow main(files);
  main.show();

  return app.exec();
}
