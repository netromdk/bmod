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

  formats/Format.h
  formats/Format.cpp
  formats/MachO.h
  formats/MachO.cpp
  )

QT5_USE_MODULES(${NAME} Core Gui)
