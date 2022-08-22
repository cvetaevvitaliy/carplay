#begin of CMakeLists.txt
cmake_minimum_required( VERSION 2.8 )
cmake_policy( SET CMP0004 OLD )

#setting module name, must use the same value if parent module exists
set ( MODULE_NAME collection )

#setting module version variable
set( CMAKE_VERBOSE_MAKEFILE off  )

#adding library source code
add_library(
	${MODULE_NAME}
	STATIC
	src/collection.c
	src/playlist.c
)

#adding executable source code

#adding local include directories
include_directories (
	../include
	../contents/src/
)

