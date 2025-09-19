#include <stdio.h>
#include <stdlib.h>

#include "apriltag.h"
#include "tag36h11.h"
#include "tag25h9.h"
#include "tag16h5.h"
#include "common/image_u8.h"
#include "common/pnm.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <image_file>\n", argv[0]);
        return 1;
    }
    
    // Load image
    image_u8_t *im = image_u8_create_from_pnm(argv[1]);
    if (!im) {
        printf("Failed to load image: %s\n", argv[1]);
        return 1;
    }
    
    printf("Image loaded: %dx%d\n", im->width, im->height);
    
    // Create detector
    apriltag_detector_t *td = apriltag_detector_create();
    if (!td) {
        printf("Failed to create detector\n");
        image_u8_destroy(im);
        return 1;
    }
    
    // Create tag families
    apriltag_family_t *tf36h11 = tag36h11_create();
    apriltag_family_t *tf25h9 = tag25h9_create();
    apriltag_family_t *tf16h5 = tag16h5_create();
    
    // Add families to detector
    apriltag_detector_add_family(td, tf36h11);
    apriltag_detector_add_family(td, tf25h9);
    apriltag_detector_add_family(td, tf16h5);
    
    // Configure detector
    td->quad_decimate = 1.0;
    td->quad_sigma = 0.0;
    td->nthreads = 1;
    td->debug = 0;
    td->refine_edges = 1;
    
    // Detect tags
    zarray_t *detections = apriltag_detector_detect(td, im);
    
    printf("\nDetection Results:\n");
    printf("==================\n");
    printf("Found %d tags\n\n", zarray_size(detections));
    
    // Print detection details
    for (int i = 0; i < zarray_size(detections); i++) {
        apriltag_detection_t *det;
        zarray_get(detections, i, &det);
        
        printf("Tag %d:\n", i + 1);
        printf("  ID: %d\n", det->id);
        printf("  Family: %s\n", det->family->name);
        printf("  Hamming distance: %d\n", det->hamming);
        printf("  Decision margin: %.3f\n", det->decision_margin);
        printf("  Center: (%.2f, %.2f)\n", det->c[0], det->c[1]);
        printf("  Corners:\n");
        for (int j = 0; j < 4; j++) {
            printf("    Corner %d: (%.2f, %.2f)\n", j + 1, det->p[j][0], det->p[j][1]);
        }
        printf("\n");
    }
    
    // Cleanup
    apriltag_detections_destroy(detections);
    apriltag_detector_destroy(td);
    tag36h11_destroy(tf36h11);
    tag25h9_destroy(tf25h9);
    tag16h5_destroy(tf16h5);
    image_u8_destroy(im);
    
    return 0;
}