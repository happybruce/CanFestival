cmake_minimum_required(VERSION 3.5)


project(canfestival)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/include/default/
    ${CMAKE_CURRENT_SOURCE_DIR}/include/unix/
    ${CMAKE_CURRENT_SOURCE_DIR}/include/timers_unix/
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/od/
)

# add_compile_options(-fPIC -Wall -g -Wmissing-prototypes -fno-strict-aliasing)
add_compile_options(-fPIC -Wall -g -fno-strict-aliasing)

add_compile_definitions(NOT_USE_DYNAMIC_LOADING)


set(CANFESTIVAL_SRC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dcf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/emcy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lifegrd.c

    # ${CMAKE_CURRENT_SOURCE_DIR}/src/lss.c

    ${CMAKE_CURRENT_SOURCE_DIR}/src/nmtMaster.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nmtSlave.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/objacces.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sdo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pdo.c
    
    ${CMAKE_CURRENT_SOURCE_DIR}/src/states.c

    # ${CMAKE_CURRENT_SOURCE_DIR}/src/symbols.c

    ${CMAKE_CURRENT_SOURCE_DIR}/src/sync.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/timer.c
)

set(CANFESTIVAL_DRV_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/can_socket/can_socket.c
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/timers_unix/timers_unix.c
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/unix/unix.c
)



set(SLAVE_APP_SRC 
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/gendcf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/od/slavedic.c
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/slave.c
)

# set(MASTER_APP_SRC 
#     ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/gendcf.c
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/masterdic.c
#     ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/master.c
# )


add_library(canfestival STATIC ${CANFESTIVAL_SRC})
add_library(canfestival_can_socket STATIC ${CANFESTIVAL_DRV_SRC})





add_executable(slave ${SLAVE_APP_SRC} )

target_link_libraries(slave 
    canfestival
    canfestival_can_socket
    pthread
    dl
    rt
)

# add_executable(master ${MASTER_APP_SRC} )
# target_link_libraries(master 
#     ${CMAKE_CURRENT_SOURCE_DIR}/lib/libcanfestival.a 
#     ${CMAKE_CURRENT_SOURCE_DIR}/lib/libcanfestival_can_socket.a 
#     pthread
#     dl
#     rt
# )
