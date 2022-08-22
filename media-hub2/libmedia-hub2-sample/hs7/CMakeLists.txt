#begin of CMakeLists.txt
cmake_minimum_required( VERSION 2.8 )
cmake_policy( SET CMP0004 OLD )

#setting module name, must use the same value if parent module exists
set ( MODULE_NAME mh-sample-carlife-hs7 )
set ( MODULE_SWITCH mh-sample-switch)

ADD_DEFINITIONS(-D_MULTI_BLE_)
ADD_DEFINITIONS(-D_MULTI_HFP_)

#setting module version variable
set( CMAKE_VERBOSE_MAKEFILE off  )

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_MH2_IPC" )
set( PKG_NAME "${PKG_NAME} libmedia-hub2-ipc-client" )
#ADD_SUBDIRECTORY( memory_leak )

#checking libraries dependency
include( FindPkgConfig )
pkg_check_modules( ${MODULE_NAME} REQUIRED
	#libiconv
	${PKG_NAME}
	glib-2.0
	#ilmControl
	#ilm
	#ilmInput
	libudev
	#app-rpc
	#libama-base
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

add_executable(
	${MODULE_NAME}
	${CMAKE_CURRENT_SOURCE_DIR}/src/carlife_hs7.c
)
add_executable(
	${MODULE_SWITCH}
	${CMAKE_CURRENT_SOURCE_DIR}/src/switch.c
)


target_link_libraries(
	${MODULE_SWITCH}
	${${MODULE_NAME}_EXTRA_LDFLAGS}
)

#adding local include directories
include_directories (
)

#setting local target link parameters
target_link_libraries(
	${MODULE_NAME}
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
)

#installing module executable
install(
	TARGETS
	${MODULE_NAME}
	DESTINATION		bin
)

install(
	TARGETS
	${MODULE_SWITCH}
	DESTINATION	    bin
)

