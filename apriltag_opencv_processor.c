#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "apriltag.h"
#include "tag36h11.h"
#include "tag25h9.h"
#include "tag16h5.h"
#include "common/image_u8.h"

typedef struct {
    int id;
    char family_name[32];
    double center_x, center_y;
    double corners[4][2];
    int hamming_distance;
    double decision_margin;
} apriltag_result_t;

typedef struct {
    apriltag_detector_t *detector;
    apriltag_family_t *tf_36h11;
    apriltag_family_t *tf_25h9;
    apriltag_family_t *tf_16h5;
    int initialized;
} apriltag_processor_t;

static apriltag_processor_t g_processor = {0};

// 初始化AprilTag检测器
int apriltag_processor_init() {
    if (g_processor.initialized) {
        return 0;
    }
    
    g_processor.detector = apriltag_detector_create();
    if (!g_processor.detector) {
        printf("Failed to create AprilTag detector\n");
        return -1;
    }
    
    // 创建多个标签家族
    g_processor.tf_36h11 = tag36h11_create();
    g_processor.tf_25h9 = tag25h9_create();
    g_processor.tf_16h5 = tag16h5_create();
    
    if (!g_processor.tf_36h11 || !g_processor.tf_25h9 || !g_processor.tf_16h5) {
        printf("Failed to create tag families\n");
        return -1;
    }
    
    // 添加标签家族到检测器
    apriltag_detector_add_family(g_processor.detector, g_processor.tf_36h11);
    apriltag_detector_add_family(g_processor.detector, g_processor.tf_25h9);
    apriltag_detector_add_family(g_processor.detector, g_processor.tf_16h5);
    
    // 设置检测参数
    g_processor.detector->quad_decimate = 1.0;
    g_processor.detector->quad_sigma = 0.0;
    g_processor.detector->nthreads = 1;
    g_processor.detector->debug = 0;
    g_processor.detector->refine_edges = 1;
    
    g_processor.initialized = 1;
    printf("AprilTag processor initialized successfully\n");
    return 0;
}

// 清理资源
void apriltag_processor_cleanup() {
    if (!g_processor.initialized) {
        return;
    }
    
    if (g_processor.detector) {
        apriltag_detector_destroy(g_processor.detector);
    }
    
    if (g_processor.tf_36h11) {
        tag36h11_destroy(g_processor.tf_36h11);
    }
    
    if (g_processor.tf_25h9) {
        tag25h9_destroy(g_processor.tf_25h9);
    }
    
    if (g_processor.tf_16h5) {
        tag16h5_destroy(g_processor.tf_16h5);
    }
    
    g_processor.initialized = 0;
}

// 从OpenCV Mat数据创建image_u8_t
image_u8_t* create_image_u8_from_opencv_data(uint8_t* data, int width, int height, int channels) {
    image_u8_t* im = image_u8_create(width, height);
    if (!im) {
        return NULL;
    }
    
    if (channels == 1) {
        // 灰度图像，直接复制
        for (int y = 0; y < height; y++) {
            memcpy(&im->buf[y * im->stride], &data[y * width], width);
        }
    } else if (channels == 3) {
        // RGB图像，转换为灰度
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                uint8_t r = data[y * width * 3 + x * 3 + 0];
                uint8_t g = data[y * width * 3 + x * 3 + 1];
                uint8_t b = data[y * width * 3 + x * 3 + 2];
                
                // 使用标准RGB到灰度转换公式
                uint8_t gray = (uint8_t)(0.299 * r + 0.587 * g + 0.114 * b);
                im->buf[y * im->stride + x] = gray;
            }
        }
    } else {
        printf("Unsupported number of channels: %d\n", channels);
        image_u8_destroy(im);
        return NULL;
    }
    
    return im;
}

// 处理OpenCV图像并检测AprilTag
int process_opencv_image(uint8_t* image_data, int width, int height, int channels, 
                        apriltag_result_t* results, int max_results) {
    
    if (!g_processor.initialized) {
        printf("AprilTag processor not initialized\n");
        return -1;
    }
    
    // 创建image_u8_t结构
    image_u8_t* im = create_image_u8_from_opencv_data(image_data, width, height, channels);
    if (!im) {
        printf("Failed to create image_u8_t from OpenCV data\n");
        return -1;
    }
    
    // 执行检测
    zarray_t* detections = apriltag_detector_detect(g_processor.detector, im);
    
    int num_detections = zarray_size(detections);
    if (num_detections > max_results) {
        num_detections = max_results;
    }
    
    printf("Detected %d AprilTags in %dx%d image\n", num_detections, width, height);
    
    // 提取检测结果
    for (int i = 0; i < num_detections; i++) {
        apriltag_detection_t* det;
        zarray_get(detections, i, &det);
        
        results[i].id = det->id;
        strncpy(results[i].family_name, det->family->name, sizeof(results[i].family_name) - 1);
        results[i].family_name[sizeof(results[i].family_name) - 1] = '\0';
        
        results[i].center_x = det->c[0];
        results[i].center_y = det->c[1];
        
        for (int j = 0; j < 4; j++) {
            results[i].corners[j][0] = det->p[j][0];
            results[i].corners[j][1] = det->p[j][1];
        }
        
        results[i].hamming_distance = det->hamming;
        results[i].decision_margin = det->decision_margin;
        
        printf("Tag %d: ID=%d, Family=%s, Center=(%.2f, %.2f), Hamming=%d, Margin=%.3f\n",
               i, results[i].id, results[i].family_name, 
               results[i].center_x, results[i].center_y,
               results[i].hamming_distance, results[i].decision_margin);
    }
    
    // 清理资源
    apriltag_detections_destroy(detections);
    image_u8_destroy(im);
    
    return num_detections;
}

// 打印检测结果详细信息
void print_detection_results(apriltag_result_t* results, int num_results) {
    printf("\n=== AprilTag Detection Results ===\n");
    printf("Total detections: %d\n\n", num_results);
    
    for (int i = 0; i < num_results; i++) {
        printf("Detection %d:\n", i + 1);
        printf("  Tag ID: %d\n", results[i].id);
        printf("  Family: %s\n", results[i].family_name);
        printf("  Center: (%.2f, %.2f)\n", results[i].center_x, results[i].center_y);
        printf("  Hamming Distance: %d\n", results[i].hamming_distance);
        printf("  Decision Margin: %.3f\n", results[i].decision_margin);
        printf("  Corners:\n");
        for (int j = 0; j < 4; j++) {
            printf("    Corner %d: (%.2f, %.2f)\n", j + 1, 
                   results[i].corners[j][0], results[i].corners[j][1]);
        }
        printf("\n");
    }
}

// 测试函数（仅在需要时使用）
void test_processor() {
    printf("AprilTag OpenCV Processor Test\n");
    
    // 初始化处理器
    if (apriltag_processor_init() != 0) {
        return;
    }
    
    printf("Processor initialized. Ready to process OpenCV images.\n");
    printf("Call process_opencv_image() with your OpenCV image data.\n");
    
    // 清理
    apriltag_processor_cleanup();
}