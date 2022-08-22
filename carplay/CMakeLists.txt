#begin of CMakeLists.txt
cmake_minimum_required( VERSION 2.8.10 )
cmake_policy( SET CMP0004 OLD )

set ( MODULE_NAME plugin-carplay)
#setting module version variable
set( CMAKE_INSTALL_RPATH_USE_LINK_PATH on )
set( CMAKE_VERBOSE_MAKEFILE off  )

include( FindPkgConfig )
pkg_check_modules( ${MODULE_NAME} REQUIRED
	#libiconv
	glib-2.0
	gio-2.0
	gobject-2.0
	gstreamer-1.0
	libmedia-hub2
	libCoreUtils
	libAirPlay
	libAirPlaySupport
	libAudioStream
	libScreenStream
)


foreach( CFLAG ${${MODULE_NAME}_CFLAGS} )
	set( ${MODULE_NAME}_EXTRA_CFLAGS "${${MODULE_NAME}_EXTRA_CFLAGS} ${CFLAG}" )
endforeach( CFLAG )

#adding link parameters from libraries dependency
foreach( LDFLAG ${${MODULE_NAME}_LDFLAGS} )
	set( ${MODULE_NAME}_EXTRA_LDFLAGS "${${MODULE_NAME}_EXTRA_LDFLAGS} ${LDFLAG}" )
endforeach( LDFLAG )

if ( B511 )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${${MODULE_NAME}_EXTRA_CFLAGS} -fPIC -std=gnu99 -D__STDC_LIMIT_MACROS=1 -DB511=1" )
else( B511 )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${${MODULE_NAME}_EXTRA_CFLAGS} -fPIC -std=gnu99 -D__STDC_LIMIT_MACROS=1" )
endif( B511 )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -DCFCOMPAT_NOTIFICATIONS_ENABLED=0 -DCFL_BINARY_PLISTS=1 -DAIRPLAY_THREADED_MAIN=1" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCFLITE_ENABLED=1 -DDEBUG_CF_OBJECTS_ENABLED=1 -DDEBUG_EXPORT_ERROR_STRINGS=1" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTARGET_HAS_MD5_UTILS=1 -DAES_UTILS_HAS_GLADMAN_GCM=0 -DAES_UTILS_USE_GLADMAN_AES=1 -DGNU_MP=1" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTARGET_HAS_SHA_UTILS=1 -DTARGET_NO_OPENSSL=1 -DUSE_VIA_ACE_IF_PRESENT=0" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DNS_SD_LIBDISPATCH=0 -DASYNC_CONNECTION_BONJOUR=0 -DBONJOUR_ENABLED=0" ) 
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DAIRPLAY_DEVICE_NAME=\"\\\"Media-Hub v2.0\\\"\"" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DAIRPLAY_HTTP_SERVER_LEGACY" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTARGET_OS_POSIX  ")
IF(GCOV)
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
ENDIF()
set(
	CARPLAY_LIB_SOURCE
	${CMAKE_CURRENT_SOURCE_DIR}/src/dev_carplay.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/carplay.c
)


include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}/
)
add_library(
	${MODULE_NAME}
	MODULE
	${CARPLAY_LIB_SOURCE}
)

target_link_libraries(
	${MODULE_NAME}
	${${MODULE_NAME}_EXTRA_LDFLAGS}
	"-ldns_sd"
)

install(
	TARGETS
	${MODULE_NAME}
	DESTINATION     lib/media-hub2
)

install(
	FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/carplay.ini
	DESTINATION		/etc
)
