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

  widgets/MainWindow.h
  widgets/MainWindow.cpp
  widgets/TreeWidget.h
  widgets/TreeWidget.cpp
  widgets/LineEdit.h
  widgets/LineEdit.cpp
  widgets/BinaryWidget.h
  widgets/BinaryWidget.cpp
  widgets/MachineCodeWidget.h
  widgets/MachineCodeWidget.cpp
  widgets/ConversionHelper.h
  widgets/ConversionHelper.cpp

  panes/Pane.h
  panes/ArchPane.h
  panes/ArchPane.cpp
  panes/ProgramPane.h
  panes/ProgramPane.cpp
  panes/AssemblyPane.h
  panes/AssemblyPane.cpp
  panes/StringsPane.h
  panes/StringsPane.cpp
  panes/GenericPane.h
  panes/GenericPane.cpp

  formats/Format.h
  formats/Format.cpp
  formats/MachO.h
  formats/MachO.cpp

  asm/Asm.h
  asm/AsmX86.h
  asm/AsmX86.cpp
  asm/Disassembler.h
  asm/Disassembler.cpp
  )

QT5_USE_MODULES(${NAME} Core Gui Widgets)
