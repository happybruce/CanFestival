cmake_minimum_required(VERSION 3.15)

project(canfestival LANGUAGES C CXX)

option(BUILD_EXAMPLES "Build example executables" ON)

# Source files
set(CANFESTIVAL_SRC
    src/dcf.c
    src/emcy.c
    src/lifegrd.c
    src/lss.c
    src/nmtMaster.c
    src/nmtSlave.c
    src/objacces.c
    src/sdo.c
    src/pdo.c
    src/states.c
    # src/symbols.c
    src/sync.c
    src/timer.c
)

# Driver source files
set(CANFESTIVAL_DRV_SRC
    drivers/can_socket/can_socket.c
    drivers/timers_unix/timers_linux.c
    drivers/unix/unix.c
)




# Main library
add_library(canfestival STATIC ${CANFESTIVAL_SRC})
target_include_directories(canfestival PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/unix>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/timers_unix>
)
target_compile_options(canfestival PRIVATE -fPIC -Wall -g -fno-strict-aliasing)
target_compile_definitions(canfestival PUBLIC NOT_USE_DYNAMIC_LOADING CO_ENABLE_LSS CO_ENABLE_LSS_FS DEBUG_ERR_CONSOLE_ON)

install(TARGETS canfestival
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)

# Driver library
add_library(canfestival_can_socket STATIC ${CANFESTIVAL_DRV_SRC})
target_include_directories(canfestival_can_socket PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/unix>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/timers_unix>
)
target_compile_options(canfestival_can_socket PRIVATE -fPIC -Wall -g -fno-strict-aliasing)

install(TARGETS canfestival_can_socket
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)



# Examples

if(BUILD_EXAMPLES)
    set(SLAVE_APP_C_SRC
        examples/linux/dcf/slave.c
        examples/linux/dcf/od/slavedic.c
    )

    set(MASTER_APP_C_SRC
        examples/linux/dcf/master.c
        examples/linux/dcf/gendcf.c
        examples/linux/dcf/od/masterdic.c
    )

    set(SLAVE_APP_CPP_SRC
        examples/linux/dcf/slave.cpp
        examples/linux/dcf/od/slavedic.c
    )

    set(MASTER_APP_CPP_SRC
        examples/linux/dcf/master.cpp
        examples/linux/dcf/gendcf.c
        examples/linux/dcf/od/masterdic.c
    )

    set(LINK_LIBS canfestival canfestival_can_socket pthread dl rt)

    add_executable(slave ${SLAVE_APP_C_SRC})
    target_link_libraries(slave PRIVATE ${LINK_LIBS})
    target_include_directories(slave PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/unix>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/timers_unix>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/od>
    )
    install(TARGETS slave RUNTIME DESTINATION bin)

    add_executable(master ${MASTER_APP_C_SRC})
    target_link_libraries(master PRIVATE ${LINK_LIBS})
    target_include_directories(master PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/unix>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/timers_unix>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/od>
    )
    install(TARGETS master RUNTIME DESTINATION bin)

    add_executable(slave_cpp ${SLAVE_APP_CPP_SRC})
    target_link_libraries(slave_cpp PRIVATE ${LINK_LIBS})
    target_include_directories(slave_cpp PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/unix>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/timers_unix>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/od>
    )
    install(TARGETS slave_cpp RUNTIME DESTINATION bin)

    add_executable(master_cpp ${MASTER_APP_CPP_SRC})
    target_link_libraries(master_cpp PRIVATE ${LINK_LIBS})
    target_include_directories(master_cpp PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/unix>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/timers_unix>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/od>
    )
    install(TARGETS master_cpp RUNTIME DESTINATION bin)
endif()
