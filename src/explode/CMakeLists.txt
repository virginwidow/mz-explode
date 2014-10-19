set (src 
  exceptions.cc
  io.cc
  exe_file.cc
  unpklite.cc
  unlzexe.cc
  unexepack.cc
  knowledge_dynamics.cc
)

set (hdr 
  exceptions.hh
  io.hh
  exe_file.hh
  unpklite.hh
  unlzexe.hh
  byte_order.hh
  struct_reader.hh
  unexepack.hh
  knowledge_dynamics.hh
)

if (EXPLODE_STATIC_BUILD)
  add_library (explode STATIC ${src} ${hdr})
else ()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILD_MZEXPLODE_AS_SHARED_LIB")
  set (CMAKE_CC_FLAGS  "${CMAKE_CC_FLAGS} -DBUILD_MZEXPLODE_AS_STATIC_LIB")
  add_library (explode SHARED ${src} ${hdr})
endif ()
