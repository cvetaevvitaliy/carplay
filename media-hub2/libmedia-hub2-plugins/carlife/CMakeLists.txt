#begin of CMakeLists.txt
cmake_minimum_required( VERSION 2.8.10 )
cmake_policy( SET CMP0004 OLD )

#setting module name, must use the same value if parent module exists
set ( MODULE_NAME plugin-carlife )

set( CMAKE_VERBOSE_MAKEFILE off  )

if( EXISTS ${CMAKE_SOURCE_DIR}/../.svn )
	FIND_PACKAGE( Subversion )
	if( SUBVERSION_FOUND )
		Subversion_WC_INFO( ${CMAKE_SOURCE_DIR}/../ ${MODULE_NAME} )
		set( REVERSION ${${MODULE_NAME}_WC_REVISION} )
	endif( SUBVERSION_FOUND)
else( EXISTS ${CMAKE_SOURCE_DIR}/../.svn )
	set( REVERSION 0 )
endif( EXISTS ${CMAKE_SOURCE_DIR}/../.svn )

set( SOVERSION            0                                    )
set( VERSION              "${SOVERSION}.${REVERSION}"                   )
set( PREFIX               ${CMAKE_INSTALL_PREFIX}              )
set( EXEC_PREFIX          "\${prefix}"                         )
set( LIBDIR               "\${prefix}/lib"                     )
set( INCLUDEDIR           "\${prefix}/include"      )
#checking libraries dependency
include( FindPkgConfig )

pkg_check_modules( ${MODULE_NAME} REQUIRED
	#libiconv
	libmedia-hub2
	gstreamer-1.0
	libcarlife_c
	glib-2.0
	gobject-2.0
	gio-2.0
	libudev
	blkid
	libusb-1.0
)

#adding compile parameters from libraries dependency
foreach( CFLAG ${${MODULE_NAME}_CFLAGS} )
	set( ${MODULE_NAME}_EXTRA_CFLAGS "${${MODULE_NAME}_EXTRA_CFLAGS} ${CFLAG}" )
endforeach( CFLAG )

#adding link parameters from libraries dependency
foreach( LDFLAG ${${MODULE_NAME}_LDFLAGS} )
	set( ${MODULE_NAME}_EXTRA_LDFLAGS "${${MODULE_NAME}_EXTRA_LDFLAGS} ${LDFLAG}" )
endforeach( LDFLAG )
#$(SDKTARGETSYSROOT)
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${${MODULE_NAME}_EXTRA_CFLAGS} -fPIC " )
IF(GCOV)
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
ENDIF()
#--sysroot=/root/AMA/targets/XENIA/rootfs
#-L/root/AMA/targets/XENIA/rootfs/usr/lib -lprotobuf
set( CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} ${${MODULE_NAME}_EXTRA_CFLAGS} -fPIC -Wreturn-type")
#adding local include directories
include_directories (

	./include
	./bind/include
	./CarLifeVehicleLib-8th-December-2016/CarLifeLibSourceCode/trunk/include
)

#adding library source code
add_library(
	${MODULE_NAME}
	MODULE
	${CMAKE_CURRENT_SOURCE_DIR}/src/dev_carlife.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/carlife.c
)
	
#setting local target version
#set_target_properties(
#	${MODULE_NAME}
#	PROPERTIES VERSION ${VERSION}
#	SOVERSION ${SOVERSION}
#)

#setting local target link parameters
#target_link_libraries(
#	${MODULE_NAME}
#	${${MODULE_NAME}_EXTRA_LDFLAGS}
#	tag_c
#)

#installing module library

target_link_libraries(
	${MODULE_NAME}
	${${MODULE_NAME}_EXTRA_LDFLAGS}
)
install(
	TARGETS
	${MODULE_NAME}
	DESTINATION		lib/media-hub2
)

install(
	FILES
	${CMAKE_CURRENT_SOURCE_DIR}/bdim
	${CMAKE_CURRENT_SOURCE_DIR}/bdim.jar
	${CMAKE_CURRENT_SOURCE_DIR}/bdsc/bdsc16
	${CMAKE_CURRENT_SOURCE_DIR}/bdsc/bdsc17
	${CMAKE_CURRENT_SOURCE_DIR}/bdsc/bdsc18
	${CMAKE_CURRENT_SOURCE_DIR}/bdsc/bdsc19
	${CMAKE_CURRENT_SOURCE_DIR}/bdsc/bdsc19_01
	DESTINATION		/etc
)
