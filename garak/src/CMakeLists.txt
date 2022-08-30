#
# NOTE: Output build param information
#
message(STATUS "Building ${PACKAGE_NAME}_v${PACKAGE_VERSION} [${CMAKE_BUILD_TYPE}]")

#
# NOTE: Target core as a shared library
#
add_library(${PACKAGE_NAME} SHARED ${MASTER_MODULE})

target_include_directories(${PACKAGE_NAME} PUBLIC ${GARAK_INCLUDE_DIR})
target_link_libraries(
  ${PACKAGE_NAME}
  PRIVATE project_options
          project_warnings
          stdc++fs
          asio)

#
# NOTE: Set the install targets
# This will enable the library header files to be installed at /usr/include/
# and the shared object files to be installed to /usr/lib/
#
install(TARGETS ${PACKAGE_NAME} LIBRARY DESTINATION lib)
install(DIRECTORY "${GARAK_INCLUDE_DIR}/" DESTINATION include)
set_target_properties(${PACKAGE_NAME} PROPERTIES VERSION ${${PACKAGE_NAME}_VERSION} SOVERSION ${${PACKAGE_NAME}_VMAJOR})
