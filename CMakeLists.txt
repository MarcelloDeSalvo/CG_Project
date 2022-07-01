cmake_minimum_required(VERSION 3.21)
project(CG_Project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(headers)

# Including GLFW
add_subdirectory(glfw)
include_directories(glfw/include)

add_executable(CG_Project Assignment0.cpp)

set(ENV{VULKAN_SDK} "C:/VulkanSDK/1.3.216.0")
find_package(Vulkan REQUIRED)
target_link_libraries(CG_Project glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)
