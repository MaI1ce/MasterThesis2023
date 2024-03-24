cmake_minimum_required(VERSION 3.16)
project("DS2_Host")
set(CMAKE_CXX_STANDARD 17)


# -	add_subdirectory is used when you have a subdirectory that contains its own CMakeLists.txt, 
#	which might define its own targets and other build settings. This command adds that 
#	subdirectory to the build, and CMake will process the CMakeLists.txt in that subdirectory.

# -	target_include_directories is used to add include directories to a specific target. 
#	These directories typically contain header files that the target needs, 
#	but they do not define any targets of their own.

#So, in essence, add_subdirectory is about incorporating a sub-build with its own targets into 
#the main build, while target_include_directories is about specifying where a particular target 
#should look for its header files.

# https://logins.github.io/programming/2020/05/17/CMakeInVisualStudio.html

set(EXTERNALS "${PROJECT_SOURCE_DIR}/externals")
set(MODULE "${PROJECT_SOURCE_DIR}/source/module")
set(SUB_MODULE "${PROJECT_SOURCE_DIR}/source/ds2")

# pybind11
add_subdirectory(${EXTERNALS}/pybind11)

#target name should be the same as declared in PYBIND11_MODULE in main cpp source
pybind11_add_module(
    ds2 
    ${MODULE}/python_wrapper.cpp
    ${SUB_MODULE}/ds2_host_class.cpp
    ${SUB_MODULE}/src/commit.c
    ${SUB_MODULE}/src/poly.c
    ${SUB_MODULE}/src/fips202.c
    ${SUB_MODULE}/src/ntt.c
    ${SUB_MODULE}/src/util.c
    ${SUB_MODULE}/src/rand.c
    ${SUB_MODULE}/src/reduce.c
)

target_include_directories(
	ds2
	PRIVATE "${SUB_MODULE}/inc" #part of the interface
)

set_target_properties(ds2 PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
)