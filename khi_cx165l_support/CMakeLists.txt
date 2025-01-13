cmake_minimum_required(VERSION 3.12)
project(khi_cx165l_support)

find_package(ament_cmake REQUIRED)
ament_package()

install(DIRECTORY meshes urdf DESTINATION share/${PROJECT_NAME})
