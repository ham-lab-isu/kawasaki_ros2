cmake_minimum_required(VERSION 3.8)
project(khi_robot_interfaces)

###############################################################################
# Dependencies
###############################################################################
find_package(ament_cmake REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(trajectory_msgs REQUIRED)

###############################################################################
# Message definitions
###############################################################################
set(msg_files
  msg/AlarmInfo.msg
  msg/AlarmCauseRemedy.msg
  msg/ErrorInfo.msg
  msg/InformJobCrudResultCodes.msg
  msg/InitTrajEnum.msg
  msg/IoResultCodes.msg
  msg/MotionReadyEnum.msg
  msg/QueueResultEnum.msg
  msg/SelectionResultCodes.msg
)

###############################################################################
# Service definitions
###############################################################################
set(srv_files
  srv/KhiRobotCmd.srv
  srv/GetActiveAlarmInfo.srv
  srv/ListInformJobs.srv
  srv/QueueTrajPoint.srv
  srv/ReadMRegister.srv
  srv/ReadSingleIO.srv
  srv/ReadGroupIO.srv
  srv/ResetError.srv
  srv/SelectMotionTool.srv
  srv/StartTrajMode.srv
  srv/StartPointQueueMode.srv
  srv/WriteMRegister.srv
  srv/WriteSingleIO.srv
  srv/WriteGroupIO.srv
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
