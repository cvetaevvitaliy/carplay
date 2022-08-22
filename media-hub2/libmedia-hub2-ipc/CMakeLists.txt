#begin of CMakeLists.txt
cmake_minimum_required( VERSION 2.8 )
cmake_policy( SET CMP0004 OLD )

#setting module name, must use the same value if parent module exists
set ( MODULE_NAME media-hub2-ipc )

#setting module version variable
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

set(CMAKE_INSTALL_PREFIX "/usr")
set( SOVERSION            0                                    )
set( VERSION              "${SOVERSION}.${REVERSION}"                   )
set( PREFIX               ${CMAKE_INSTALL_PREFIX}              )
set( EXEC_PREFIX          "\${prefix}"                         )
set( LIBDIR               "\${prefix}/lib"                     )
set( INCLUDEDIR           "\${prefix}/include"      )

#if( X86_64 )
#	set( DBUS_SOURCE "mh_dbus_x64.c" )
#else( X86_64 )
#	set( DBUS_SOURCE "mh_dbus.c" )
#endif( X86_64 )
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set( DBUS_SOURCE "mh_dbus_x64.c" )
else(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set( DBUS_SOURCE "mh_dbus.c")
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)
#checking libraries dependency
include( FindPkgConfig )

pkg_check_modules( ${MODULE_NAME} REQUIRED
	#libiconv
	glib-2.0
	gio-unix-2.0
	gobject-2.0
	libmedia-hub2
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

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${${MODULE_NAME}_EXTRA_CFLAGS} -fPIC -Wimplicit-function-declaration -Wreturn-type" )

IF(GCOV)
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
ENDIF()

#adding library source code
add_library(
	${MODULE_NAME}-client
	SHARED
	${CMAKE_SOURCE_DIR}/client/client.c
	${CMAKE_SOURCE_DIR}/client/object.c
	${CMAKE_SOURCE_DIR}/client/core.c
	${CMAKE_SOURCE_DIR}/client/dev.c
	${CMAKE_SOURCE_DIR}/client/folder.c
	${CMAKE_SOURCE_DIR}/client/item.c
	${CMAKE_SOURCE_DIR}/client/playlist.c
	${CMAKE_SOURCE_DIR}/client/playback.c
	${CMAKE_SOURCE_DIR}/client/collection.c
	${CMAKE_SOURCE_DIR}/client/filter.c
	${CMAKE_SOURCE_DIR}/client/misc.c
	${CMAKE_SOURCE_DIR}/client/carplay.c
	${CMAKE_SOURCE_DIR}/codegen/${DBUS_SOURCE}
	${CMAKE_SOURCE_DIR}/builttime.c
	${CMAKE_SOURCE_DIR}/client/carlife.c
)

set_target_properties(
	${MODULE_NAME}-client
	PROPERTIES VERSION ${VERSION}
	SOVERSION ${SOVERSION}
)

add_library(
	${MODULE_NAME}-server
	SHARED
	${CMAKE_SOURCE_DIR}/server/server.c
	${CMAKE_SOURCE_DIR}/codegen/${DBUS_SOURCE}

)

set_target_properties(
	${MODULE_NAME}-server
	PROPERTIES VERSION ${VERSION}
	SOVERSION ${SOVERSION}
)
#adding executable source code
#adding local include directories
include_directories (
	include
	codegen
)

#setting local target link parameters
target_link_libraries(
	${MODULE_NAME}-client
	${${MODULE_NAME}_EXTRA_LDFLAGS}
	"rt"
)

target_link_libraries(
	${MODULE_NAME}-server
	${${MODULE_NAME}_EXTRA_LDFLAGS}
	"rt"
)

#adding package configuration file
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/pkg-config/lib${MODULE_NAME}-client.pc.in
	lib${MODULE_NAME}-client.pc
	@ONLY
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/pkg-config/lib${MODULE_NAME}-server.pc.in
	lib${MODULE_NAME}-server.pc
	@ONLY
)
#installing module executable

#installing module library
install(
	TARGETS
	${MODULE_NAME}-client ${MODULE_NAME}-server
	DESTINATION		/usr/lib
)

#installing module header file

#installing module pc file
install( 
	FILES
	${CMAKE_CURRENT_BINARY_DIR}/lib${MODULE_NAME}-client.pc ${CMAKE_CURRENT_BINARY_DIR}/lib${MODULE_NAME}-server.pc
	DESTINATION		/usr/lib/pkgconfig
)
install(
	FILES
	${CMAKE_SOURCE_DIR}/include/mh_client.h
	DESTINATION		/usr/include/

)
add_custom_target(
	archive
	tar -czf lib${MODULE_NAME}.tar.gz 
		${CMAKE_INSTALL_PREFIX}/lib/lib${MODULE_NAME}-client.so
		${CMAKE_INSTALL_PREFIX}/lib/lib${MODULE_NAME}-client.so.${SOVERSION}
		${CMAKE_INSTALL_PREFIX}/lib/lib${MODULE_NAME}-client.so.${SOVERSION}.${REVERSION}
		${CMAKE_INSTALL_PREFIX}/lib/pkgconfig/lib${MODULE_NAME}-client.pc
		${CMAKE_INSTALL_PREFIX}/lib/lib${MODULE_NAME}-server.so
		${CMAKE_INSTALL_PREFIX}/lib/lib${MODULE_NAME}-server.so.${SOVERSION}
		${CMAKE_INSTALL_PREFIX}/lib/lib${MODULE_NAME}-server.so.${SOVERSION}.${REVERSION}
		${CMAKE_INSTALL_PREFIX}/lib/pkgconfig/lib${MODULE_NAME}-server.pc
)
