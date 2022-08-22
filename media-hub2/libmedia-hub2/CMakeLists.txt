#egin of CMakeLists.txt
cmake_minimum_required( VERSION 2.8.10 )
cmake_policy( SET CMP0004 OLD )

#setting module name, must use the same value if parent module exists
set ( MODULE_NAME media-hub2 )

#setting module version variable
set( CMAKE_INSTALL_RPATH_USE_LINK_PATH on )
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
set( VERSION              "${SOVERSION}.${REVERSION}"          )
set( PREFIX               ${CMAKE_INSTALL_PREFIX}              )
set( EXEC_PREFIX          "\${prefix}"                         )
set( LIBDIR               "\${prefix}/lib"                     )
set( INCLUDEDIR           "\${prefix}/include"      )

#checking libraries dependency
include( FindPkgConfig )

pkg_check_modules( ${MODULE_NAME} REQUIRED
	#libiconv
	glib-2.0
	gio-2.0
	gobject-2.0
	gstreamer-1.0
	taglib_c
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
set( SUB_MODULES "collection;contents;core;io;playback;player;misc;dev" )
set( CMAKE_LISTS_NAME "CMakeLists.txt" )

foreach( M ${SUB_MODULES} )
#checking if we have the source code
	if( EXISTS ${CMAKE_SOURCE_DIR}/${M}/${CMAKE_LISTS_NAME} )
		ADD_SUBDIRECTORY( ${M} )
		set( LOCAL_MODULES "${LOCAL_MODULES};${M}" )
#else we will use the prebuilt library
	else( EXISTS ${CMAKE_SOURCE_DIR}/${M}/${CMAKE_LISTS_NAME} )
		set( PREBUILT_MODULES "${PREBUILT_MODULES};${CMAKE_SOURCE_DIR}/lib/lib${M}.a" )
	endif( EXISTS ${CMAKE_SOURCE_DIR}/${M}/${CMAKE_LISTS_NAME} )
endforeach( M )

#adding executable source cod

#adding local include directories
include_directories (
	./include
)

#adding library source code
add_library(
	${MODULE_NAME}
	SHARED
	${CMAKE_SOURCE_DIR}/builttime.c
)

#adding custom target
add_custom_target(
	builttime
	touch ${CMAKE_SOURCE_DIR}/builttime.c
)

#adding dependency
add_dependencies(
	${MODULE_NAME}
	builttime
)

#setting local target version
set_target_properties(
	${MODULE_NAME}
	PROPERTIES VERSION ${VERSION}
	SOVERSION ${SOVERSION}
)

#setting local target link parameters
target_link_libraries(
	${MODULE_NAME}
	"-Wl,--whole-archive"
	${LOCAL_MODULES}
	${PREBUILT_MODULES}
	"-Wl,--no-whole-archive"
	${${MODULE_NAME}_EXTRA_LDFLAGS}
)

#adding package configuration file
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/pkg-config/lib${MODULE_NAME}.pc.in
	lib${MODULE_NAME}.pc
	@ONLY
)

#installing module executable

#installing module library
install(
	TARGETS
	${MODULE_NAME}
	DESTINATION		/usr/lib
)

#installing module header file
install(
	FILES
	${CMAKE_SOURCE_DIR}/include/mh_api.h
	${CMAKE_SOURCE_DIR}/include/mh_io.h
	${CMAKE_SOURCE_DIR}/include/mh_dev.h
	${CMAKE_SOURCE_DIR}/include/mh_core.h
	${CMAKE_SOURCE_DIR}/include/mh_folder.h
	${CMAKE_SOURCE_DIR}/include/mh_item.h
	${CMAKE_SOURCE_DIR}/include/mh_playlist.h
	${CMAKE_SOURCE_DIR}/include/mh_filter.h
	${CMAKE_SOURCE_DIR}/include/mh_pb.h
	${CMAKE_SOURCE_DIR}/include/mh_col.h
	${CMAKE_SOURCE_DIR}/include/mh_contents.h
	${CMAKE_SOURCE_DIR}/include/mh_music.h
	${CMAKE_SOURCE_DIR}/include/mh_picture.h
	${CMAKE_SOURCE_DIR}/include/mh_movie.h
	${CMAKE_SOURCE_DIR}/include/mh_misc.h
	${CMAKE_SOURCE_DIR}/include/mh_player.h
	${CMAKE_SOURCE_DIR}/include/mh_carplay.h
	${CMAKE_SOURCE_DIR}/include/mh_carlife.h
	DESTINATION		include/
)

#installing module pc file
install( 
	FILES
	${CMAKE_CURRENT_BINARY_DIR}/lib${MODULE_NAME}.pc       # from
	DESTINATION		/usr/lib/pkgconfig/                                  # to
)

#adding a custom archive target
add_custom_target(
	archive
	tar -czf lib${MODULE_NAME}.tar.gz 
		${CMAKE_INSTALL_PREFIX}/include/mh_api.h
		${CMAKE_INSTALL_PREFIX}/lib/lib${MODULE_NAME}.so
		${CMAKE_INSTALL_PREFIX}/lib/lib${MODULE_NAME}.so.${SOVERSION}
		${CMAKE_INSTALL_PREFIX}/lib/lib${MODULE_NAME}.so.${SOVERSION}.${REVERSION}
		${CMAKE_INSTALL_PREFIX}/lib/pkgconfig/lib${MODULE_NAME}.pc
)
