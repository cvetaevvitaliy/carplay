#begin of CMakeLists.txt
cmake_minimum_required( VERSION 2.8.10 )
cmake_policy( SET CMP0004 OLD )

#setting module name, must use the same value if parent module exists
set ( MODULE_NAME mh2-plugins )

#setting module version variable
set( CMAKE_INSTALL_RPATH_USE_LINK_PATH on )
set( CMAKE_VERBOSE_MAKEFILE off  )

#checking libraries dependency
include( FindPkgConfig )

pkg_check_modules( ${MODULE_NAME} REQUIRED
	#libiconv
	glib-2.0
	gio-2.0
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
#checking sub modules existence, each module must be separated by a semicolon
if( PLUGIN_STORAGE)
	ADD_SUBDIRECTORY("storage")
endif( PLUGIN_STORAGE)

if( PLUGIN_IDEV)
	ADD_SUBDIRECTORY("idev")
endif(PLUGIN_IDEV)

if(PLUGIN_CARPLAY)
	ADD_SUBDIRECTORY("carplay")
endif(PLUGIN_CARPLAY)

if( PLUGIN_CARLIFE)
	ADD_SUBDIRECTORY("carlife")
endif(PLUGIN_CARLIFE)

#set( SUB_MODULES "storage;idev;carlife;carplay" )
#foreach( M ${SUB_MODULES} )
#		ADD_SUBDIRECTORY( ${M} )
#endforeach( M )

#adding a custom archive target
add_custom_target(
	archive
	tar -czf libmh2-plugins.tar.gz
		${CMAKE_INSTALL_PREFIX}/lib/media-hub2/*
)
