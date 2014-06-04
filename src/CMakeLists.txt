SET(NAME bmod)

ADD_EXECUTABLE(
  ${NAME}

  main.cpp

  formats/Format.h
  formats/MachO.h
  formats/MachO.cpp
  )

QT5_USE_MODULES(${NAME} Core Gui)
