SET(SDK_MIN "10.8")
SET(SDK "10.8")
SET(DEV_SDK "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${SDK}.sdk")

IF (NOT EXISTS "${DEV_SDK}" AND NOT IS_DIRECTORY "${DEV_SDK}")
  MESSAGE("Could not find Mac OSX SDK at: ${DEV_SDK}")
  MESSAGE("Aborting!")
  RETURN()
ENDIF()

ADD_DEFINITIONS(
  -DMAC
  -DGCC_VISIBILITY
  -mmacosx-version-min=${SDK_MIN}
  )

SET(CMAKE_OSX_SYSROOT ${DEV_SDK})
