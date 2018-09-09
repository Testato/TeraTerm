﻿
include(${CMAKE_CURRENT_LIST_DIR}/script_support.cmake)

if(MINGW)
  set(OPENSSL_INCLUDE_DIRS
	${CMAKE_CURRENT_LIST_DIR}/openssl_${TOOLSET}/include
	)

  set(OPENSSL_LIB
	${CMAKE_CURRENT_LIST_DIR}/openssl_${TOOLSET}/lib/libcrypto.a
  )

else()

  set(OPENSSL_ROOT ${CMAKE_CURRENT_LIST_DIR}/openssl_${TOOLSET})
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(OPENSSL_ROOT "${OPENSSL_ROOT}_x64")
  endif()

  set(OPENSSL_INCLUDE_DIRS
	"$<$<CONFIG:Debug>:${OPENSSL_ROOT}_debug/include>"
	"$<$<CONFIG:Release>:${OPENSSL_ROOT}/include>"
	)

  set(OPENSSL_LIB
	debug ${OPENSSL_ROOT}_debug/lib/libeay32.lib
	optimized ${OPENSSL_ROOT}/lib/libeay32.lib
	)

endif()
