#
# NOTE: add the executables
#
set(ScratchFile "${PACKAGE_NAME}_scratch.bin")

add_executable(${ScratchFile} "${GARAK_EXAMPLES_SOURCE_DIR}/scratch.cpp" ${GARAK_SOURCE_DIR}/version.cpp)

target_include_directories(${ScratchFile} PUBLIC ${GARAK_INCLUDE_DIR})
target_link_libraries(
  ${ScratchFile}
  PRIVATE project_options
          project_warnings
          stdc++fs
          asio)
