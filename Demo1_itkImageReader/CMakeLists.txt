cmake_minimum_required(VERSION 2.8)

project(ITKImageReader)

find_package(ITK REQUIRED)

include(${ITK_USE_FILE})

add_executable(${PROJECT_NAME} ITKImageReader.cpp)

target_link_libraries(${PROJECT_NAME} ${ITK_LIBRARIES})
