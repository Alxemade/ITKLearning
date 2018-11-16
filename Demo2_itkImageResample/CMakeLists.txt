cmake_minimum_required(VERSION 2.8)

project(ITKImageResample)

find_package(ITK REQUIRED)

include(${ITK_USE_FILE})

add_executable(${PROJECT_NAME} ITKImageResample.cpp)

target_link_libraries(${PROJECT_NAME} ${ITK_LIBRARIES})
