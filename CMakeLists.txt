cmake_minimum_required(VERSION 2.6)
project (textmining)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

if(EXISTS "${CMAKE_MODULE_PATH}/common.cmake")
    include(common)
else()
    message(FATAL_ERROR "Run ./configure")
endif()

include_directories(src)

# Main binaries

add_executable (TextMiningCompiler src/compiler.cc)
add_executable (TextMiningApp src/app.cc)

set_target_properties(TextMiningCompiler PROPERTIES RUNTIME_OUTPUT_DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(TextMiningApp PROPERTIES RUNTIME_OUTPUT_DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}")

# Test tools

add_executable (tool-deserialize EXCLUDE_FROM_ALL test/tool-deserialize.cc)
add_executable (tool-deserialize-print EXCLUDE_FROM_ALL
    test/tool-deserialize-print.cc)
add_executable (tool-print EXCLUDE_FROM_ALL test/tool-deserialize-print.cc)
add_executable (tool-serialize EXCLUDE_FROM_ALL test/tool-serialize.cc)
add_executable (example-dl EXCLUDE_FROM_ALL test/example-dl.cc)

add_custom_target(tools DEPENDS tool-deserialize tool-deserialize-print
    tool-print tool-serialize example-dl)

# Unit tests

enable_testing()
add_executable (unit EXCLUDE_FROM_ALL test/unit.cc)
add_test(unit unit)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS unit)
