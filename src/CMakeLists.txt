# aux_source_directory(./* DIR_SRCS)

# add_executable(${ProjectName} ${DIR_SRCS})

set(FOLDERS table io util coding)

foreach(FD ${FOLDERS})
  # message(${FD})
  add_subdirectory(${FD})

  aux_source_directory(${FD} MODULE_LIST)
endforeach(FD)

# Find all source files and store basenames without extension in BASE_NAME_NOEXT
foreach(MODULE ${MODULE_LIST})
  # message(${MODULE})
  string(REGEX REPLACE ".+/" "" BASE_NAME ${MODULE})
  string(REGEX REPLACE ".cc" "" BASE_NAME_NOEXT ${BASE_NAME})
  # message(${BASE_NAME_NOEXT})
  set(STATIC_LIB_LIST ${STATIC_LIB_LIST} ${BASE_NAME_NOEXT})
endforeach(MODULE)

link_directories(${ROOT_DIR}/lib)

# Specified static library and its dependencies
# aux_source_directory(. MODULE_LIST)
add_executable(main main.cc)

# message(${STATIC_LIB_LIST})
target_link_libraries(
  main
  -Wl,--start-group
  ${STATIC_LIB_LIST}
  -Wl,--end-group
  pthread
)
# target_link_libraries(main coding io table)

# Test for modules
add_subdirectory("test")