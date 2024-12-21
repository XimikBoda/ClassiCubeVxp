cmake_minimum_required (VERSION 3.8)

project ("ClassiCubeVxp")

set(APP_NAME "ClassiCube")
set(DEVELOPER_NAME "Ximik_Boda")
set(RAM "3000" CACHE STRING "Ram requirement (in kb)")
set(IMSI "1234567890" CACHE STRING "Imsi of first SIM-card")
set(API "File SIM card ProMng" CACHE STRING "Api for app")
option(BACKGROUND "App can work background" OFF)

set(MRE_SDK "$ENV{MRE_SDK}" CACHE STRING "MRE SDK dir")
set(TinyMRESDK "$ENV{TinyMRESDK}" CACHE STRING "TinyMRESDK dir")

add_definitions(-DMRE)

if(BACKGROUND)
    add_definitions(-DSUPPORT_BG)
endif()

add_subdirectory(resourses)
add_subdirectory(core)
add_subdirectory(mreapi)
add_subdirectory(vxp)
add_subdirectory(run)
add_subdirectory(src)

add_subdirectory(main)