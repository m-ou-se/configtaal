cmake_minimum_required(VERSION 3.1)

project(configtaal VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-sign-compare")
endif()

add_subdirectory(third-party/stringpool)

add_library(configtaal-parser STATIC
	src/parse.cpp
	src/precedence.cpp
)
target_link_libraries(configtaal-parser string_tracker)

add_executable(configtaal-test
	src/print_error.cpp
	src/test.cpp
)
target_link_libraries(configtaal-test configtaal-parser)

add_custom_target(check
	COMMAND "${CMAKE_SOURCE_DIR}/test/test" "$<TARGET_FILE:configtaal-test>"
	DEPENDS configtaal-test
	USES_TERMINAL
)

add_custom_target(update-tests
	COMMAND "${CMAKE_SOURCE_DIR}/test/test" "--update-expected" "$<TARGET_FILE:configtaal-test>"
	DEPENDS configtaal-test
	USES_TERMINAL
)
