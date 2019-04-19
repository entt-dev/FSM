file(GLOB_RECURSE SRCS *.cpp *.c)
# file(GLOB_RECURSE SRCS *.c)

SET(CMAKE_CXX_COMPILER /usr/bin/g++-8)
SET(CMAKE_C_COMPILER /usr/bin/g++-8)

add_library(libfsm SHARED ${SRCS})

target_compile_features(libfsm
        PRIVATE
        cxx_std_17
)

# add_subdirectory(thirdparty)

target_include_directories(libfsm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)


if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary

  # do Wno-class-memaccess to disable the memaccess warnings thrown for cereal
  # thirdparty headers...
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused -Wno-class-memaccess")
endif()
