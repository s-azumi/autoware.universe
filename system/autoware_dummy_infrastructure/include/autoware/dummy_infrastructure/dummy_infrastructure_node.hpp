// Copyright 2021 Tier IV
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef AUTOWARE__DUMMY_INFRASTRUCTURE__DUMMY_INFRASTRUCTURE_NODE_HPP_
#define AUTOWARE__DUMMY_INFRASTRUCTURE__DUMMY_INFRASTRUCTURE_NODE_HPP_

#include "autoware_utils/ros/polling_subscriber.hpp"

#include <rclcpp/rclcpp.hpp>

#include <autoware_internal_planning_msgs/msg/planning_factor_array.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tier4_v2x_msgs/msg/infrastructure_command_array.hpp>
#include <tier4_v2x_msgs/msg/virtual_traffic_light_state_array.hpp>

#include <chrono>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace autoware::dummy_infrastructure
{
using autoware_internal_planning_msgs::msg::PlanningFactorArray;
using nav_msgs::msg::Odometry;
using tier4_v2x_msgs::msg::InfrastructureCommand;
using tier4_v2x_msgs::msg::InfrastructureCommandArray;
using tier4_v2x_msgs::msg::VirtualTrafficLightState;
using tier4_v2x_msgs::msg::VirtualTrafficLightStateArray;

class DummyInfrastructureNode : public rclcpp::Node
{
public:
  explicit DummyInfrastructureNode(const rclcpp::NodeOptions & node_options);

  struct NodeParam
  {
    double update_rate_hz{};
    bool use_first_command{};
    bool use_command_state{};
    std::string instrument_id{};
    bool approval{};
    bool is_finalized{};
    bool auto_approval_mode{};
    double stop_distance_threshold{};
    double stop_velocity_threshold{};
  };

private:
  autoware_utils::InterProcessPollingSubscriber<InfrastructureCommandArray> sub_command_array_{
    this, "~/input/command_array"};
  autoware_utils::InterProcessPollingSubscriber<PlanningFactorArray> sub_planning_factors_{
    this, "~/input/planning_factors"};
  autoware_utils::InterProcessPollingSubscriber<Odometry> sub_odometry_{this, "~/input/odometry"};

  // Data Buffer
  InfrastructureCommandArray::ConstSharedPtr command_array_{};
  PlanningFactorArray::ConstSharedPtr planning_factors_{};
  Odometry::ConstSharedPtr current_odometry_{};
  std::set<std::string> approved_command_ids_{};

  // Publisher
  rclcpp::Publisher<VirtualTrafficLightStateArray>::SharedPtr pub_state_array_{};

  // Timer
  rclcpp::TimerBase::SharedPtr timer_{};

  bool isDataReady();
  void onTimer();

  // Auto approval check
  std::pair<bool, bool> getApprovalConditions() const;
  std::optional<std::string> getCurrentCommandId() const;
  std::pair<bool, bool> checkApprovalCommand(
    const std::string & command_id, const bool is_stopped, const bool is_near_stop_line) const;

  // Parameter Server
  OnSetParametersCallbackHandle::SharedPtr set_param_res_;
  rcl_interfaces::msg::SetParametersResult onSetParam(
    const std::vector<rclcpp::Parameter> & params);

  // Parameter
  NodeParam node_param_{};
};

}  // namespace autoware::dummy_infrastructure

#endif  // AUTOWARE__DUMMY_INFRASTRUCTURE__DUMMY_INFRASTRUCTURE_NODE_HPP_
