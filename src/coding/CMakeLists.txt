aux_source_directory(. SRC_LIST)

# Build static library for each source file
foreach(MODULE ${SRC_LIST})
  string(REGEX REPLACE "./" "" BASE_NAME ${MODULE})
  string(REGEX REPLACE ".cc" "" BASE_NAME_NOEXT ${BASE_NAME})

  add_library(${BASE_NAME_NOEXT} STATIC ${MODULE})
endforeach(MODULE)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)