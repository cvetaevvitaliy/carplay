#begin of CMakeLists.txt
cmake_minimum_required( VERSION 2.8 )
cmake_policy( SET CMP0004 OLD )

#setting module name, must use the same value if parent module exists
set ( MODULE_NAME misc )

#setting module version variable
set( CMAKE_VERBOSE_MAKEFILE off  )

#checking libraries dependency
include( FindPkgConfig )

pkg_check_modules( ${MODULE_NAME} REQUIRED
	#libiconv
)

#adding library source code
add_library(
	${MODULE_NAME}
	STATIC
	src/object.c
	src/log.c
	src/memory.c
	src/misc.c
)

#adding local include directories
include_directories (
	../include
)


