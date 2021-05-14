cmake_minimum_required (VERSION 2.6)
project(chif)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(chif src/main.c)
install(TARGETS chif RUNTIME DESTINATION bin)

set(SERVICE_FILE_SRC_DIR ${PROJECT_SOURCE_DIR}/service_files)
install(FILES ${SERVICE_FILE_SRC_DIR}/xyz.openbmc_project.GxpChif.service DESTINATION /lib/systemd/system/)
