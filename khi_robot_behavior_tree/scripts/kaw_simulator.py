import rclpy
from rclpy.node import Node
from khi_robot_interfaces.srv import KhiRobotCmd
from khi_robot_interfaces.msg import MotionReadyEnum
from std_srvs.srv import Trigger

class RobotServiceNode(Node):
    def __init__(self):
        super().__init__('kaw_simulator')

        self.create_service(KhiRobotCmd, 'khi_robot_cmd', self.enable_robot_callback)
        self.create_service(Trigger, 'stop_traj_mode', self.disable_robot_callback)
        self.create_service(KhiRobotCmd, 'start_point_queue_mode', self.enable_robot_callback)
        self.io = {}

    async def enable_robot_callback(self, request, response):
        response.message = 'Robot enabled'
        response.result_code = MotionReadyEnum()
        response.result_code.value = MotionReadyEnum.READY
        return response


    async def disable_robot_callback(self, request, response):
        response.message = 'Robot disabled'
        response.success = True
        return response

def main(args=None):
    rclpy.init(args=args)
    node = RobotServiceNode()
    node.get_logger().info('Started Kaw simulator')
    rclpy.spin(node)
    rclpy.shutdown()

if __name__ == '__main__':
    main()