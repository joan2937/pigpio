cmake_minimum_required(VERSION 3.0)

project(pigpio LANGUAGES C VERSION 0.71)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

find_package(Threads REQUIRED)
find_package(RT REQUIRED)

option(BUILD_SHARED_LIBS "Create shared libraries" ON)
option(MAN_PAGES "Create manual pages" OFF)
option(SOCKET_INTERFACE "Build socket interface" ON)
option(TESTS "Build tests" OFF)
option(PIG2VCD "Build VCD format converter" OFF)

add_compile_options(-Wall)

# libpigpio.(so|a)
add_library(pigpio pigpio.c command.c custom.cext)

install(TARGETS pigpio
	EXPORT ${PROJECT_NAME}Targets
	LIBRARY  DESTINATION lib${LIB_SUFFIX}
	ARCHIVE  DESTINATION lib${LIB_SUFFIX}
	RUNTIME  DESTINATION bin
	INCLUDES DESTINATION include
)

install(FILES pigpio.h
	DESTINATION include
	PERMISSIONS OWNER_READ OWNER_WRITE
		GROUP_READ
		WORLD_READ
)

if(TESTS)
	# x_pigpio
	add_executable(x_pigpio x_pigpio.c)
	target_link_libraries(x_pigpio pigpio RT::RT Threads::Threads)
endif()

if(SOCKET_INTERFACE)
	# libpigpiod_if.(so|a)
	add_library(pigpiod_if pigpiod_if.c command.c)

	# libpigpiod_if2.(so|a)
	add_library(pigpiod_if2 pigpiod_if2.c command.c)

	# pigpiod
	add_executable(pigpiod pigpiod.c)
	target_link_libraries(pigpiod pigpio RT::RT Threads::Threads)

	# pigs
	add_executable(pigs pigs.c command.c)
	target_link_libraries(pigs Threads::Threads)

	install(TARGETS pigpiod_if pigpiod_if2 pigpiod pigs
		EXPORT ${PROJECT_NAME}Targets
		LIBRARY  DESTINATION lib${LIB_SUFFIX}
		ARCHIVE  DESTINATION lib${LIB_SUFFIX}
		RUNTIME  DESTINATION bin
		INCLUDES DESTINATION include
	)

	install(FILES pigpiod_if.h pigpiod_if2.h
		DESTINATION include
		PERMISSIONS OWNER_READ OWNER_WRITE
			GROUP_READ
			WORLD_READ
	)

	if(TESTS)
		# x_pigpiod_if
		add_executable(x_pigpiod_if x_pigpiod_if.c)
		target_link_libraries(x_pigpiod_if pigpiod_if RT::RT Threads::Threads)

		# x_pigpiod_if2
		add_executable(x_pigpiod_if2 x_pigpiod_if2.c)
		target_link_libraries(x_pigpiod_if2 pigpiod_if2 RT::RT Threads::Threads)
	endif()
endif()

if(PIG2VCD)
	# pig2vcd
	add_executable(pig2vcd pig2vcd.c command.c)
	target_link_libraries(pig2vcd Threads::Threads)

	install(TARGETS pig2vcd
		EXPORT ${PROJECT_NAME}Targets
		LIBRARY  DESTINATION lib${LIB_SUFFIX}
		ARCHIVE  DESTINATION lib${LIB_SUFFIX}
		RUNTIME  DESTINATION bin
		INCLUDES DESTINATION include
	)
endif()

# Configure and install project

include (GenerateExportHeader)
include (CMakePackageConfigHelpers)

generate_export_header(${PROJECT_NAME})

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
	VERSION ${${PROJECT_NAME}_VERSION}
	COMPATIBILITY AnyNewerVersion
)

export(EXPORT ${PROJECT_NAME}Targets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake"
  NAMESPACE pigpio::
)

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

if (MAN_PAGES)
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
endif()

# Install python modules.
find_package(Python COMPONENTS Interpreter QUIET)

if(Python_FOUND)
	install(CODE "execute_process(COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/setup.py install --prefix=${CMAKE_INSTALL_PREFIX})")
endif()

# package project

include (CPack)
