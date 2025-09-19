#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "apriltag.h"
#include "tag36h11.h"
#include "common/image_u8.h"
#include "common/pjpeg.h"

int main(int argc, char *argv[]) {
    const char *image_dir = "ros2ws/apriltag/";
    
    if (argc > 1) {
        image_dir = argv[1];
    }
    
    printf("AprilTag Detection Program\n");
    printf("Scanning directory: %s\n", image_dir);
    
    apriltag_family_t *tf = tag36h11_create();
    if (!tf) {
        printf("Failed to create tag family\n");
        return 1;
    }
    
    apriltag_detector_t *td = apriltag_detector_create();
    if (!td) {
        printf("Failed to create detector\n");
        tag36h11_destroy(tf);
        return 1;
    }
    
    apriltag_detector_add_family(td, tf);
    
    td->quad_decimate = 1.0;
    td->quad_sigma = 0.0;
    td->nthreads = 1;
    td->debug = 0;
    td->refine_edges = 1;
    
    DIR *dir = opendir(image_dir);
    if (!dir) {
        printf("Cannot open directory: %s\n", image_dir);
        apriltag_detector_destroy(td);
        tag36h11_destroy(tf);
        return 1;
    }
    
    struct dirent *entry;
    int total_detections = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".pgm") || strstr(entry->d_name, ".pnm") || strstr(entry->d_name, ".png") || strstr(entry->d_name, ".jpg")) {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", image_dir, entry->d_name);
            
            printf("\nProcessing: %s\n", entry->d_name);
            
            image_u8_t *im = NULL;
            
            if (strstr(entry->d_name, ".pgm") || strstr(entry->d_name, ".pnm") || strstr(entry->d_name, ".png")) {
                im = image_u8_create_from_pnm(filepath);
            } else if (strstr(entry->d_name, ".jpg")) {
                int err = 0;
                pjpeg_t *pjpeg = pjpeg_create_from_file(filepath, 0, &err);
                if (pjpeg) {
                    im = pjpeg_to_u8_baseline(pjpeg);
                    pjpeg_destroy(pjpeg);
                }
            }
            
            if (!im) {
                printf("  Failed to load image\n");
                continue;
            }
            
            printf("  Image size: %dx%d\n", im->width, im->height);
            
            zarray_t *detections = apriltag_detector_detect(td, im);
            
            printf("  Detections found: %d\n", zarray_size(detections));
            
            for (int i = 0; i < zarray_size(detections); i++) {
                apriltag_detection_t *det;
                zarray_get(detections, i, &det);
                
                printf("    Tag ID: %d\n", det->id);
                printf("    Family: %s\n", det->family->name);
                printf("    Hamming distance: %d\n", det->hamming);
                printf("    Decision margin: %.3f\n", det->decision_margin);
                printf("    Center: (%.2f, %.2f)\n", det->c[0], det->c[1]);
                printf("    Corners:\n");
                for (int j = 0; j < 4; j++) {
                    printf("      (%.2f, %.2f)\n", det->p[j][0], det->p[j][1]);
                }
                printf("\n");
                
                total_detections++;
            }
            
            apriltag_detections_destroy(detections);
            image_u8_destroy(im);
        }
    }
    
    closedir(dir);
    
    printf("=== SUMMARY ===\n");
    printf("Total tags detected: %d\n", total_detections);
    
    apriltag_detector_destroy(td);
    tag36h11_destroy(tf);
    
    return 0;
}