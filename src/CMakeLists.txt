SET(NAME bmod)

ADD_EXECUTABLE(
  ${NAME}

  main.cpp

  Reader.h
  Reader.cpp

  formats/Format.h
  formats/Format.cpp
  formats/MachO.h
  formats/MachO.cpp
  )

QT5_USE_MODULES(${NAME} Core Gui)
