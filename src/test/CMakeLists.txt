# Unit test for Cache modules
add_executable(cache_test cache_test.cc)

target_link_libraries(
  cache_test
  -Wl,--start-group
  ${STATIC_LIB_LIST}
  -Wl,--end-group
  pthread
)

# Main test
add_executable(main_test main_test.cc)

target_link_libraries(
  main_test
  -Wl,--start-group
  ${STATIC_LIB_LIST}
  -Wl,--end-group
  pthread
)

# Performance test
add_executable(perf_test perf_test.cc)

target_link_libraries(
  perf_test
  -Wl,--start-group
  ${STATIC_LIB_LIST}
  -Wl,--end-group
  pthread
)

add_subdirectory("mvcc_demo")