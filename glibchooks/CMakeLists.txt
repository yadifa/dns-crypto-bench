cmake_minimum_required(VERSION 3.28)
project(glibchooks C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-strict-prototypes")

set(CMAKE_C_STANDARD 23)

add_library(glibchooks SHARED
        glibchooks_main.c
        library.h
        filedescriptor.c
        filedescriptor.h
        alloc.c
        hooks.c
        hooks.h
        alloc.h)
install(TARGETS glibchooks RUNTIME DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}")

add_library(glibchooksctrl SHARED glibchooks_controller.c glibchooks_controller.h)
install(TARGETS glibchooksctrl RUNTIME DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}")

add_executable(hooked hooked.c)
target_link_libraries(hooked PUBLIC glibchooksctrl)
install(TARGETS hooked RUNTIME DESTINATION "${CMAKE_INSTALL_FULL_SBINDIR}")
