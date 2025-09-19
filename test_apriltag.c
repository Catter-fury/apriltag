#include <stdio.h>
#include <apriltag/apriltag.h>
#include <apriltag/tagStandard41h12.h>

int main() {
    printf("Testing AprilTag library...\n");
    
    apriltag_detector_t *td = apriltag_detector_create();
    if (td == NULL) {
        printf("Failed to create detector\n");
        return 1;
    }
    
    apriltag_family_t *tf = tagStandard41h12_create();
    if (tf == NULL) {
        printf("Failed to create tag family\n");
        apriltag_detector_destroy(td);
        return 1;
    }
    
    apriltag_detector_add_family(td, tf);
    
    printf("✓ AprilTag detector created successfully\n");
    printf("✓ Tag family (Standard41h12) loaded successfully\n");
    
    tagStandard41h12_destroy(tf);
    apriltag_detector_destroy(td);
    
    printf("✓ AprilTag library test passed!\n");
    return 0;
}