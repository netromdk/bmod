SET(NAME bmod)

ADD_EXECUTABLE(
  ${NAME}

  main.cpp

  Util.h
  Util.cpp

  Reader.h
  Reader.cpp

  Section.h
  Section.cpp

  MainWindow.h
  MainWindow.cpp

  BinaryWidget.h
  BinaryWidget.cpp

  panes/Pane.h
  panes/ArchPane.h
  panes/ArchPane.cpp
  panes/ProgramPane.h
  panes/ProgramPane.cpp

  formats/Format.h
  formats/Format.cpp
  formats/MachO.h
  formats/MachO.cpp
  )

QT5_USE_MODULES(${NAME} Core Gui Widgets)
