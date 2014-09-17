cmake_minimum_required (VERSION 2.6)
project                (unpklite)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (UNPK_ROOT       ${CMAKE_SOURCE_DIR})
set (UNPK_SRC        ${CMAKE_SOURCE_DIR}/src)

include_directories (${UNPK_SRC})
add_definitions ("-g")
set (modules
  explode
  unpklite
)

foreach (module ${modules})
  add_subdirectory(${UNPK_SRC}/${module} obj/${module})
endforeach ()


