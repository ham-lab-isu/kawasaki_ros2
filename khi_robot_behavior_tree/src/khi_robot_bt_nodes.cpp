#pragma once

#include <rclcpp/rclcpp.hpp>
#include <khi_robot_msgs/srv/khi_robot_cmd.hpp>
#include <string>
#include <khi_robot_behavior_tree/khi_robot_bt_nodes.h>
#include <chrono>

namespace khi_robot_behavior_tree
{

// Example helper function (optional).
// Replaces the inline 'getBTInput' that throws on failure.
template <typename T>
T requireInput(BT::TreeNode* node, const std::string& port_name)
{
  BT::Expected<T> val = node->getInput<T>(port_name);
  if (!val)
  {
    throw BT::RuntimeError("Failed to get required input [" + port_name + "]: " + val.error());
  }
  return val.value();
}

class KhiRobotCmdNode : public BT::StatefulActionNode
{
public:
  // Required constructor signature if you use static providedPorts().
  KhiRobotCmdNode(const std::string& name, const BT::NodeConfiguration& config)
    : BT::StatefulActionNode(name, config)
  {
    // Retrieve the ROS node from the blackboard (provided by RosNodeParams).
    if (!config.blackboard->get("node", node_))
    {
      throw BT::RuntimeError("KhiRobotCmdNode: missing ROS node in blackboard");
    }
  }

  // List all input/output ports for this node
  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("type", "Command type for the robot"),
      BT::InputPort<std::string>("cmd", "Command string for the robot"),
      BT::OutputPort<int32_t>("driver_ret", "Driver return code"),
      BT::OutputPort<int32_t>("as_ret", "Action server return code"),
      BT::OutputPort<std::string>("cmd_ret", "Command return message")
    };
  }

  //-----------------------------
  //  Overridden from BT::StatefulActionNode
  //-----------------------------

  // Called once when the Action is activated.
  BT::NodeStatus onStart() override
  {
    // 1. Get input ports
    try
    {
      cmd_type_ = requireInput<std::string>(this, "type");
      cmd_      = requireInput<std::string>(this, "cmd");
    }
    catch (const std::exception& e)
    {
      RCLCPP_ERROR(node_->get_logger(), "[KhiRobotCmdNode::onStart] %s", e.what());
      return BT::NodeStatus::FAILURE;
    }

    // 2. Create the service client (only once)
    if (!client_)
    {
      client_ = node_->create_client<khi_robot_msgs::srv::KhiRobotCmd>("khi_robot_cmd");
      if (!client_->wait_for_service(std::chrono::seconds(2)))
      {
        RCLCPP_ERROR(node_->get_logger(),
                     "[KhiRobotCmdNode] Service 'khi_robot_cmd' not available.");
        return BT::NodeStatus::FAILURE;
      }
    }

    // 3. Create the request and populate fields
    request_ = std::make_shared<khi_robot_msgs::srv::KhiRobotCmd::Request>();
    request_->type = cmd_type_;
    request_->cmd  = cmd_;

    // 4. Asynchronously send the request
    future_ = client_->async_send_request(request_);

    // 5. Transition to RUNNING and wait in onRunning()
    return BT::NodeStatus::RUNNING;
  }

  // Called repeatedly while state is RUNNING.
  BT::NodeStatus onRunning() override
  {
    // Check if future is valid
    if (!future_.valid())
    {
      RCLCPP_ERROR(node_->get_logger(), "[KhiRobotCmdNode::onRunning] Invalid future.");
      return BT::NodeStatus::FAILURE;
    }

    // See if the service call has completed
    auto status = future_.wait_for(std::chrono::milliseconds(5));
    if (status == std::future_status::ready)
    {
      auto response = future_.get();

      // Set outputs
      setOutput("driver_ret", response->driver_ret);
      setOutput("as_ret",    response->as_ret);
      setOutput("cmd_ret",   response->cmd_ret);

      // Decide success/failure based on your robot's logic
      if (response->driver_ret == 0 && response->as_ret == 0)
      {
        return BT::NodeStatus::SUCCESS;
      }
      else
      {
        // Store error for debugging
        config().blackboard->set("ERROR_MESSAGE", response->cmd_ret);
        return BT::NodeStatus::FAILURE;
      }
    }

    // If not ready, keep running
    return BT::NodeStatus::RUNNING;
  }

  // Called if the Action is halted (e.g., by a parent node).
  void onHalted() override
  {
    RCLCPP_INFO(node_->get_logger(), "[KhiRobotCmdNode] Halted.");
  }

private:
  // Pointer to the ROS node from blackboard
  rclcpp::Node::SharedPtr node_;

  // Client and future for sending service requests
  rclcpp::Client<khi_robot_msgs::srv::KhiRobotCmd>::SharedPtr client_;
  rclcpp::Client<khi_robot_msgs::srv::KhiRobotCmd>::SharedFuture future_;

  // Service request
  khi_robot_msgs::srv::KhiRobotCmd::Request::SharedPtr request_;

  // Internal data
  std::string cmd_type_;
  std::string cmd_;
};

}  // namespace khi_robot_behavior_tree
