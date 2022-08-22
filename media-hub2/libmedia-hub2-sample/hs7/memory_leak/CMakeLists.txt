#begin of CMakeLists.txt
cmake_minimum_required( VERSION 2.8 )
cmake_policy( SET CMP0004 OLD )

#setting module name, must use the same value if parent module exists
set ( MODULE_NAME mh-sample-hs7 )

ADD_DEFINITIONS(-D_MULTI_BLE_)
ADD_DEFINITIONS(-D_MULTI_HFP_)

#setting module version variable
set( CMAKE_VERBOSE_MAKEFILE off  )


#checking libraries dependency
include( FindPkgConfig )
pkg_check_modules( ${MODULE_NAME} REQUIRED
	#libiconv
	libmedia-hub2
	libmedia-hub2-misc
	glib-2.0
	ilmControl
	ilm
	ilmInput
	libudev
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
	${CMAKE_CURRENT_SOURCE_DIR}/sample.c
)


#adding local include directories
include_directories (
)

#setting local target link parameters
target_link_libraries(
	${MODULE_NAME}
	${${MODULE_NAME}_EXTRA_LDFLAGS}
)

#installing module executable
install(
	TARGETS
	${MODULE_NAME}
	DESTINATION		bin
)


