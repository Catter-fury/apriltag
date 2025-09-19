#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "apriltag.h"
#include "apriltag_pose.h"
#include "tag36h11.h"
#include "tag25h9.h"
#include "tag16h5.h"
#include "common/image_u8.h"
#include "common/pnm.h"

// 计算两点之间的欧式距离
double euclidean_distance(double x, double y, double z) {
    return sqrt(x*x + y*y + z*z);
}

// 从旋转矩阵提取欧拉角
void rotation_matrix_to_euler(matd_t* R, double* roll, double* pitch, double* yaw) {
    double sy = sqrt(MATD_EL(R, 0, 0) * MATD_EL(R, 0, 0) + MATD_EL(R, 1, 0) * MATD_EL(R, 1, 0));
    
    int singular = sy < 1e-6;
    
    if (!singular) {
        *roll = atan2(MATD_EL(R, 2, 1), MATD_EL(R, 2, 2));
        *pitch = atan2(-MATD_EL(R, 2, 0), sy);
        *yaw = atan2(MATD_EL(R, 1, 0), MATD_EL(R, 0, 0));
    } else {
        *roll = atan2(-MATD_EL(R, 1, 2), MATD_EL(R, 1, 1));
        *pitch = atan2(-MATD_EL(R, 2, 0), sy);
        *yaw = 0;
    }
    
    // 转换为度
    *roll = *roll * 180.0 / M_PI;
    *pitch = *pitch * 180.0 / M_PI;
    *yaw = *yaw * 180.0 / M_PI;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <image_file>\n", argv[0]);
        return 1;
    }
    
    // 加载图像
    image_u8_t *im = image_u8_create_from_pnm(argv[1]);
    if (!im) {
        printf("Failed to load image: %s\n", argv[1]);
        return 1;
    }
    
    printf("Image loaded: %dx%d\n", im->width, im->height);
    
    // 创建检测器
    apriltag_detector_t *td = apriltag_detector_create();
    if (!td) {
        printf("Failed to create detector\n");
        image_u8_destroy(im);
        return 1;
    }
    
    // 创建标签族
    apriltag_family_t *tf36h11 = tag36h11_create();
    apriltag_family_t *tf25h9 = tag25h9_create();
    apriltag_family_t *tf16h5 = tag16h5_create();
    
    // 添加标签族到检测器
    apriltag_detector_add_family(td, tf36h11);
    apriltag_detector_add_family(td, tf25h9);
    apriltag_detector_add_family(td, tf16h5);
    
    // 配置检测器
    td->quad_decimate = 1.0;
    td->quad_sigma = 0.0;
    td->nthreads = 1;
    td->debug = 0;
    td->refine_edges = 1;
    
    // 检测标签
    zarray_t *detections = apriltag_detector_detect(td, im);
    
    printf("\nDetection Results with Pose Estimation:\n");
    printf("=====================================\n");
    printf("Found %d tags\n\n", zarray_size(detections));
    
    // 相机内参 (假设的典型值，实际应该通过相机标定获得)
    double fx = 500.0;  // 焦距x
    double fy = 500.0;  // 焦距y
    double cx = im->width / 2.0;   // 主点x
    double cy = im->height / 2.0;  // 主点y
    double tagsize = 0.1;  // 标签尺寸，单位米 (假设10cm)
    
    printf("Camera Parameters (estimated):\n");
    printf("  fx: %.1f pixels\n", fx);
    printf("  fy: %.1f pixels\n", fy);
    printf("  cx: %.1f pixels\n", cx);
    printf("  cy: %.1f pixels\n", cy);
    printf("  Tag size: %.2f meters\n\n", tagsize);
    
    // 处理每个检测到的标签
    for (int i = 0; i < zarray_size(detections); i++) {
        apriltag_detection_t *det;
        zarray_get(detections, i, &det);
        
        printf("Tag %d:\n", i + 1);
        printf("  ID: %d\n", det->id);
        printf("  Family: %s\n", det->family->name);
        printf("  Hamming distance: %d\n", det->hamming);
        printf("  Decision margin: %.3f\n", det->decision_margin);
        printf("  Center: (%.2f, %.2f)\n", det->c[0], det->c[1]);
        
        // 准备姿态估计信息
        apriltag_detection_info_t info;
        info.det = det;
        info.tagsize = tagsize;
        info.fx = fx;
        info.fy = fy;
        info.cx = cx;
        info.cy = cy;
        
        // 估计姿态
        apriltag_pose_t pose;
        pose.R = matd_create(3, 3);
        pose.t = matd_create(3, 1);
        
        double error = estimate_tag_pose(&info, &pose);
        
        // 提取位置和旋转
        double tx = MATD_EL(pose.t, 0, 0);
        double ty = MATD_EL(pose.t, 1, 0);
        double tz = MATD_EL(pose.t, 2, 0);
        
        double distance = euclidean_distance(tx, ty, tz);
        
        double roll, pitch, yaw;
        rotation_matrix_to_euler(pose.R, &roll, &pitch, &yaw);
        
        printf("  Pose Estimation:\n");
        printf("    Distance: %.3f meters\n", distance);
        printf("    Position (x,y,z): (%.3f, %.3f, %.3f) meters\n", tx, ty, tz);
        printf("    Rotation (roll,pitch,yaw): (%.1f°, %.1f°, %.1f°)\n", roll, pitch, yaw);
        printf("    Pose error: %.6f\n", error);
        
        // 输出旋转矩阵
        printf("    Rotation Matrix:\n");
        for (int row = 0; row < 3; row++) {
            printf("      [");
            for (int col = 0; col < 3; col++) {
                printf(" %8.4f", MATD_EL(pose.R, row, col));
            }
            printf(" ]\n");
        }
        
        printf("  Corner coordinates:\n");
        for (int j = 0; j < 4; j++) {
            printf("    Corner %d: (%.2f, %.2f)\n", j + 1, det->p[j][0], det->p[j][1]);
        }
        printf("\n");
        
        // 清理内存
        matd_destroy(pose.R);
        matd_destroy(pose.t);
    }
    
    // 清理
    apriltag_detections_destroy(detections);
    apriltag_detector_destroy(td);
    tag36h11_destroy(tf36h11);
    tag25h9_destroy(tf25h9);
    tag16h5_destroy(tf16h5);
    image_u8_destroy(im);
    
    return 0;
}