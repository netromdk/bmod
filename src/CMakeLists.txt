SET(NAME bmod)

ADD_EXECUTABLE(
  ${NAME}

  main.cpp
  )

QT5_USE_MODULES(${NAME} Core Gui)
