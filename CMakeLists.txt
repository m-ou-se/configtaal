cmake_minimum_required(VERSION 3.1)

project(conftaal VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-sign-compare")
endif()

add_subdirectory(third-party/stringpool)
include_directories(third-party/mstd/include)

add_library(conftaal-parser STATIC
	src/operator.cpp
	src/parse.cpp
)
target_link_libraries(conftaal-parser string_tracker)

add_executable(conftaal-test
	src/print_error.cpp
	src/test.cpp
)
target_link_libraries(conftaal-test conftaal-parser)

add_custom_target(check
	COMMAND "${CMAKE_SOURCE_DIR}/test/test" "$<TARGET_FILE:conftaal-test>"
	DEPENDS conftaal-test
	USES_TERMINAL
)

add_custom_target(update-tests
	COMMAND "${CMAKE_SOURCE_DIR}/test/test" "--update-expected" "$<TARGET_FILE:conftaal-test>"
	DEPENDS conftaal-test
	USES_TERMINAL
)
