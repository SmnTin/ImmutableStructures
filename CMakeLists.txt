cmake_minimum_required(VERSION 3.6)
project(immu
        VERSION 0.0.1
        DESCRIPTION "Header-only C++ library that provides some easy-to-use immutable data structures."
        HOMEPAGE_URL "https://github.com/SmnTin/immu-cpp")

set(CMAKE_CXX_STANDARD 17)


#the library

add_library(immu INTERFACE)
target_include_directories(immu INTERFACE
        $<BUILD_INTERFACE:${immu_BINARY_DIR}/>
        $<BUILD_INTERFACE:${immu_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
install(TARGETS immu EXPORT ImmuConfig)
install(EXPORT ImmuConfig
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Immu")
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/immu
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

#a development version of the library
add_library(immu-dev INTERFACE)
target_link_libraries(immu-dev INTERFACE
        immu)

#tests
add_executable(immu_my_test main.cpp)
target_link_libraries(immu_my_test immu-dev)