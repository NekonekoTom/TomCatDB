# MVCC demo
add_executable(mvcc mvcc.cc)

target_link_libraries(
  mvcc
  -Wl,--start-group
  ${STATIC_LIB_LIST}
  -Wl,--end-group
  pthread
)