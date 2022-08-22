#begin of CMakeLists.txt
cmake_minimum_required( VERSION 2.8 )
cmake_policy( SET CMP0004 OLD )

#setting module name, must use the same value if parent module exists
set ( MODULE_NAME media-hub2-carplay )

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

set( SOVERSION            0                                    )
set( VERSION              "${SOVERSION}.${REVERSION}"                   )
set( PREFIX               ${CMAKE_INSTALL_PREFIX}              )
set( EXEC_PREFIX          "\${prefix}"                         )
set( LIBDIR               "\${prefix}/lib"                     )
set( INCLUDEDIR           "\${prefix}/include"      )

#checking libraries dependency
include( FindPkgConfig )

pkg_check_modules( ${MODULE_NAME} REQUIRED
	# libiconv
	glib-2.0
	gio-unix-2.0
	gobject-2.0
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

#adding library source code
add_library(
	${MODULE_NAME}
	SHARED
	${CMAKE_SOURCE_DIR}/src/server.c
	${CMAKE_SOURCE_DIR}/codegen/mh_dbus_carplay.c
)

add_library(
	${MODULE_NAME}-client
	SHARED
	${CMAKE_SOURCE_DIR}/src/client.c
	${CMAKE_SOURCE_DIR}/codegen/mh_dbus_carplay.c
)

set_target_properties(
	${MODULE_NAME}
	PROPERTIES VERSION ${VERSION}
	SOVERSION ${SOVERSION}
)

set_target_properties(
	${MODULE_NAME}-client
	PROPERTIES VERSION ${VERSION}
	SOVERSION ${SOVERSION}
)

#adding executable source codegen
#adding local include directories
include_directories (
	include
	codegen
)

#setting local target link parameters
target_link_libraries(
	${MODULE_NAME}
	${${MODULE_NAME}_EXTRA_LDFLAGS}
	"rt"
)

target_link_libraries(
	${MODULE_NAME}-client
	${${MODULE_NAME}_EXTRA_LDFLAGS}
	"rt"
)

#adding package configuration file
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/pkg-config/lib${MODULE_NAME}.pc.in
	lib${MODULE_NAME}.pc
	@ONLY
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/pkg-config/lib${MODULE_NAME}-client.pc.in
	lib${MODULE_NAME}-client.pc
	@ONLY
)
#installing module executable

#installing module library
install(
	TARGETS
	${MODULE_NAME}
	${MODULE_NAME}-client
	DESTINATION		lib
)

#installing module header FILES
install(
	FILES
	${CMAKE_SOURCE_DIR}/include/mh_carplay.h
	DESTINATION		include/
)

#installing module pc file
install( 
	FILES
	${CMAKE_CURRENT_BINARY_DIR}/lib${MODULE_NAME}.pc 
	${CMAKE_CURRENT_BINARY_DIR}/lib${MODULE_NAME}-client.pc 
	DESTINATION		lib/pkgconfig
)
