#pragma once

#include <behaviortree_ros2/bt_service_node.hpp>
#include <behaviortree_ros2/bt_topic_sub_node.hpp>
#include <khi_robot_msgs/srv/khi_robot_cmd.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <trajectory_msgs/msg/joint_trajectory.hpp>
#include <string>
#include <sstream>

namespace khi_robot_behavior_tree
{

inline static const std::string ERROR_MESSAGE_KEY = "error_message";

/**
 * @brief Custom RosServiceNode that overrides onFailure to store an error message.
 */
template <typename T>
class RosServiceNode : public BT::RosServiceNode<T>
{
public:
  // IMPORTANT: The underlying BT::RosServiceNode expects a 3-argument constructor:
  // RosServiceNode(const std::string &instance_name,
  //                const BT::NodeConfig &config,
  //                const BT::RosNodeParams &params)
  using BT::RosServiceNode<T>::RosServiceNode;

  /**
   * @brief Called when a service call fails for any reason (unreachable, timeout, etc).
   */
  inline BT::NodeStatus onFailure(BT::ServiceNodeErrorCode error) override
  {
    std::stringstream ss;
    ss << "Service '" << BT::RosServiceNode<T>::prev_service_name_ << "'";

    switch (error)
    {
      case BT::SERVICE_UNREACHABLE:
        ss << " is unreachable";
        break;
      case BT::SERVICE_TIMEOUT:
        ss << " timed out";
        break;
      case BT::INVALID_REQUEST:
        ss << " was sent an invalid request";
        break;
      case BT::SERVICE_ABORTED:
        ss << " was aborted";
        break;
      default:
        break;
    }

    // Store the error message in blackboard
    this->config().blackboard->set(ERROR_MESSAGE_KEY, ss.str());
    return BT::NodeStatus::FAILURE;
  }
};

/**
 * @brief A Behavior Tree node that sends a service request to "khi_robot_cmd".
 */
class KhiRobotCmd : public RosServiceNode<khi_robot_msgs::srv::KhiRobotCmd>
{
public:
  // Constructor with the (name, config, params) signature
  KhiRobotCmd(const std::string& name,
              const BT::NodeConfiguration& config,
              const BT::RosNodeParams& params)
    : RosServiceNode<khi_robot_msgs::srv::KhiRobotCmd>(name, config, params)
  {
  }

  // Keys for input & output ports
  inline static const std::string TYPE_KEY       = "type";
  inline static const std::string CMD_KEY        = "cmd";
  inline static const std::string DRIVER_RET_KEY = "driver_ret";
  inline static const std::string AS_RET_KEY     = "as_ret";
  inline static const std::string CMD_RET_KEY    = "cmd_ret";

  /**
   * @brief List all the ports available (input / output).
   */
  static BT::PortsList providedPorts()
  {
    return providedBasicPorts({
      BT::InputPort<std::string>(TYPE_KEY, "Command type for the robot"),
      BT::InputPort<std::string>(CMD_KEY,  "Command string for the robot"),
      BT::OutputPort<int32_t>(DRIVER_RET_KEY, "Driver return code"),
      BT::OutputPort<int32_t>(AS_RET_KEY,     "Action server return code"),
      BT::OutputPort<std::string>(CMD_RET_KEY,"Command return message")
    });
  }

  /**
   * @brief Populate the request from the Behavior Tree input ports.
   */
  bool setRequest(Request::SharedPtr& request) override
  {
    request->type = this->template getInput<std::string>(TYPE_KEY).value_or("");
    request->cmd  = this->template getInput<std::string>(CMD_KEY).value_or("");
    return true;
  }

  /**
   * @brief Process the response and set the output ports.
   */
  BT::NodeStatus onResponseReceived(const Response::SharedPtr& response) override
  {
    this->setOutput(DRIVER_RET_KEY, response->driver_ret);
    this->setOutput(AS_RET_KEY,     response->as_ret);
    this->setOutput(CMD_RET_KEY,    response->cmd_ret);

    if (response->driver_ret == 0 && response->as_ret == 0)
    {
      return BT::NodeStatus::SUCCESS;
    }
    else
    {
      // Store the error in the blackboard
      this->config().blackboard->set(ERROR_MESSAGE_KEY, response->cmd_ret);
      return BT::NodeStatus::FAILURE;
    }
  }
};

/**
 * @brief Subscribes to 'sensor_msgs::JointState' and modifies a trajectory
 *        based on the last received state, then outputs the result.
 */
class AddJointsToTrajectoryNode : public BT::RosTopicSubNode<sensor_msgs::msg::JointState>
{
public:
  // Constructor with (name, config, params) to match RosTopicSubNode
  AddJointsToTrajectoryNode(const std::string& name,
                            const BT::NodeConfiguration& config,
                            const BT::RosNodeParams& params)
    : BT::RosTopicSubNode<sensor_msgs::msg::JointState>(name, config, params)
  {
  }

  // Port keys
  inline static const std::string TRAJECTORY_INPUT_PORT_KEY  = "input";
  inline static const std::string TRAJECTORY_OUTPUT_PORT_KEY = "output";
  inline static const std::string CONTROLLER_JOINT_NAMES_PARAM = "controller_joint_names";

  /**
   * @brief Declare input and output ports for the NodeConfiguration.
   */
  static BT::PortsList providedPorts()
  {
    return providedBasicPorts({
      BT::InputPort<trajectory_msgs::msg::JointTrajectory>(TRAJECTORY_INPUT_PORT_KEY),
      BT::OutputPort<trajectory_msgs::msg::JointTrajectory>(TRAJECTORY_OUTPUT_PORT_KEY)
    });
  }

  /**
   * @brief Called on each tick; 'last_msg' is the last subscribed JointState.
   */
  BT::NodeStatus onTick(const sensor_msgs::msg::JointState::SharedPtr& last_msg) override;
};

}  // namespace khi_robot_behavior_tree
