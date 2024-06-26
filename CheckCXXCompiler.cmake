# Compiler check.
# Must support c++14
# If python2 is supported then we can not use c++17. 
if(COMPILER_IS_TESTED)
    return()
endif()

########################### COMPILER MACROS #####################################
include(CheckCXXCompilerFlag)
if(WIN32)
        CHECK_CXX_COMPILER_FLAG("/std:c++14" COMPILER_SUPPORTS_CXX14 )
else(WIN32)
        CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14 )
        CHECK_CXX_COMPILER_FLAG("-Wno-strict-aliasing" COMPILER_WARNS_STRICT_ALIASING )
endif(WIN32)

# Turn warning to error: Not all of the options may be supported on all
# versions of compilers. be careful here.
add_definitions(-Wall
    #-Wno-return-type-c-linkage
    -Wno-unused-variable
    -Wno-unused-function
    #-Wno-unused-private-field
    )

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.1")
        message(FATAL_ERROR "Insufficient gcc version. Minimum requried 5.1")
    endif()
    add_definitions( -Wno-unused-local-typedefs )
    add_definitions( -fmax-errors=5 )
elseif(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
    add_definitions( -Wno-unused-local-typedef )
endif()

add_definitions(-fPIC)
if(COMPILER_WARNS_STRICT_ALIASING)
    add_definitions( -Wno-strict-aliasing )
endif(COMPILER_WARNS_STRICT_ALIASING)

# Disable some harmless warnings.
CHECK_CXX_COMPILER_FLAG( "-Wno-unused-but-set-variable"
    COMPILER_SUPPORT_UNUSED_BUT_SET_VARIABLE_NO_WARN
    )
if(COMPILER_SUPPORT_UNUSED_BUT_SET_VARIABLE_NO_WARN)
    add_definitions( "-Wno-unused-but-set-variable" )
endif(COMPILER_SUPPORT_UNUSED_BUT_SET_VARIABLE_NO_WARN)

if(COMPILER_SUPPORTS_CXX14)
    if(WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++14")
    else(WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    endif(WIN32)
    if(APPLE)
        add_definitions( -mllvm -inline-threshold=1000 )
    endif(APPLE)
else(COMPILER_SUPPORTS_CXX14)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} is too old. \n"
      "Please use a compiler which has full c++14 support."
      )
endif(COMPILER_SUPPORTS_CXX14)

set(COMPILER_IS_TESTED ON)
