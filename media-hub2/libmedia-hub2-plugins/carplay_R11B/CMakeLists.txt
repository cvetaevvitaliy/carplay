
#begin of CMakeLists.txt
cmake_minimum_required( VERSION 2.8.10 )
cmake_policy( SET CMP0004 OLD )

#setting module name, must use the same value if parent module exists
set ( MODULE_NAME plugin-carplay )

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
#	libusb-1.0
	libmedia-hub2
		gstreamer-1.0>=1.4
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
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${${MODULE_NAME}_EXTRA_CFLAGS} -fPIC -std=gnu99 -D__STDC_LIMIT_MACROS=1" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -DCFCOMPAT_NOTIFICATIONS_ENABLED=0 -DCFL_BINARY_PLISTS=1 -DAIRPLAY_THREADED_MAIN=1" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCFLITE_ENABLED=1 -DDEBUG_CF_OBJECTS_ENABLED=1 -DDEBUG_EXPORT_ERROR_STRINGS=1" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTARGET_HAS_MD5_UTILS=1 -DAES_UTILS_HAS_GLADMAN_GCM=0 -DAES_UTILS_USE_GLADMAN_AES=1" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTARGET_HAS_SHA_UTILS=1 -DTARGET_NO_OPENSSL=1 -DUSE_VIA_ACE_IF_PRESENT=0" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DAIRPLAY_DEVICE_NAME=\"\\\"Media-Hub v2.0\\\"\" -DMFI_AUTH_DEVICE_PATH=\"\\\"/dev/i2c-1\\\"\"" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DAIRPLAY_CONFIG_FILE_PATH=\"\\\"/etc/carplay.ini\\\"\"" )

set(
	CARPLAY_LIB_SOURCE
	${CMAKE_CURRENT_SOURCE_DIR}/src/carplay.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/dev_carplay.c

	${CMAKE_CURRENT_SOURCE_DIR}/src/External/AppleLossless/ag_dec.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/External/AppleLossless/AppleLosslessDecoder.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/External/AppleLossless/BitUtilities.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/External/AppleLossless/dp_dec.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/External/AppleLossless/matrix_dec.c

	${CMAKE_CURRENT_SOURCE_DIR}/src/External/Curve25519/curve25519-donna.c

	${CMAKE_CURRENT_SOURCE_DIR}/src/External/GladmanAES/aescrypt.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/External/GladmanAES/aeskey.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/External/GladmanAES/aes_modes.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/External/GladmanAES/aestab.c

	${CMAKE_CURRENT_SOURCE_DIR}/src/Sources/AirPlayMain.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sources/AirPlayNTPClient.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sources/AirPlayReceiverPOSIX.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sources/AirPlayReceiverServer.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sources/AirPlayReceiverSession.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sources/AirPlayReceiverSessionScreen.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sources/AirPlaySettings.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sources/AirPlayUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sources/AirTunesClock.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sources/AirTunesServer.c

	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/AESUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/APSDebugServices.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/APSMFiSAP.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/APSMFiServerPlatformLinux.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/AsyncConnection.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/AudioConverterLite.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/AudioUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/AudioUtilsStub.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/Base64Utils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/CFCompat.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/CFLiteBinaryPlist.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/CFLite.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/CFLitePreferencesFile.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/CFLiteRunLoopSelect.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/CFUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/CommandLineUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/DataBufferUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/DebugIPCUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/DispatchLite.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/HIDUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/HIDUtilsVirtual.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/HTTPClient.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/HTTPMessage.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/HTTPNetUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/HTTPServer.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/HTTPUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/MathUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/MD5Utils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/MiscUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/NetPerf.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/NetUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/NetworkChangeListener.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/NTPUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/RandomNumberUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/ScreenUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/ScreenUtilsStub.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/SHAUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/StringUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/SystemUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/ThreadUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/TickUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/TimeUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/URLUtils.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/utfconv.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support/UUIDUtils.c
)

#adding local include directories
include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}/src/Support
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sources
	${CMAKE_CURRENT_SOURCE_DIR}/src/External/GladmanAES
	${CMAKE_CURRENT_SOURCE_DIR}/src/External/Curve25519
	${CMAKE_CURRENT_SOURCE_DIR}/src/External/AppleLossless

	${CMAKE_CURRENT_SOURCE_DIR}/src
		${GST_INCLUDE_DIRS}
)

#adding library source code
add_library(
	${MODULE_NAME}
	MODULE
	${CARPLAY_LIB_SOURCE}
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
	"-ldns_sd"
)

#installing module library
install(
	TARGETS
	${MODULE_NAME}                                      
	DESTINATION     lib/media-hub2                        
)

#installing a custom file
install(
	FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/carplay.ini
	DESTINATION		/etc
)

#installing module header file
#installing module pc file

