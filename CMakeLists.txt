cmake_minimum_required (VERSION 2.6)
project                (explode)

if (DEFINED CMAKE_BUILD_TYPE)
  message ("Appling CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} from command line")
  set (CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of
build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug
Release RelWithDebInfo MinSizeRel.")
else ()
   set (CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build,
options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release
RelWithDebInfo MinSizeRel.")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set (EXPLODE_DEBUG_BUILD 1)
endif ()

include(CheckCXXSourceCompiles)

# Toolset detection.
if (NOT EXPLODE_TOOLSET)
  set(EXPLODE_TOOLSET "unknown")
  if (MSVC60)
    set(EXPLODE_TOOLSET "vc6")
    set(EXPLODE_COMPILER "msvc")
    set(EXPLODE_COMPILER_VERSION "6.0")
  elseif(MSVC70)
    set(EXPLODE_TOOLSET "vc7")
    set(EXPLODE_COMPILER "msvc")
    set(EXPLODE_COMPILER_VERSION "7.0")
  elseif(MSVC71)
    set(EXPLODE_TOOLSET "vc71")
    set(EXPLODE_COMPILER "msvc")
    set(EXPLODE_COMPILER_VERSION "7.1")
  elseif(MSVC80)
    set(EXPLODE_TOOLSET "vc80")
    set(EXPLODE_COMPILER "msvc")
    set(EXPLODE_COMPILER_VERSION "8.0")
  elseif(MSVC90)
    set(EXPLODE_TOOLSET "vc90")
    set(EXPLODE_COMPILER "msvc")
    set(EXPLODE_COMPILER_VERSION "9.0")
  elseif(MSVC10)
    set(EXPLODE_TOOLSET "vc10")
    set(EXPLODE_COMPILER "msvc")
    set(EXPLODE_COMPILER_VERSION "10.0")
  elseif(MSVC11)
    set(EXPLODE_TOOLSET "vc11")
    set(EXPLODE_COMPILER "msvc")
    set(EXPLODE_COMPILER_VERSION "11.0")
  elseif(MSVC12)
    set(EXPLODE_TOOLSET "vc12")
    set(EXPLODE_COMPILER "msvc")
    set(EXPLODE_COMPILER_VERSION "12.0")	
  elseif(MSVC)
    set(EXPLODE_TOOLSET "vc")
    set(EXPLODE_COMPILER "msvc")
    set(EXPLODE_COMPILER_VERSION "unknown")
  elseif(BORLAND)
    set(EXPLODE_TOOLSET "bcb")
    set(EXPLODE_COMPILER "msvc")
    set(EXPLODE_COMPILER_VERSION "unknown")
  elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(EXPLODE_COMPILER "gcc")

    # Execute GCC with the -dumpversion option, to give us a version string
    execute_process(
      COMMAND ${CMAKE_CXX_COMPILER} "-dumpversion" 
      OUTPUT_VARIABLE GCC_VERSION_STRING)
    
    # Match only the major and minor versions of the version string
    string(REGEX MATCH "[0-9]+.[0-9]+" GCC_MAJOR_MINOR_VERSION_STRING
      "${GCC_VERSION_STRING}")

    # Match the full compiler version for the build name
    string(REGEX MATCH "[0-9]+.[0-9]+.[0-9]+" EXPLODE_COMPILER_VERSION
      "${GCC_VERSION_STRING}")
    
    # Strip out the period between the major and minor versions
    string(REGEX REPLACE "\\." "" EXPLODE_VERSIONING_GCC_VERSION
      "${GCC_MAJOR_MINOR_VERSION_STRING}")
    
    # Set the GCC versioning toolset
    set(EXPLODE_TOOLSET "gcc${EXPLODE_VERSIONING_GCC_VERSION}")
  elseif(CMAKE_CXX_COMPILER MATCHES "/icpc$" OR CMAKE_CXX_COMPILER MATCHES "/icpc.exe$")
    set(EXPLODE_TOOLSET "intel")
    set(EXPLODE_COMPILER "intel")
    set(CMAKE_COMPILER_IS_INTEL ON)
    execute_process(
      COMMAND ${CMAKE_CXX_COMPILER} "-dumpversion"
      OUTPUT_VARIABLE INTEL_VERSION_STRING
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(EXPLODE_COMPILER_VERSION ${INTEL_VERSION_STRING})
  elseif (CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(EXPLODE_TOOLSET "clang")
    set(EXPLODE_COMPILER "clang")
    set(CMAKE_COMPILER_IS_CLANG ON)
    set(EXPLODE_COMPILER_VERSION ${CMAKE_C_COMPILER_VERSION})
  elseif (CMAKE_C_COMPILER_ID MATCHES "Sun")
    set(EXPLODE_TOOLSET "SunCC")
    set(EXPLODE_COMPILER "SunCC")
    set(CMAKE_COMPILER_IS_SUNCC ON)
    execute_process(
      COMMAND ${CMAKE_CXX_COMPILER} "-V"
      ERROR_FILE "${PROJECT_SOURCE_DIR}/ccver.tmp"
      ERROR_STRIP_TRAILING_WHITESPACE)
    FILE (STRINGS "${PROJECT_SOURCE_DIR}/ccver.tmp" SUNCC_VERSION_RAW LIMIT_COUNT 1)
    STRING (REGEX MATCH "[0-9]+(\\.[0-9]+)*" SUNCC_VERSION_STRING ${SUNCC_VERSION_RAW})
    FILE (REMOVE "${PROJECT_SOURCE_DIR}/ccver.tmp")
    set(EXPLODE_COMPILER_VERSION ${SUNCC_VERSION_STRING})
  endif(MSVC60)
endif (NOT EXPLODE_TOOLSET)


message(STATUS "Explode compiler: ${EXPLODE_COMPILER}")
message(STATUS "Explode toolset:  ${EXPLODE_TOOLSET}")

# create cache entry
set(EXPLODE_PLATFORM "unknown")

# check for 64 bit build

if( (MSVC AND CMAKE_CL_64) OR CMAKE_GENERATOR MATCHES Win64)
  set(BUILD_64BIT 1)
elseif(CMAKE_SIZEOF_VOID_P MATCHES 8)
  set(BUILD_64BIT 1)
endif()

if (BUILD_64BIT)
  message (STATUS "64 bit build enabled")
else ()
	message (STATUS "32 bit build enabled")
endif ()

# Reflect 64-bits options
if (BUILD_64BIT)
  set (GCC_64_FLAG -m64)
  set (ICC_64_FLAG -m64)
  set (SUNCC_64_FLAG -m64)
  set (MSVC_64_FLAG " /DWIN32 /D_WIN32 ")
else ()
    set (GCC_64_FLAG -m32)
  set (ICC_64_FLAG -m32)
  set (SUNCC_64_FLAG -m32)
    set (MSVC_64_FLAG " /DWIN32 /D_WIN32 ")
endif ()

# Multi-threading support
if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
  set(MULTI_THREADED_COMPILE_FLAGS "-pthreads")
  set(MULTI_THREADED_LINK_LIBS rt)
  set(EXPLODE_PLATFORM "sunos")
elseif(CMAKE_SYSTEM_NAME STREQUAL "BeOS")
  # No threading options necessary for BeOS
  set(EXPLODE_PLATFORM "beos")
elseif(CMAKE_SYSTEM_NAME MATCHES ".*BSD")
  set(MULTI_THREADED_COMPILE_FLAGS "-pthread")
  set(MULTI_THREADED_LINK_LIBS pthread)
  set(EXPLODE_PLATFORM "bsd")
elseif(CMAKE_SYSTEM_NAME STREQUAL "DragonFly")
  # DragonFly is a FreeBSD bariant
  set(MULTI_THREADED_COMPILE_FLAGS "-pthread")
  set(EXPLODE_PLATFORM "dragonfly")
elseif(CMAKE_SYSTEM_NAME STREQUAL "IRIX")
  # TODO: GCC on Irix doesn't support multi-threading?
  set(EXPLODE_PLATFORM "irix")
elseif(CMAKE_SYSTEM_NAME STREQUAL "HP-UX")
  # TODO: gcc on HP-UX does not support multi-threading?
  set(EXPLODE_PLATFORM "hpux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # No threading options necessary for Mac OS X
  set(EXPLODE_PLATFORM "macos")
elseif(UNIX)
  # Assume -pthread and -lrt on all other variants
  if (EXPLODE_TOOLSET STREQUAL "SunCC")
    set(MULTI_THREADED_COMPILE_FLAGS "-D_REENTRANT")
    set(MULTI_THREADED_LINK_FLAGS " ${SUNCC_64_FLAG} ")  
    set(MULTI_THREADED_LINK_LIBS pthread rt)
  else ()  
    set(MULTI_THREADED_COMPILE_FLAGS "-pthread -D_REENTRANT")
    set(MULTI_THREADED_LINK_FLAGS "")  
    set(MULTI_THREADED_LINK_LIBS pthread rt)
  endif ()

  if (MINGW)
    set(EXPLODE_PLATFORM "mingw")
  elseif(CYGWIN)
    set(EXPLODE_PLATFORM "cygwin")
  elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(EXPLODE_PLATFORM "linux")
  else()
    set(EXPLODE_PLATFORM "unix")
  endif()
elseif(WIN32)
  set(EXPLODE_PLATFORM "windows")
else()
  set(EXPLODE_PLATFORM "unknown")
endif()

# create cache entry
set(EXPLODE_PLATFORM ${EXPLODE_PLATFORM} CACHE STRING "Explode platform name")

# Set the build name 
set(EXPLODE_BUILDNAME "${EXPLODE_COMPILER}-${EXPLODE_COMPILER_VERSION}-${EXPLODE_PLATFORM}")
message(STATUS "Explode BUILDNAME: ${EXPLODE_BUILDNAME}")

# =============================================================================
# Compiler flags
# =============================================================================

include (TestBigEndian)

test_big_endian (EXPLODE_HAS_BIGENDIAN)

if (${EXPLODE_HAS_BIGENDIAN})
  message (STATUS "BigEndian platform detected")
else ()
  message (STATUS "LittleEndian platform detected")
endif ()


# --------------------------------------------------
# GCC flags
# --------------------------------------------------

if (UNIX OR MINGW)
  set (LFS_FLAGS "-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE")
else ()
endif ()

if (${EXPLODE_HAS_BIGENDIAN})
  set (EXPLODE_ENDIANITY_FLAG "-DEXPLODE_BIG_ENDIAN")
endif ()

if (BUILD_64BIT) 
   set (EXPLODE_BITNESS_FLAG "-DEXPLODE_BUILD_64BIT")				
endif ()

set (EXPLODE_DEFINITIONS "${EXPLODE_BITNESS_FLAG} ${EXPLODE_ENDIANITY_FLAG}")

# --------------------------------------------------
# GCC flags
# --------------------------------------------------

# warning
set (GCC_COMMON_WARNINGS "-Wall -Wunknown-pragmas -Wformat-security -Wpacked  -Wparentheses  -Wpointer-arith -Wwrite-strings -Wundef -Wextra -Wconversion -Wno-long-long")

set (GCC_WARNINGS "${GCC_COMMON_WARNINGS} -Wmissing-prototypes")
set (GPP_WARNINGS "${GCC_COMMON_WARNINGS} -Wnon-virtual-dtor") 

set (GCC_NO_WARNINGS "-w")
set (GPP_NO_WARNINGS "${GCC_NO_WARNINGS}") 

#compiler flags
set (GCC_COMMON_FLAGS "${GCC_64_FLAG} -pipe -fstrict-aliasing -pedantic -fvisibility=hidden -ffunction-sections -fdata-sections ${LFS_FLAGS} ${EXPLODE_ENDIANITY_FLAG}")
set (GCC_FLAGS "${GCC_COMMON_FLAGS}")
set (GPP_FLAGS "${GCC_COMMON_FLAGS} -fvisibility-inlines-hidden")

set (GCC_FLAGS_DEBUG "${GCC_FLAGS} -g")
set (GCC_FLAGS_RELEASE "${GCC_FLAGS} -O2")
set (GPP_FLAGS_DEBUG "${GPP_FLAGS} -g")
set (GPP_FLAGS_RELEASE "${GPP_FLAGS} -O2")

set (GCC_DEFINES_DEBUG "-D_DEBUG=1 -DEXPLODE_DEBUG")
set (GCC_DEFINES_RELEASE "-DNDEBUG")

# --------------------------------------------------
# INTEL flags
# --------------------------------------------------

# warning
set (ICC_COMMON_WARNINGS "-Wall -Wpointer-arith -Wwrite-strings -Wcheck -Wp64  -Wconversion")

set (ICC_WARNINGS "${ICC_COMMON_WARNINGS} -Wmissing-prototypes")
set (ICC_NO_WARNINGS "-w")

set (IPP_WARNINGS "${ICC_COMMON_WARNINGS} -Wnon-virtual-dtor ") 
set (IPP_NO_WARNINGS "-w")

#compiler flags
set (ICC_COMMON_FLAGS "${ICC_64_FLAG} -pipe -fno-strict-aliasing -fvisibility=hidden ${LFS_FLAGS}")
set (ICC_FLAGS "${ICC_COMMON_FLAGS}")
set (IPP_FLAGS "${ICC_COMMON_FLAGS} -fvisibility-inlines-hidden")

set (ICC_FLAGS_DEBUG "${ICC_FLAGS} -g")
set (ICC_FLAGS_RELEASE "${ICC_FLAGS} -O2")
set (IPP_FLAGS_DEBUG "${IPP_FLAGS} -g")
set (IPP_FLAGS_RELEASE "${IPP_FLAGS} -O2")

set (ICC_DEFINES_DEBUG "-D_DEBUG=1 -DEXPLODE_DEBUG")
set (ICC_DEFINES_RELEASE "-DNDEBUG")

# --------------------------------------------------
# MSVC flags
# --------------------------------------------------

# warning
set (MSVC_CC_COMMON_WARNINGS "/W3")

set (MSVC_CC_WARNINGS "${MSVC_CC_COMMON_WARNINGS}")
set (MSVC_CXX_WARNINGS "${MSVC_CC_COMMON_WARNINGS} ") 

set (MSVC_CC_NO_WARNINGS "/W0")
set (MSVC_CXX_NO_WARNINGS "${MSVC_CC_NO_WARNINGS} ") 

#compiler flags
set (MSVC_CC_COMMON_FLAGS "${MSVC_CC_64_FLAG} /nologo /EHsc /GR /Zc:wchar_t")
set (MSVC_CC_FLAGS "${MSVC_CC_COMMON_FLAGS}")
set (MSVC_CXX_FLAGS "${MSVC_CC_COMMON_FLAGS}")

set (MSVC_CC_FLAGS_DEBUG "${MSVC_CC_FLAGS} /Zi  /GR /Ob0 /Od /MDd")
set (MSVC_CC_FLAGS_RELEASE "${MSVC_CC_FLAGS} /O2 /MD /Zi ")

set (MSVC_CXX_FLAGS_DEBUG "${MSVC_CXX_FLAGS} /Zi  /GR /Ob0 /Od /MDd")
set (MSVC_CXX_FLAGS_RELEASE "${MSVC_CXX_FLAGS} /O2 /MD /Zi ")

set (MSVC_COMMON_DEFINES 
  "/DNOMINMAX ${MSVC_64_FLAG} /D_WINDOWS /D_CONSOLE /D_CRT_SECURE_NO_WARNINGS /D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECAT")

set (MSVC_CC_DEFINES_DEBUG "${MSVC_COMMON_DEFINES} /D_DEBUG=1 /DEXPLODE_DEBUG")
set (MSVC_CC_DEFINES_RELEASE "${MSVC_COMMON_DEFINES} /DNDEBUG")

set (MSVC_CC_LINKER_FLAGS_DEBUG "/DEBUG")

# --------------------------------------------------
# SunCC flags
# --------------------------------------------------

set (SUN_CC_COMMON_WARNINGS "-errtags=yes")

set (SUN_CC_WARNINGS "${SUN_CC_COMMON_WARNINGS} ")
set (SUN_CXX_WARNINGS "${SUN_CC_COMMON_WARNINGS} +w2 ")

set (SUN_CC_COMMON_FLAGS "-features=extensions -mt ${SUNCC_64_FLAG} ${LFS_FLAGS}")

set (SUN_CXX_FLAGS_DEBUG "${SUN_CC_COMMON_FLAGS} -g ")
set (SUN_CXX_FLAGS_RELEASE "${SUN_CC_COMMON_FLAGS} -xtarget=native -fast -xarch=amd64  ")

set (SUN_CC_FLAGS_DEBUG "${SUN_CC_COMMON_FLAGS} -g ")
set (SUN_CC_FLAGS_RELEASE "${SUN_CC_COMMON_FLAGS} -xtarget=native -fast -xarch=amd64 ")

set (SUN_CC_DEFINES_DEBUG "-D_DEBUG=1 -DEXPLODE_DEBUG ")
set (SUN_CC_DEFINES_RELEASE "-DNDEBUG  ")

set (SUN_CC_LINKER_FLAGS_DEBUG "-g  -lpthread")
set (SUN_CC_LINKER_FLAGS_RELEASE "-g  -lpthread")

# --------------------------------------------------
# CLang flags
# --------------------------------------------------

set (CLANG_CC_COMMON_WARNINGS "-Weverything -pedantic -Wno-padded -Wno-exit-time-destructors")

set (CLANG_CC_WARNINGS "${CLANG_CC_COMMON_WARNINGS} ")
set (CLANG_CXX_WARNINGS "${CLANG_CC_COMMON_WARNINGS} ")

#set (CLANG_CC_COMMON_FLAGS "-features=extensions -mt ${SUNCC_64_FLAG} ${LFS_FLAGS}")

set (CLANG_CXX_FLAGS_DEBUG "  -g ")
set (CLANG_CXX_FLAGS_RELEASE "-O2  ")

set (CLANG_CC_FLAGS_DEBUG "  -g ")
set (CLANG_CC_FLAGS_RELEASE " -O2 ")

set (CLANG_CC_DEFINES_DEBUG " -D_DEBUG=1 -DEXPLODE_DEBUG ")
set (CLANG_CC_DEFINES_RELEASE " -DNDEBUG  ")

set (CLANG_CC_LINKER_FLAGS_DEBUG " -lpthread")
set (CLANG_CC_LINKER_FLAGS_RELEASE "  -lpthread")

# ======================================================================
macro (config_compile_and_linker_flags_name CXX_NAME CC_NAME)
  if (NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    set (cxx_flags "${${CXX_NAME}_WARNINGS} ${${CXX_NAME}_FLAGS_DEBUG} ${${CC_NAME}_DEFINES_DEBUG}")
    set (cc_flags  "${${CC_NAME}_WARNINGS} ${${CC_NAME}_FLAGS_DEBUG} ${${CC_NAME}_DEFINES_DEBUG}")
	
    set (cxx_no_warn_flags "${${CXX_NAME}_NO_WARNINGS} ${${CXX_NAME}_FLAGS_DEBUG} ${${CC_NAME}_DEFINES_DEBUG}")
    set (cc_no_warn_flags  "${${CC_NAME}_NO_WARNINGS} ${${CC_NAME}_FLAGS_DEBUG} ${${CC_NAME}_DEFINES_DEBUG}")

    set (linker_flags  "${${CC_NAME}_LINKER_FLAGS_DEBUG}")
  else ()
    set (cxx_flags "${${CXX_NAME}_WARNINGS} ${${CXX_NAME}_FLAGS_RELEASE} ${${CC_NAME}_DEFINES_RELEASE}")
    set (cc_flags  "${${CC_NAME}_WARNINGS} ${${CC_NAME}_FLAGS_RELEASE} ${${CC_NAME}_DEFINES_RELEASE}")

    set (cxx_no_warn_flags "${${CXX_NAME}_NO_WARNINGS} ${${CXX_NAME}_FLAGS_RELEASE} ${${CC_NAME}_DEFINES_RELEASE}")
    set (cc_no_warn_flags  "${${CC_NAME}_NO_WARNINGS} ${${CC_NAME}_FLAGS_RELEASE} ${${CC_NAME}_DEFINES_RELEASE}")

    set (linker_flags  "${${CC_NAME}_LINKER_FLAGS_RELEASE}")
  endif ()

  set (cc_flags "${cc_flags} ${MULTI_THREADED_COMPILE_FLAGS} ${EXPLODE_COMMON_CC_FLAGS} ${EXPLODE_DEFINITIONS}")
  set (cxx_flags "${cxx_flags} ${MULTI_THREADED_COMPILE_FLAGS} ${EXPLODE_COMMON_CXX_FLAGS} ${EXPLODE_DEFINITIONS}")
endmacro ()
# ======================================================================
if (EXPLODE_TOOLSET STREQUAL "intel")
  config_compile_and_linker_flags_name (IPP ICC)
elseif (EXPLODE_TOOLSET STREQUAL "gcc44" OR EXPLODE_TOOLSET STREQUAL "gcc45" OR EXPLODE_TOOLSET STREQUAL "gcc46" OR EXPLODE_TOOLSET STREQUAL "gcc47")
  config_compile_and_linker_flags_name (GPP GCC)
elseif (EXPLODE_TOOLSET STREQUAL "vc80" OR EXPLODE_TOOLSET STREQUAL "vc90" OR EXPLODE_TOOLSET STREQUAL "vc11" OR EXPLODE_TOOLSET STREQUAL "vc10" OR EXPLODE_TOOLSET STREQUAL "vc12")
  config_compile_and_linker_flags_name (MSVC_CXX MSVC_CC)
elseif (EXPLODE_TOOLSET STREQUAL "SunCC")
  config_compile_and_linker_flags_name (SUN_CXX SUN_CC)
elseif (EXPLODE_TOOLSET STREQUAL "clang")
  config_compile_and_linker_flags_name (CLANG_CXX CLANG_CC)
else ()
  message (FATAL_ERROR "Unknown compiler ${EXPLODE_TOOLSET}")
endif ()

set (cc_flags "${cc_flags} -DSOURCES_ROOT=${CMAKE_SOURCE_DIR}")
set (cxx_flags "${cxx_flags} -DSOURCES_ROOT=${CMAKE_SOURCE_DIR}")



message ("------------------------------------------")
message ("CC  flags: ${cc_flags}")
message ("C++ flags: ${cxx_flags}")
message ("------------------------------------------")




# ==================================================================================
set (EXPLODE_PRODUCT_DIR ${CMAKE_BINARY_DIR}/bin)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EXPLODE_PRODUCT_DIR})

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EXPLODE_PRODUCT_DIR})
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${EXPLODE_PRODUCT_DIR} )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )


set (EXPLODE_ROOT       ${CMAKE_SOURCE_DIR})
set (EXPLODE_SRC        ${CMAKE_SOURCE_DIR}/src)

include_directories (${EXPLODE_SRC})
set (CMAKE_CC_FLAGS "${cc_flags}")

set (CMAKE_CXX_FLAGS "${cxx_flags}")

if (linker_flags)
  set (CMAKE_EXE_LINKER_FLAGS    "${linker_flags}")
  set (CMAKE_SHARED_LINKER_FLAGS "${linker_flags}")
  set (CMAKE_MODULE_LINKER_FLAGS "${linker_flags}")
endif ()


set (modules
  explode
  unpklite
  unlzexe
  mzdump
  unittest
)

foreach (module ${modules})
  add_subdirectory(${EXPLODE_SRC}/${module} obj/${module})
endforeach ()


