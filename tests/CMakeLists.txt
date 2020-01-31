FILE(GLOB testprograms "${CMAKE_CURRENT_SOURCE_DIR}/*.*")
list(FILTER testprograms EXCLUDE REGEX CMakeLists.txt)

INSTALL(PROGRAMS ${testprograms}
  DESTINATION    tests/
  COMPONENT      PythonFiles
)
