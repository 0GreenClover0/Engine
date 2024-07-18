cmake_minimum_required(VERSION 3.21 FATAL_ERROR)
project(Engine VERSION 2.0)

IF (WIN32)
  message(NOTICE "\n")
  message(NOTICE "********************** IMPORTANT NOTICE ****************************\n")
  message(NOTICE "Make sure you have enabled Developer Mode in your system settings!")
  message(NOTICE "To enable Developer Mode go to Settings -> Update & Security -> For developers -> Developer Mode ON")
  message(NOTICE "\n********************************************************************")
  message(NOTICE "\n")
ENDIF()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(global_settings)

# ---- Dependencies ----
add_subdirectory(thirdparty)

# ---- Main project's files ----
add_subdirectory(src)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})