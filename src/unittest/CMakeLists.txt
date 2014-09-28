set (src
	main.cc       
	md5.c
)

set (hdr 
	md5.h
)

add_executable (unittest ${src} ${hdr})
target_link_libraries (unittest explode)