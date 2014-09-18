set (src 
  exceptions.cc
  io.cc
  exe_file.cc
  unpklite.cc
  unlzexe.cc
)

set (hdr 
  exceptions.hh
  io.hh
  exe_file.hh
  unpklite.hh
  unlzexe.hh
)

add_library (explode STATIC ${src} ${hdr})