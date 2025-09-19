# AprilTag OpenCV 处理器

这是一个用于处理OpenCV图像数据并检测AprilTag标签的完整解决方案。

## 文件说明

- `apriltag_opencv_processor.c` - 核心处理器实现
- `apriltag_opencv_processor.h` - 头文件
- `example_usage.c` - 使用示例
- `Makefile.processor` - 编译脚本

## 主要功能

1. **多格式支持**: 支持灰度图像(1通道)和彩色图像(3通道)
2. **多标签家族**: 支持tag36h11、tag25h9、tag16h5
3. **完整信息输出**: 提供标签ID、家族、位置、角点、置信度等信息

## 核心函数

### `apriltag_processor_init()`
初始化AprilTag检测器，必须在使用前调用。

### `process_opencv_image()`
```c
int process_opencv_image(uint8_t* image_data, int width, int height, int channels, 
                        apriltag_result_t* results, int max_results);
```
- `image_data`: OpenCV图像数据指针
- `width`: 图像宽度
- `height`: 图像高度  
- `channels`: 通道数(1=灰度, 3=RGB)
- `results`: 结果数组
- `max_results`: 最大检测数量
- 返回值: 检测到的标签数量

### `apriltag_processor_cleanup()`
清理资源，程序结束前调用。

## 检测结果结构

```c
typedef struct {
    int id;                    // 标签ID
    char family_name[32];      // 标签家族名称
    double center_x, center_y; // 中心坐标
    double corners[4][2];      // 四个角点坐标
    int hamming_distance;      // 汉明距离(0最佳)
    double decision_margin;    // 决策边界(越大越可靠)
} apriltag_result_t;
```

## 使用步骤

1. **编译**:
```bash
make -f Makefile.processor
```

2. **在你的ROS2代码中集成**:
```c
#include "apriltag_opencv_processor.h"

// 初始化
apriltag_processor_init();

// 处理图像回调
void image_callback(const sensor_msgs::msg::Image::SharedPtr msg) {
    uint8_t* image_data = msg->data.data();
    int width = msg->width;
    int height = msg->height;
    int channels = msg->step / msg->width;
    
    apriltag_result_t results[10];
    int num_detections = process_opencv_image(image_data, width, height, channels, 
                                            results, 10);
    
    // 处理检测结果
    for (int i = 0; i < num_detections; i++) {
        printf("检测到标签 ID: %d, 位置: (%.2f, %.2f)\n", 
               results[i].id, results[i].center_x, results[i].center_y);
    }
}

// 程序结束时清理
apriltag_processor_cleanup();
```

## ROS2集成建议

1. **订阅图像话题**:
```cpp
auto subscription = this->create_subscription<sensor_msgs::msg::Image>(
    "camera/image", 10, std::bind(&YourNode::image_callback, this, _1));
```

2. **发布检测结果**:
创建自定义消息类型或使用geometry_msgs发布标签位置

3. **参数配置**:
可以通过ROS2参数配置检测器参数(decimation, threads等)

## 性能优化

- 设置合适的`quad_decimate`值以平衡速度和精度
- 对于实时应用，建议使用多线程(`nthreads > 1`)
- 根据需要选择检测的标签家族以提高速度

## 注意事项

- 确保图像数据格式正确(灰度或RGB)
- 检测结果的坐标系以图像左上角为原点
- Hamming距离为0表示完美检测
- Decision margin越大表示检测越可靠