#
# NOTE: add the executables
#
set(ScratchFile "${PACKAGE_NAME}_scratch.bin")

add_executable(${ScratchFile} "${APP_EXAMPLES_SOURCE_DIR}/scratch.cpp" ${APP_SOURCE_DIR}/version.cpp)
target_include_directories(${ScratchFile} PUBLIC ${APP_INCLUDE_DIR})
target_link_libraries(${ScratchFile} PRIVATE project_options project_warnings stdc++fs)
