add_executable(conftaal-test-parse
	parse.cpp
)
target_link_libraries(conftaal-test-parse conftaal-parse)

add_custom_target(conftaal-run-test-parse
	COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test" "$<TARGET_FILE:conftaal-test-parse>"
	DEPENDS conftaal-test-parse
	USES_TERMINAL
)

add_custom_target(update-tests
	COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test" "--update-expected" "$<TARGET_FILE:conftaal-test-parse>"
	DEPENDS conftaal-test-parse
	USES_TERMINAL
)
add_dependencies(check conftaal-run-test-parse)
