cmake_minimum_required(VERSION 3.8)
project(conftaal VERSION 1.0)

if (NOT DEFINED CMAKE_CXX_STANDARD)
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if (NOT TARGET mstd)
	find_package(mstd)
endif()
if (NOT TARGET stringpool::string_pool)
	find_package(stringpool)
endif()

add_library(conftaal-parse
	src/operator.cpp
	src/parse.cpp
	src/print_error.cpp
)
target_link_libraries(conftaal-parse mstd stringpool::string_tracker)

add_library(conftaal-evaluate
	src/evaluate.cpp
)
target_link_libraries(conftaal-evaluate conftaal-parse)

add_custom_target(check)
add_subdirectory(test/parse)

# Install libraries.
include(GNUInstallDirs)
install(TARGETS conftaal-parse conftaal-evaluate EXPORT ${PROJECT_NAME}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install CMake configuration files.
configure_file(config.cmake.in ${PROJECT_NAME}-config.cmake @ONLY)
set(CMAKE_INSTALL_CMAKEDIR "lib/cmake/${PROJECT_NAME}" CACHE PATH "CMake project files")
install(EXPORT ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_CMAKEDIR} NAMESPACE ${PROJECT_NAME}::)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake" DESTINATION "${CMAKE_INSTALL_CMAKEDIR}")
