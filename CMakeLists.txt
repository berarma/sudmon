cmake_minimum_required (VERSION 3.7)
project (SudMon VERSION 0.0.0 DESCRIPTION "SUD library and CLI monitor.")

configure_file(
    "${PROJECT_SOURCE_DIR}/src/ProjectConfig.h.in"
    "${PROJECT_BINARY_DIR}/ProjectConfig.h"
    )

include_directories("${PROJECT_BINARY_DIR}")

add_library(sud SHARED src/sud.cpp src/sud.hpp)
target_link_libraries (sud hidapi-libusb)
set_target_properties(sud
	PROPERTIES VERSION ${PROJECT_VERSION}
	PROPERTIES SOVERSION 0
	PROPERTIES PUBLIC_HEADER src/sud.hpp)

add_executable(sudmon src/main.cpp src/io.cpp src/sud.hpp src/io.hpp)
target_link_libraries (sudmon sud)
target_compile_options(sudmon PUBLIC -Wall -g)

include(GNUInstallDirs)
install(TARGETS sudmon sud
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES 99-sud.rules DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/udev/rules.d COMPONENT config)

