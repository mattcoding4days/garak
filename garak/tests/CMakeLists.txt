include(GoogleTest)

#
# NOTE: Add all test source files
#
set(GARAK_TEST_SOURCES utils_test.cpp containers_module_test.cpp)

#
# NOTE: Declare a custom name for the test executable
#
set(PACKAGE_UNIT_TEST_NAME "${PACKAGE_NAME}_tests.bin")

#
# NOTE: Add all test sources to the executable, and any other sources
#
add_executable(${PACKAGE_UNIT_TEST_NAME} ${GARAK_TEST_SOURCES} ${MASTER_MODULE})

#
# NOTE: Link any libraries we need to the test executable. The most notable being
# the gtest_main library.
#
target_include_directories(${PACKAGE_UNIT_TEST_NAME} PUBLIC ${GARAK_INCLUDE_DIR})
target_link_libraries(${PACKAGE_UNIT_TEST_NAME} PRIVATE project_options project_warnings gtest_main)

#
# NOTE: Signal google test to discover all tests
#
gtest_discover_tests(${PACKAGE_UNIT_TEST_NAME})
