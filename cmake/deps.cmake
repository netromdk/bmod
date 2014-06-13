# Qt 5
IF (NOT WIN32)
  # Added some paths to help find the modules on some systems.
  SET(CMAKE_PREFIX_PATH
    ${CMAKE_PREFIX_PATH}
    "/usr/local/opt"
    "/usr/local/opt/qt5"
    "/usr/local/Qt-5.3.0"
    "/usr/local/Qt-5.2.1"
    )
ENDIF()

SET(CMAKE_AUTOMOC ON) # Moc when necessary.

# As moc files are generated in the binary dir, tell CMake to always
# look for includes there:
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

FIND_PACKAGE(Qt5Core REQUIRED)
FIND_PACKAGE(Qt5Gui REQUIRED)
FIND_PACKAGE(Qt5Widgets REQUIRED)
