
# Enable ExternalProject CMake module
include(ExternalProject)

# Download and install GoogleTest
ExternalProject_Add(
    gtest
    URL https://github.com/google/googletest/archive/master.zip
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
    # Disable install step
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(gtest source_dir binary_dir)

# Create a libgtest target to be used as a dependency by test programs
add_library(libgtest IMPORTED STATIC GLOBAL)

# Set libgtest properties
set_target_properties(libgtest PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/lib/libgtest.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
)

# I couldn't make it work with INTERFACE_INCLUDE_DIRECTORIES
include_directories("${source_dir}/googletest/include"
                    "${source_dir}/googlemock/include")

include(GoogleTest)

file(GLOB_RECURSE SRCS *.cpp)
# file(GLOB SRCS *.cpp physics/*.cpp audio/*.cpp obstacle/*.cpp navigation/*.cpp)

ADD_EXECUTABLE(unit_tests ${SRCS})

gtest_discover_tests(unit_tests)

target_compile_features(unit_tests
        PRIVATE
        cxx_std_17
)

target_link_libraries(unit_tests libfsm libgtest)

add_custom_target(check ${CMAKE_COMMAND} -E env CTEST_OUTPUT_ON_FAILURE=1
                  ${CMAKE_CTEST_COMMAND} -C $<CONFIG> --verbose
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
