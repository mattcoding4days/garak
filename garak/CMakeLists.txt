set(PACKAGE_NAME "garak")

#
# NOTE: `get_version()` defines the variable `${${PACKAGE_NAME}_VERSION}` which is used below to set
# the version of the library  (see `cmake/Utils.cmake`)
#
get_version()

#
# NOTE: Rename `${${PACKAGE_NAME}_VERSION}` variable to `${PACKAGE_VERSION}` for easy reference in the
# current `CMakeLists.txt` file.
#
set(PACKAGE_VERSION ${${PACKAGE_NAME}_VERSION})

#
# NOTE: Set our source and include directory variables for use through the build
#
set(GARAK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(GARAK_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(GARAK_TEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")
set(GARAK_EXAMPLES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/examples")

#
# NOTE: add additional project options
#
option(GARAK_BUILD_TESTING "Enable Test builds" ON)
option(GARAK_BUILD_EXAMPLES "Enable example builds" ON)

#
# NOTE: Prevent in source builds (can't build in src/ or in project root)
#
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR OR CMAKE_CURRENT_BINARY_DIR STREQUAL GARAK_SOURCE_DIR)
  message(
    FATAL_ERROR
      "In source builds are not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

#
# NOTE: Bundle all modules here
#
set(UTILS_MODULE
    "${GARAK_INCLUDE_DIR}/${PACKAGE_NAME}/utils/module.hpp"
    "${GARAK_INCLUDE_DIR}/${PACKAGE_NAME}/utils/common.hpp"
    "${GARAK_INCLUDE_DIR}/${PACKAGE_NAME}/utils/version.hpp"
    "${GARAK_SOURCE_DIR}/utils/version.cpp")

set(ERROR_MODULE
    "${GARAK_INCLUDE_DIR}/${PACKAGE_NAME}/error/module.hpp"
    "${GARAK_INCLUDE_DIR}/${PACKAGE_NAME}/error/base_error.hpp"
    "${GARAK_INCLUDE_DIR}/${PACKAGE_NAME}/error/source_location.hpp"
    "${GARAK_SOURCE_DIR}/error/base_error.cpp"
    "${GARAK_SOURCE_DIR}/error/source_location.cpp")

set(CONTAINERS_MODULE
    "${GARAK_INCLUDE_DIR}/${PACKAGE_NAME}/containers/module.hpp"
    "${GARAK_INCLUDE_DIR}/${PACKAGE_NAME}/containers/thread_safe_dequeue.hpp"
    "${GARAK_INCLUDE_DIR}/${PACKAGE_NAME}/containers/thread_safe_map.hpp")

set(NET_MODULE
    "${GARAK_INCLUDE_DIR}/${PACKAGE_NAME}/net/module.hpp"
    "${GARAK_INCLUDE_DIR}/${PACKAGE_NAME}/net/client.hpp"
    "${GARAK_INCLUDE_DIR}/${PACKAGE_NAME}/net/connection.hpp"
    "${GARAK_INCLUDE_DIR}/${PACKAGE_NAME}/net/error.hpp"
    "${GARAK_INCLUDE_DIR}/${PACKAGE_NAME}/net/packet.hpp"
    "${GARAK_INCLUDE_DIR}/${PACKAGE_NAME}/net/server.hpp")

#
# NOTE: The master module includes all other modules defined above. This makes it easy to include sources/headers
# into executables for examples, and tests, as well as installation targets like the library itself
#
set(MASTER_MODULE
    "${GARAK_INCLUDE_DIR}/${PACKAGE_NAME}/garak.hpp"
    ${UTILS_MODULE}
    ${ERROR_MODULE}
    ${CONTAINERS_MODULE}
    ${NET_MODULE})

#
# NOTE: Add the threads library
#
find_package(Threads REQUIRED)

#
# NOTE: This is where 3rd party libraries should be installed via the light weight CPM dependency
# management system. The CPM wiki is here: https://github.com/cpm-cmake/CPM.cmake/wiki/More-Snippets
#
# NOTE: googletest version >= 3.11.0 now has google mock, and google benchmark libraries built into it,
# so we get 3 for 1
#
# NOTE: Using `CPMFindPackage()` to install package only if it is not already installed.
#
if(GARAK_BUILD_TESTING)
  cpmaddpackage(
    NAME
    googletest
    GITHUB_REPOSITORY
    google/googletest
    GIT_TAG
    release-1.11.0
    VERSION
    1.11.0
    OPTIONS
    "BUILD_GMOCK ON"
    "INSTALL_GTEST OFF"
    "gtest_force_shared_crt ON")
  if(googletest_ADDED)
    message(STATUS "gtest configured correctly")
    set(googletest_Installed
        ON
        PARENT_SCOPE)
  elseif(googletest_Installed)
    message(STATUS "gtest already installed")
  else()
    message(WARNING "gtest was not configured properly")
  endif()

  # add tests
  enable_testing()
  message(STATUS "${PACKAGE_NAME} -- Tests Enabled")
  add_subdirectory("tests")
endif()

#
# NOTE: Add the bundled boost asio version 1.24.0 stand alone as an interface library
# https://github.com/chriskohlhoff/asio
#
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE "${GARAK_INCLUDE_DIR}")
target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
target_link_libraries(asio INTERFACE Threads::Threads)
message(STATUS "Adding bundled asio standalone")

#
# NOTE: Add the src directory to complete building the shared library
#
add_subdirectory("src")

#
# NOTE: Build project examples
#
if(GARAK_BUILD_EXAMPLES)
  message(STATUS "${PACKAGE_NAME} -- Examples Enabled")
  add_subdirectory("examples")
endif()
