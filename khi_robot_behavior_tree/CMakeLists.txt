cmake_minimum_required(VERSION 3.5)
project(khi_robot_behavior_tree)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(behaviortree_ros2 REQUIRED)
find_package(khi_robot_interfaces REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(trajectory_msgs REQUIRED)

add_library(${PROJECT_NAME} SHARED src/khi_robot_bt_nodes.cpp)
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
         "$<INSTALL_INTERFACE:include>")
ament_target_dependencies(${PROJECT_NAME} khi_robot_interfaces behaviortree_ros2
                          sensor_msgs trajectory_msgs)
target_compile_definitions(${PROJECT_NAME} PRIVATE BT_PLUGIN_EXPORT)

# Install the headers
install(DIRECTORY include/${PROJECT_NAME} DESTINATION include/)

# Install the library(ies)
install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}-targets
  DESTINATION lib)
ament_export_targets(${PROJECT_NAME}-targets HAS_LIBRARY_TARGET)

# Install kaw simulator
install(PROGRAMS scripts/kaw_simulator.py DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY config DESTINATION share/${PROJECT_NAME}/)

ament_export_dependencies(behaviortree_ros2 khi_robot_interfaces sensor_msgs
                          trajectory_msgs)
ament_package()