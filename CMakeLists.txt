project( breakthrough )
cmake_minimum_required( VERSION 3.20 )

set( CMAKE_EXPORT_COMPILE_COMMANDS on )
set( CMAKE_CXX_STANDARD_REQUIRED 20 )
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_BUILD_TYPE debug )
set( CMAKE_VERBOSE_MAKEFILE on )

#add_compile_options("-fsanitize=address")
#add_link_options("-fsanitize=address")

set(breakthrough_dir ${CMAKE_SOURCE_DIR})
set(data_dir ${breakthrough_dir}/data)
set(scripts_dir ${breakthrough_dir}/scripts)

add_library(bt game.cpp bitboard.cpp)
target_include_directories(bt PUBLIC ${breakthrough_dir})

add_library(epsilonGreedy epsilonGreedy.cpp)
target_link_libraries(epsilonGreedy bt)

add_library(mcts mcts.cpp)
target_link_libraries(mcts bt)

add_executable(main main.cpp)
target_link_libraries(main bt epsilonGreedy)

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
# Arena
############################################################
add_executable(arena_agentGreedy arena/epsilonGreedyvsRandom.cpp)
target_link_libraries(arena_agentGreedy bt epsilonGreedy)

############################################################
# Tests
############################################################
add_executable(test_actionsgen tests/actions_gen.cpp)
target_link_libraries(test_actionsgen bt epsilonGreedy)

add_executable(benchmark_playouts tests/sample_benchmark.cpp)
target_link_libraries(benchmark_playouts bt epsilonGreedy)

add_executable(view_bbs tests/view_bbs.cpp)
target_link_libraries(view_bbs bt)

add_executable(do_undo tests/do_undo.cpp)
target_link_libraries(do_undo bt)

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
