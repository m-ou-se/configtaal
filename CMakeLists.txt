cmake_minimum_required(VERSION 3.1)

project(conftaal VERSION 1.0)

if (NOT DEFINED CMAKE_CXX_STANDARD)
	set(CMAKE_CXX_STANDARD 17)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-sign-compare")
endif()

if (NOT TARGET mstd)
	find_package(mstd)
endif()
if (NOT TARGET stringpool::string_pool)
	find_package(stringpool)
endif()

add_library(conftaal-parser
	src/operator.cpp
	src/parse.cpp
	src/print_error.cpp
)
target_link_libraries(conftaal-parser mstd stringpool::string_tracker)

get_directory_property(parent PARENT_DIRECTORY)
if (NOT parent)
	add_executable(conftaal-test
		src/test.cpp
	)
	target_link_libraries(conftaal-test conftaal-parser)

	add_custom_target(check
		COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test/test" "$<TARGET_FILE:conftaal-test>"
		DEPENDS conftaal-test
		USES_TERMINAL
	)

	add_custom_target(update-tests
		COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test/test" "--update-expected" "$<TARGET_FILE:conftaal-test>"
		DEPENDS conftaal-test
		USES_TERMINAL
	)
endif()

# Install libraries.
include(GNUInstallDirs)
install(TARGETS conftaal-parser EXPORT ${PROJECT_NAME}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install CMake configuration files.
configure_file(config.cmake.in ${PROJECT_NAME}-config.cmake @ONLY)
set(CMAKE_INSTALL_CMAKEDIR "lib/cmake/${PROJECT_NAME}" CACHE PATH "CMake project files")
install(EXPORT ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_CMAKEDIR} NAMESPACE ${PROJECT_NAME}::)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake" DESTINATION "${CMAKE_INSTALL_CMAKEDIR}")
