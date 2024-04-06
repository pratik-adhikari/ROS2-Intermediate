#include <chrono>
#include <cmath>
#include <functional>
#include <geometry_msgs/msg/twist.hpp>
#include <rcl_interfaces/msg/set_parameters_result.hpp>
#include <rclcpp/rclcpp.hpp>
#include <string>

using namespace std::chrono_literals;

class VelParam : public rclcpp::Node {
public:
  rcl_interfaces::msg::SetParametersResult
  parametersCallback(const std::vector<rclcpp::Parameter> &parameters) {
    rcl_interfaces::msg::SetParametersResult result;
    result.successful = false;
    result.reason = "";
    for (const auto &parameter : parameters) {
      if (parameter.get_name() == "velocity" && parameter.as_double() > 0.2) {
        RCLCPP_INFO(this->get_logger(), "Parameter 'velocity' not changed!");
        result.reason = "Parameter 'velocity' cannot be higher than 0.2";
      } else {
        RCLCPP_INFO(this->get_logger(), "Parameter 'velocity' changed!");
        result.successful = true;
        result.reason = "Parameter 'velocity' is lower than 0.2";
      }
    }
    return result;
  }

  VelParam() : Node("param_vel_node") {
    this->declare_parameter<std::double_t>("velocity", 0.0);
    timer_ = this->create_wall_timer(
        1000ms, std::bind(&VelParam::timer_callback, this));
    publisher_ =
        this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
    callback_handle_ = this->add_on_set_parameters_callback(
        std::bind(&VelParam::parametersCallback, this, std::placeholders::_1));
  }
  void timer_callback() {
    this->get_parameter("velocity", vel_parameter_);
    RCLCPP_INFO(this->get_logger(), "Velocity parameter is: %f",
                vel_parameter_);
    auto message = geometry_msgs::msg::Twist();
    message.linear.x = vel_parameter_;
    publisher_->publish(message);
  }

private:
  std::double_t vel_parameter_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
  OnSetParametersCallbackHandle::SharedPtr callback_handle_;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<VelParam>());
  rclcpp::shutdown();
  return 0;
}
