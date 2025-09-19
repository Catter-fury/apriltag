#ifndef APRILTAG_OPENCV_PROCESSOR_H
#define APRILTAG_OPENCV_PROCESSOR_H

#include <stdint.h>

// AprilTag检测结果结构体
typedef struct {
    int id;                    // 标签ID
    char family_name[32];      // 标签家族名称
    double center_x, center_y; // 中心坐标
    double corners[4][2];      // 四个角点坐标
    int hamming_distance;      // 汉明距离
    double decision_margin;    // 决策边界
} apriltag_result_t;

// 函数声明
int apriltag_processor_init();
void apriltag_processor_cleanup();

int process_opencv_image(uint8_t* image_data, int width, int height, int channels, 
                        apriltag_result_t* results, int max_results);

void print_detection_results(apriltag_result_t* results, int num_results);

#endif // APRILTAG_OPENCV_PROCESSOR_H