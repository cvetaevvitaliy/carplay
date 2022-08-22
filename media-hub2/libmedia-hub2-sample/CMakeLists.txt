#begin of CMakeLists.txt
cmake_minimum_required( VERSION 2.8 )
cmake_policy( SET CMP0004 OLD )

#setting module name, must use the same value if parent module exists
ADD_DEFINITIONS(-D_MULTI_BLE_)
ADD_DEFINITIONS(-D_MULTI_HFP_)
set ( MODULE_NAME mh-sample )

#setting module version variable
set( CMAKE_VERBOSE_MAKEFILE off  )

if( USE_MH2_IPC )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_MH2_IPC" )
	set( PKG_NAME "${PKG_NAME} libmedia-hub2-ipc-client" )
	set( PKG_NAME "${PKG_NAME} libmedia-hub2-streaming-client" )
else( USE_MH2_IPC )
	set( PKG_NAME "${PKG_NAME} libmedia-hub2" )
endif( USE_MH2_IPC )

set( PKG_NAME "${PKG_NAME} libmedia-hub2-misc")

if( USE_GTK_SAMPLE )
	set( PKG_NAME "${PKG_NAME} gtk+-3.0" )
	set( SAMPLE_NAME "gtk-sample.c" )
else( USE_GTK_SAMPLE )
	set( SAMPLE_NAME "sample.c" )
endif( USE_GTK_SAMPLE )

#checking libraries dependency
include( FindPkgConfig )

if( HS7 )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHS7" )
pkg_check_modules( ${MODULE_NAME} REQUIRED
	#libiconv
	${PKG_NAME}
	glib-2.0
	ilmControl
	ilm
	ilmInput
	libudev
		libmedia-hub2-misc
)
else( HS7 )
pkg_check_modules( ${MODULE_NAME} REQUIRED
	#libiconv
	${PKG_NAME}
	glib-2.0
#	ilmControl
#	ilm
#	ilmInput
	libudev
		libmedia-hub2-misc
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

ADD_SUBDIRECTORY( server )
#add_executable(
#	${MODULE_NAME}-carlife
#	${CMAKE_SOURCE_DIR}/src/carlife.c
#)

#adding executable source code
add_executable(
	${MODULE_NAME}
	${CMAKE_SOURCE_DIR}/src/streaming.c
)

#adding executable source code
#adding local include directories
include_directories (
)

#setting local target link parameters
target_link_libraries(
	${MODULE_NAME}
	${${MODULE_NAME}_EXTRA_LDFLAGS}
)

#target_link_libraries(
#	${MODULE_NAME}-carlife
#	${${MODULE_NAME}_EXTRA_LDFLAGS}
#)
#installing module executable
install(
	TARGETS
	${MODULE_NAME}
	DESTINATION		bin
)

#install(
#	TARGETS
#	${MODULE_NAME}-carlife
#	DESTINATION	    bin
#)
if( HS7 )
	ADD_SUBDIRECTORY( hs7)
else( HS7 )
endif( HS7 )


#installing module library

#installing module header file

#installing module pc file
add_executable(
	${MODULE_NAME}-favorite
	${CMAKE_SOURCE_DIR}/src/favorite.c
)

target_link_libraries(
	${MODULE_NAME}-favorite
	${${MODULE_NAME}_EXTRA_LDFLAGS}
)
install(
	TARGETS
	${MODULE_NAME}-favorite
	DESTINATION	    bin
)

add_executable(
	${MODULE_NAME}-carplay
	${CMAKE_SOURCE_DIR}/src/carplay.c
)
if( HS7 )
target_link_libraries(
	${MODULE_NAME}-carplay
	${${MODULE_NAME}_EXTRA_LDFLAGS}
	aac
	AnwBTCore
	anwbtif
	anwbtstack
	AnwManagerM
	AnwMsg
	anwsbc
	anwsound
	Anwwbxml
	btattach
	BTControl
	bt-vendor
	HciTransport
	SysApi
	wave
	dbus-1
	
)
else( HS7 )
#target_include_directories(${MODULE_NAME}-carplay PUBLIC /home/vitaliy/just-test/media-hub2/libmedia-hub2/include)
#message( ${MODULE_NAME}-carplay ${${MODULE_NAME}_EXTRA_LDFLAGS} -lmedia-hub2-misc)
#link_directories(${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(
	${MODULE_NAME}-carplay
	${${MODULE_NAME}_EXTRA_LDFLAGS}
#	aac
#	AnwBTCore
#	anwbtif
#	anwbtstack
#	AnwManagerM
#	AnwMsg
#	anwsbc
#	anwsound
#	Anwwbxml
#	btattach
#	BTControl
#	bt-vendor
#	HciTransport
#	SysApi
#	wave
#	dbus-1
		media-hub2-misc
		media-hub2-ipc-client
)
endif( HS7 )
install(
	TARGETS
	${MODULE_NAME}-carplay
	DESTINATION	    bin
)
