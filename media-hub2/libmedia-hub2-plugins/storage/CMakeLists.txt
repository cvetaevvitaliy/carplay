#begin of CMakeLists.txt
cmake_minimum_required( VERSION 2.8.10 )
cmake_policy( SET CMP0004 OLD )

#setting module name, must use the same value if parent module exists
set ( MODULE_NAME plugin-storage )

#setting module version variable
set( CMAKE_INSTALL_RPATH_USE_LINK_PATH on )
set( CMAKE_VERBOSE_MAKEFILE off  )

set( SOVERSION            0                                    )
set( VERSION              "${SOVERSION}.0"                   )

#checking libraries dependency
include( FindPkgConfig )

pkg_check_modules( ${MODULE_NAME} REQUIRED
	#libiconv
	libmedia-hub2
	libudev
	blkid
	taglib
	gstreamer-1.0
)

#adding compile parameters from libraries dependency
foreach( CFLAG ${${MODULE_NAME}_CFLAGS} )
	set( ${MODULE_NAME}_EXTRA_CFLAGS "${${MODULE_NAME}_EXTRA_CFLAGS} ${CFLAG}" )
endforeach( CFLAG )

#adding link parameters from libraries dependency
foreach( LDFLAG ${${MODULE_NAME}_LDFLAGS} )
	set( ${MODULE_NAME}_EXTRA_LDFLAGS "${${MODULE_NAME}_EXTRA_LDFLAGS} ${LDFLAG}" )
endforeach( LDFLAG )

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${${MODULE_NAME}_EXTRA_CFLAGS} -fPIC" )

IF(GCOV)
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
ENDIF()
#adding local include directories
include_directories (
	./include
)

#adding library source code
add_library(
	${MODULE_NAME}
	MODULE
	${CMAKE_CURRENT_SOURCE_DIR}/src/storage.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/dev_storage.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unicode2pinyin.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/hanziPinyinInitial.c
)

#setting local target version
#set_target_properties(
#	${MODULE_NAME}
#	PROPERTIES VERSION ${VERSION}
#	SOVERSION ${SOVERSION}
#)

#setting local target link parameters
target_link_libraries(
	${MODULE_NAME}
	${${MODULE_NAME}_EXTRA_LDFLAGS}
	tag_c
)

#installing module library
install(
	TARGETS
	${MODULE_NAME}
	DESTINATION		lib/media-hub2
)

