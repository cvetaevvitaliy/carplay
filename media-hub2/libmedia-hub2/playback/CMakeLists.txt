#begin of CMakeLists.txt
cmake_minimum_required( VERSION 2.8 )
cmake_policy( SET CMP0004 OLD )

#setting module name, must use the same value if parent module exists
set ( MODULE_NAME playback )

#setting module version variable
set( CMAKE_VERBOSE_MAKEFILE off  )

#adding library source code
add_library(
	${MODULE_NAME}
	STATIC
	src/playback.c
	src/pause.c
	src/playing.c
	src/ready.c
	src/seeking.c
	src/state.c
	src/switching.c
	src/error.c
)

#adding executable source code

#adding local include directories
include_directories (
	../include
	../collection/src
)

#setting local target link parameters
target_link_libraries(
	${MODULE_NAME}
	${${MODULE_NAME}_EXTRA_LDFLAGS}
)
