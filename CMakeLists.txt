cmake_minimum_required(VERSION 3.0)

project(pigpio LANGUAGES C VERSION 0.71)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

find_package(Threads REQUIRED)
find_package(RT REQUIRED)

if(NOT DEFINED BUILD_SHARED_LIBS)
set(BUILD_SHARED_LIBS "ON")
endif(NOT DEFINED BUILD_SHARED_LIBS)

add_compile_options(-Wall)

# libpigpio.(so|a)
add_library(pigpio pigpio.c command.c custom.cext)

# libpigpiod_if.(so|a)
add_library(pigpiod_if pigpiod_if.c command.c)

# libpigpiod_if2.(so|a)
add_library(pigpiod_if2 pigpiod_if2.c command.c)


# x_pigpio
add_executable(x_pigpio x_pigpio.c)
target_link_libraries(x_pigpio pigpio RT::RT Threads::Threads)

# x_pigpiod_if
add_executable(x_pigpiod_if x_pigpiod_if.c)
target_link_libraries(x_pigpiod_if pigpio_if RT::RT Threads::Threads)

# x_pigpiod_if2
add_executable(x_pigpiod_if2 x_pigpiod_if2.c)
target_link_libraries(x_pigpiod_if2 pigpio_if2 RT::RT Threads::Threads)

# pigpiod
add_executable(pigpiod pigpiod.c)
target_link_libraries(pigpiod pigpio RT::RT Threads::Threads)

# pigs
add_executable(pigs pigs.c command.c)
target_link_libraries(pigs Threads::Threads)

# pig2vcd
add_executable(pig2vcd pig2vcd.c command.c)
target_link_libraries(pig2vcd Threads::Threads)

# install

include (GenerateExportHeader)
include (CMakePackageConfigHelpers)

generate_export_header(${PROJECT_NAME})

#install(DIRECTORY
#	DESTINATION ${DESTDIR}/opt/pigpio/cgi
#	PATTERN ""
#	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
#		GROUP_READ GROUP_EXECUTE
#		WORLD_READ WORLD_EXECUTE
#)

install(TARGETS pigpio pigpiod_if pigpiod_if2 pig2vcd pigpiod pigs
    EXPORT ${PROJECT_NAME}Targets
	LIBRARY  DESTINATION lib
	ARCHIVE  DESTINATION lib
	RUNTIME  DESTINATION bin
	INCLUDES DESTINATION include
)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
	VERSION ${${PROJECT_NAME}_VERSION}
	COMPATIBILITY AnyNewerVersion
)

export(EXPORT ${PROJECT_NAME}Targets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake"
  NAMESPACE pigpio::
)

#configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}Config.cmake
#  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
#  COPYONLY
#)

set(ConfigPackageLocation lib/cmake/${PROJECT_NAME})
install(EXPORT ${PROJECT_NAME}Targets
  FILE
    ${PROJECT_NAME}Targets.cmake
  NAMESPACE
    pigpio::
  DESTINATION
    ${ConfigPackageLocation}
)

install(
  FILES
    ${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}Config.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION
    ${ConfigPackageLocation}
)

install(FILES pigpio.h pigpiod_if.h pigpiod_if2.h
	DESTINATION include
	PERMISSIONS OWNER_READ OWNER_WRITE
		GROUP_READ
		WORLD_READ
)

file(GLOB man_1_SRC "*.1")
install(FILES ${man_1_SRC}
	DESTINATION man/man1
	PERMISSIONS OWNER_READ OWNER_WRITE
		GROUP_READ
		WORLD_READ
)

file(GLOB man_3_SRC "*.3")
install(FILES ${man_3_SRC}
	DESTINATION man/man3
	PERMISSIONS OWNER_READ OWNER_WRITE
		GROUP_READ
		WORLD_READ
)

find_package(Python COMPONENTS Interpreter QUIET)

#file(GLOB setup_SRC "setup.py")
#if(Python_FOUND)
#	install(CODE "execute_process(COMMAND cd ${CMAKE_SOURCE_DIR} && ${Python_EXECUTABLE} ${setup_SRC} install)")
#endif()
#
#install(CODE "execute_process(COMMAND ldconfig)")

# One does not need a uninstall command if every file was installed via cmake
# install commands. That python script however seems to need some more
# script code.
# uninstall
#if(Python_FOUND)
#	set(PY_CMD ${Python_EXECUTABLE} ${setup_SRC} install --record /tmp/pigpio > /dev/null)
#	set(PY_CMD ${PY2_CMD} && xargs rm -f < /tmp/pigpio > /dev/null)
#endif()

#add_custom_target(uninstall
#	COMMAND cd ${CMAKE_SOURCE_DIR} && ${PY_CMD}
#)


# package project

include (CPack)