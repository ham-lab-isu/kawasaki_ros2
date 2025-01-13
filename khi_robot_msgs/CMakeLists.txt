cmake_minimum_required(VERSION 3.8)
project(khi_robot_msgs)

###############################################################################
# Dependencies
###############################################################################
find_package(ament_cmake REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(trajectory_msgs REQUIRED)

###############################################################################
# Service definitions
###############################################################################
set(srv_files
  srv/KhiRobotCmd.srv
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
  ${srv_files}
  DEPENDENCIES builtin_interfaces trajectory_msgs
  ADD_LINTER_TESTS
)

###############################################################################
# Export and package
###############################################################################
# So downstream packages can find the generated interfaces
ament_export_dependencies(rosidl_default_runtime)

ament_package()
