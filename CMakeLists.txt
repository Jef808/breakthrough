project( breakthrough )
cmake_minimum_required( VERSION 3.20 )

set( CMAKE_EXPORT_COMPILE_COMMANDS on )
set( CMAKE_CXX_STANDARD_REQUIRED 20 )
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_BUILD_TYPE release )
set( CMAKE_VERBOSE_MAKEFILE on )

# set( CMAKE_CXX_COMPILER "/usr/local/bin/g++" )
# set( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-I/usr/local/include/c++/12.0.0" )

set(breakthrough_dir ${CMAKE_SOURCE_DIR})
set(data_dir ${breakthrough_dir}/data)
set(scripts_dir ${breakthrough_dir}/scripts)

add_library(bt game.cpp bitboard.cpp)
target_include_directories(bt PUBLIC ${breakthrough_dir})

add_library(agent agent.cpp)
target_link_libraries(agent bt)

add_executable(main main.cpp)
target_link_libraries(main bt agent)

add_custom_target(
  bundle_main
  COMMAND scripts/bundler.py scripts/main_sources.txt
  DEPENDS main
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/btbundled.cpp
)

add_custom_command(
  TARGET main POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${data_dir} data
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${scripts_dir} scripts
)

############################################################
# Tests
############################################################
add_executable(test_actionsgen tests/actions_gen.cpp)
target_link_libraries(test_actionsgen bt agent)

add_executable(benchmark_playouts tests/sample_benchmark.cpp)
target_link_libraries(benchmark_playouts bt)

add_executable(view_bbs tests/view_bbs.cpp)
target_link_libraries(view_bbs bt)

add_custom_target(
  bundle_actionsgen_test
  COMMAND scripts/bundler.py scripts/test_actionsgen_sources.txt
  DEPENDS test_actionsgen
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/btbundled.cpp
)

add_custom_command(
  TARGET bundle_actionsgen_test POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${data_dir} data
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${scripts_dir} scripts
)
