#include <stdio.h>
#include <stdlib.h>
#include "apriltag_opencv_processor.h"

// 模拟从ROS2接收OpenCV图像数据的回调函数
void image_callback(uint8_t* image_data, int width, int height, int channels) {
    printf("Received image: %dx%d, channels: %d\n", width, height, channels);
    
    // 准备结果数组
    apriltag_result_t results[10];  // 最多检测10个标签
    
    // 处理图像
    int num_detections = process_opencv_image(image_data, width, height, channels, 
                                            results, 10);
    
    if (num_detections > 0) {
        printf("检测到 %d 个AprilTag标签:\n", num_detections);
        
        // 输出每个标签的信息
        for (int i = 0; i < num_detections; i++) {
            printf("标签 %d:\n", i + 1);
            printf("  ID: %d\n", results[i].id);
            printf("  类别: %s\n", results[i].family_name);
            printf("  中心位置: (%.2f, %.2f)\n", results[i].center_x, results[i].center_y);
            printf("  置信度: %.3f\n", results[i].decision_margin);
            printf("  四个角点:\n");
            for (int j = 0; j < 4; j++) {
                printf("    角点%d: (%.2f, %.2f)\n", j + 1, 
                       results[i].corners[j][0], results[i].corners[j][1]);
            }
            printf("\n");
        }
    } else {
        printf("未检测到AprilTag标签\n");
    }
}

int main() {
    printf("AprilTag OpenCV处理器使用示例\n");
    
    // 初始化处理器
    if (apriltag_processor_init() != 0) {
        printf("初始化失败\n");
        return -1;
    }
    
    printf("处理器初始化成功\n");
    
    // 这里你可以添加ROS2订阅器或其他图像获取逻辑
    // 当收到图像时，调用 image_callback 函数
    
    /*
    示例ROS2集成代码结构:
    
    void ros2_image_callback(const sensor_msgs::msg::Image::SharedPtr msg) {
        // 获取图像数据
        uint8_t* image_data = msg->data.data();
        int width = msg->width;
        int height = msg->height;
        int channels = msg->step / msg->width;  // 根据实际格式调整
        
        // 调用处理函数
        image_callback(image_data, width, height, channels);
    }
    */
    
    printf("准备接收图像数据...\n");
    
    // 程序保持运行等待图像数据
    // 在实际应用中，这里会是ROS2的spin循环
    
    // 清理资源
    apriltag_processor_cleanup();
    
    return 0;
}