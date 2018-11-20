cmake_minimum_required(VERSION 2.8)

project(ITKImageFFTW)

find_package(ITK REQUIRED)

include(${ITK_USE_FILE})

add_executable(${PROJECT_NAME} ITKImageFFTW.cpp)

target_link_libraries(${PROJECT_NAME} ${ITK_LIBRARIES})
