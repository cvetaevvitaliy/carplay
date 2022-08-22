
#begin of CMakeLists.txt
cmake_minimum_required( VERSION 2.8.10 )
cmake_policy( SET CMP0004 OLD )

#setting module name, must use the same value if parent module exists
set ( MODULE_NAME plugin-iap2 )
set( IAP2_DEMO iAP2Demo )
#set( IAP2_TEST test )
set( IAP2_AUTH auth )

#setting module version variable
set( CMAKE_INSTALL_RPATH_USE_LINK_PATH on )
set( CMAKE_VERBOSE_MAKEFILE off  )

set( SOVERSION            0                                    )
set( VERSION              "${SOVERSION}.0"                   )
#set( PREFIX               ${CMAKE_INSTALL_PREFIX}              )
#set( EXEC_PREFIX          "\${prefix}"                         )
#set( LIBDIR               "\${prefix}/lib"                     )
#set( INCLUDEDIR           "\${prefix}/include/media-hub2"      )

#checking libraries dependency
include( FindPkgConfig )

pkg_check_modules( ${MODULE_NAME} REQUIRED
	#libiconv
	glib-2.0
	gio-2.0
	gobject-2.0
	libmedia-hub2
	libudev
	alsa
	libkmod
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

#set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${${MODULE_NAME}_EXTRA_CFLAGS}" )
if ( B511 )	
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${${MODULE_NAME}_EXTRA_CFLAGS} -fPIC -DB511=1" )
else( B511 )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${${MODULE_NAME}_EXTRA_CFLAGS} -fPIC" )
IF(GCOV)
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
ENDIF()
endif( B511 )

set(
	IAP2_LIB_SOURCE
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Link/iAP2FileTransfer.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Link/iAP2LinkAccessory.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Link/iAP2Link.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Link/iAP2LinkRunLoop.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Link/iAP2Packet.c

	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Utility/iAP2BuffPool.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Utility/iAP2FSM.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Utility/iAP2ListArray.c

	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2UtilityImplementation/iAP2BuffPoolImplementation.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2UtilityImplementation/iAP2Log.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2UtilityImplementation/iAP2Time.c

	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Components/ama_ipod_drm.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Components/iAP2AuthAndIdentify.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Components/iAP2ControlSession.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Components/iAP2Media.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Components/iAP2FileSession.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Components/iAP2Hid.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Components/iAP2I2c.c

	${CMAKE_CURRENT_SOURCE_DIR}/src/idev.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/transport.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/dev_iap1.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/dev_iap2.c
)

#adding local include directories
include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Utility
	${CMAKE_CURRENT_SOURCE_DIR}/src/include
	${CMAKE_CURRENT_SOURCE_DIR}/src/inc
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Components
	${CMAKE_CURRENT_SOURCE_DIR}/src/iAP2Link
	${CMAKE_CURRENT_SOURCE_DIR}/src/
		${GST_INCLUDE_DIRS}
)

#adding library source code
add_library(
	${MODULE_NAME}
	MODULE
	${IAP2_LIB_SOURCE}
)

#adding custom target

#adding dependency

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
		${GST_LIBRARIES}
)

#installing module library
install(
	TARGETS
	${MODULE_NAME}                                      
	DESTINATION     lib/media-hub2                        
)

#installing module header file
#installing module pc file

