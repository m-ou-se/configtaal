add_executable(conftaal-test-evaluate evaluate.cpp)
target_link_libraries(conftaal-test-evaluate conftaal-parse conftaal-evaluate)

add_executable(conftaal-test-value value.cpp)
target_link_libraries(conftaal-test-value conftaal-parse conftaal-evaluate)

add_custom_target(conftaal-run-test-value    COMMAND conftaal-test-value    USES_TERMINAL)
add_custom_target(conftaal-run-test-evaluate COMMAND conftaal-test-evaluate USES_TERMINAL)

add_dependencies(check conftaal-run-test-value conftaal-run-test-evaluate)
