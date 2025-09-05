cmake_minimum_required(VERSION 3.5)

project(canfestival VERSION 1.0.0)

option(CANFESTIVAL_BUILD_SHARED "Build CanFestival as shared libraries" ON)
if(CANFESTIVAL_BUILD_SHARED)
  set(CANFESTIVAL_LIB_TYPE SHARED)
else()
  set(CANFESTIVAL_LIB_TYPE STATIC)
endif()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/include/unix/
  ${CMAKE_CURRENT_SOURCE_DIR}/include/timers_unix/
  ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/od/)

# add_compile_options(-fPIC -Wall -g -Wmissing-prototypes -fno-strict-aliasing)
add_compile_options(-fPIC -Wall -g -fno-strict-aliasing)

add_compile_definitions(NOT_USE_DYNAMIC_LOADING CO_ENABLE_LSS CO_ENABLE_LSS_FS)

add_compile_definitions(DEBUG_ERR_CONSOLE_ON)
# add_compile_definitions(DEBUG_WAR_CONSOLE_ON)

set(CANFESTIVAL_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dcf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/emcy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lifegrd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lss.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nmtMaster.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nmtSlave.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/objacces.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sdo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pdo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/states.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/symbols.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sync.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/timer.c)

set(CANFESTIVAL_DRV_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/can_socket/can_socket.c
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/timers_unix/timers_linux.c
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/unix/unix.c)

set(SLAVE_APP_C_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/slave.c
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/od/slavedic.c)

set(MASTER_APP_C_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/master.c
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/gendcf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/od/masterdic.c)

set(SLAVE_APP_CPP_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/slave.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/od/slavedic.c)

set(MASTER_APP_CPP_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/master.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/gendcf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/od/masterdic.c)

add_library(canfestival ${CANFESTIVAL_LIB_TYPE} ${CANFESTIVAL_SRC})
add_library(canfestival_can_socket ${CANFESTIVAL_LIB_TYPE} ${CANFESTIVAL_DRV_SRC})

set(LINK_LIBS canfestival canfestival_can_socket pthread dl rt)

add_executable(slave ${SLAVE_APP_C_SRC})
target_link_libraries(slave ${LINK_LIBS})

add_executable(master ${MASTER_APP_C_SRC})
target_link_libraries(master ${LINK_LIBS})

add_executable(slave_cpp ${SLAVE_APP_CPP_SRC})
target_link_libraries(slave_cpp ${LINK_LIBS})

add_executable(master_cpp ${MASTER_APP_CPP_SRC})
target_link_libraries(master_cpp ${LINK_LIBS})

add_executable(sdo_read ${CMAKE_CURRENT_SOURCE_DIR}/examples/tools/sdo_read.c ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/gendcf.c ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/od/masterdic.c)
target_link_libraries(sdo_read ${LINK_LIBS})

add_executable(sdo_write ${CMAKE_CURRENT_SOURCE_DIR}/examples/tools/sdo_write.c ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/gendcf.c ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/od/masterdic.c)
target_link_libraries(sdo_write ${LINK_LIBS})


# ----------------------------------------------------------------------------
# Install settings (use -DCMAKE_INSTALL_PREFIX=/your/prefix when configuring)
# ----------------------------------------------------------------------------
include(GNUInstallDirs)

# 需要安装的目标（库 + 可执行文件）
install(TARGETS
    canfestival
    canfestival_can_socket
    master
    slave
    master_cpp
    slave_cpp
  sdo_read
  sdo_write
    EXPORT CanFestivalTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}      # 静态库 (.a)
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}      # 共享库 (若改成 SHARED)
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})     # 可执行文件

# 安装公共头文件
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h")

# 安装生成/手写的 OD 相关头文件（若需要对外使用）
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/linux/dcf/od/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/canfestival/od
    FILES_MATCHING PATTERN "*.h")

# 生成并安装配置文件
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/CanFestivalConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CanFestivalConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/CanFestivalConfig.cmake
  @ONLY)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/CanFestivalConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/CanFestivalConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CanFestival)

install(EXPORT CanFestivalTargets
  NAMESPACE CanFestival::
  FILE CanFestivalTargets.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CanFestival)