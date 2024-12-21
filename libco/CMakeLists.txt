cmake_minimum_required (VERSION 3.8)

set(PROJECT_NAME "libco")

add_library(${PROJECT_NAME} STATIC 
	libco.c libco.h
	thread.c thread.h
)

target_include_directories(${PROJECT_NAME} PUBLIC ".")

target_link_libraries(${PROJECT_NAME} mreapi)