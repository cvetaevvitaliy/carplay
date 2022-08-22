#begin of CMakeLists.txt
cmake_minimum_required( VERSION 2.8 )
cmake_policy( SET CMP0004 OLD )

#setting module name, must use the same value if parent module exists
set ( MODULE_NAME mh-sample-server )

#setting module version variable
set( CMAKE_VERBOSE_MAKEFILE off  )

#checking libraries dependency
include( FindPkgConfig )

if( HS7 )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHS7" )
pkg_check_modules( ${MODULE_NAME} REQUIRED
	#libiconv
	glib-2.0
	alsa
	libmedia-hub2-ipc-server
	audiohub
#	libmedia-hub2-streaming
)
else( HS7 )
pkg_check_modules( ${MODULE_NAME} REQUIRED
	#libiconv
	glib-2.0
	alsa
	libmedia-hub2-ipc-server
	libpulse-simple
#	libmedia-hub2-streaming
)
endif( HS7 )


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

#adding executable source code
add_executable(
	${MODULE_NAME}
	#${CMAKE_SOURCE_DIR}/server/server.c
	${CMAKE_SOURCE_DIR}/server/server_with_carplay.c
)
#adding local include directories
include_directories (
)

if( HS7 )
#setting local target link parameters
target_link_libraries(
	${MODULE_NAME}
	${${MODULE_NAME}_EXTRA_LDFLAGS}
)

else( HS7 )
#setting local target link parameters
target_link_libraries(
	${MODULE_NAME}
	${${MODULE_NAME}_EXTRA_LDFLAGS}
	"-lpulse-simple"
)
endif( HS7 )

#installing module executable
install(
	TARGETS
	${MODULE_NAME}
	DESTINATION		bin
)

#installing module library
install(
)

#installing module header file

#installing module pc file
install( 
)
