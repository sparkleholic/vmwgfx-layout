cmake_minimum_required(VERSION 3.0.0)
project(vmwgfx-setlayout VERSION 0.1.0)

set(targets)

add_executable(vmwgfx-setlayout setlayout.c)
set(targets ${targets} vmwgfx-setlayout)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

install(TARGETS ${targets}
        RUNTIME DESTINATION bin)

configure_file(unit/vmwgfx-setlayout.cmake vmwgfx-setlayout.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/vmwgfx-setlayout.service DESTINATION /lib/systemd/system)
