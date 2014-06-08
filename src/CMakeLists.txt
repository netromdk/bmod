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

  BinaryObject.h
  BinaryObject.cpp

  MainWindow.h
  MainWindow.cpp

  BinaryWidget.h
  BinaryWidget.cpp

  MachineCodeWidget.h
  MachineCodeWidget.cpp

  ConversionHelper.h
  ConversionHelper.cpp

  panes/Pane.h
  panes/ArchPane.h
  panes/ArchPane.cpp
  panes/ProgramPane.h
  panes/ProgramPane.cpp
  panes/StringsPane.h
  panes/StringsPane.cpp

  formats/Format.h
  formats/Format.cpp
  formats/MachO.h
  formats/MachO.cpp
  )

QT5_USE_MODULES(${NAME} Core Gui Widgets)
