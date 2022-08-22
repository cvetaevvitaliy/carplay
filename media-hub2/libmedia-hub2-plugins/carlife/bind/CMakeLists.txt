#begin of CMakeLists.txt
cmake_minimum_required( VERSION 2.8.10 )
cmake_policy( SET CMP0004 OLD )

#setting module name, must use the same value if parent module exists
set ( MODULE_NAME carlife_c )

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
	libcarlifevehicle	
	glib-2.0
	protobuf
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
set( CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} ${${MODULE_NAME}_EXTRA_CFLAGS} -fPIC -Wreturn-type")
#checking sub modules existence, each module must be separated by a semicolon
#adding executable source cod

#adding local include directories
include_directories (
	./include
	../CarLifeVehicleLib-8th-December-2016/CarLifeLibSourceCode/trunk/protobuf/cc/include
)

#adding library source code
add_library(
	${MODULE_NAME}
	SHARED
	${CMAKE_CURRENT_SOURCE_DIR}/src/CCarLifeLib_c.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/CTransPackageProcess
)


#setting local target version
#set_target_properties(
#	${MODULE_NAME}
#	PROPERTIES VERSION ${VERSION}
#	SOVERSION ${SOVERSION}
#)
target_link_libraries(
	${MODULE_NAME}
	${${MODULE_NAME}_EXTRA_LDFLAGS}
	"-Wl,--whole-archive"
	${LOCAL_MODULES}
	${PREBUILT_MODULES}
	"-Wl,--no-whole-archive"
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
	DESTINATION		lib
)

#installing module pc file
install( 
	FILES
	${CMAKE_CURRENT_BINARY_DIR}/lib${MODULE_NAME}.pc       # from
	DESTINATION		lib/pkgconfig/                                  # to
)
install( 
	FILES
	${CMAKE_CURRENT_SOURCE_DIR}/include/CCarLifeLibWrapper_c.h
	DESTINATION		include                                  # to
)


