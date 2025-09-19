#!/usr/bin/env python3
from PIL import Image, ImageDraw, ImageFont
import sys

def annotate_apriltag_detection():
    # Load the original image
    img = Image.open('test/data/ap.png')
    if img is None:
        print("Failed to load image")
        return
    
    # Detection results from our C program with pose estimation
    detections = [
        {
            'id': 0,
            'family': 'tag36h11',
            'hamming': 0,
            'decision_margin': 110.274,
            'center': (253.86, 275.69),
            'corners': [
                (286.76, 242.93),
                (220.93, 242.93),
                (220.93, 308.47),
                (286.77, 308.44)
            ],
            'pose': {
                'distance': 0.770,
                'position': (-0.023, 0.118, 0.761),
                'rotation': (-0.7, 0.3, -180.0),
                'pose_error': 0.000000
            }
        }
    ]
    
    # Create annotated image
    annotated = img.copy()
    draw = ImageDraw.Draw(annotated)
    
    for i, det in enumerate(detections):
        # Draw corners as points
        for j, corner in enumerate(det['corners']):
            x, y = int(corner[0]), int(corner[1])
            draw.ellipse([x-4, y-4, x+4, y+4], fill=(0, 255, 0))
        
        # Draw tag boundary
        corners = det['corners']
        for j in range(4):
            x1, y1 = corners[j]
            x2, y2 = corners[(j+1) % 4]
            draw.line([x1, y1, x2, y2], fill=(0, 255, 0), width=2)
        
        # Draw center point
        center_x, center_y = int(det['center'][0]), int(det['center'][1])
        draw.ellipse([center_x-6, center_y-6, center_x+6, center_y+6], fill=(255, 0, 0))
        
        # Add text annotation with pose information
        text_y = center_y - 60
        draw.text((center_x - 30, text_y), f"ID: {det['id']}", fill=(255, 255, 255))
        draw.text((center_x - 50, text_y + 15), f"Family: {det['family']}", fill=(255, 255, 255))
        draw.text((center_x - 50, text_y + 30), f"Distance: {det['pose']['distance']:.2f}m", fill=(255, 255, 0))
        draw.text((center_x - 50, text_y + 45), f"Margin: {det['decision_margin']:.1f}", fill=(255, 255, 255))
        
        # Add 3D position information
        pos = det['pose']['position']
        draw.text((center_x - 70, text_y + 60), f"Position: ({pos[0]:.2f}, {pos[1]:.2f}, {pos[2]:.2f})", fill=(0, 255, 255))
        
        # Add rotation information
        rot = det['pose']['rotation']
        draw.text((center_x - 70, text_y + 75), f"Rotation: ({rot[0]:.1f}°, {rot[1]:.1f}°, {rot[2]:.1f}°)", fill=(0, 255, 255))
    
    # Save annotated image
    annotated.save('test/data/ap_annotated.png')
    print("Annotated image saved as 'test/data/ap_annotated.png'")
    
    # Print summary
    print("\nAprilTag Detection Summary:")
    print("=" * 40)
    print(f"Image size: {img.size[0]}x{img.size[1]}")
    print(f"Tags detected: {len(detections)}")
    print()
    
    for i, det in enumerate(detections):
        print(f"Tag {i+1}:")
        print(f"  ID: {det['id']}")
        print(f"  Family: {det['family']}")
        print(f"  Hamming distance: {det['hamming']}")
        print(f"  Decision margin: {det['decision_margin']:.3f}")
        print(f"  Center position: ({det['center'][0]:.2f}, {det['center'][1]:.2f})")
        print(f"  Corner coordinates:")
        for j, corner in enumerate(det['corners']):
            print(f"    Corner {j+1}: ({corner[0]:.2f}, {corner[1]:.2f})")
        
        # Add pose information
        pose = det['pose']
        print(f"  Pose Estimation:")
        print(f"    Distance: {pose['distance']:.3f} meters")
        pos = pose['position']
        print(f"    3D Position (x,y,z): ({pos[0]:.3f}, {pos[1]:.3f}, {pos[2]:.3f}) meters")
        rot = pose['rotation']
        print(f"    Rotation (roll,pitch,yaw): ({rot[0]:.1f}°, {rot[1]:.1f}°, {rot[2]:.1f}°)")
        print(f"    Pose error: {pose['pose_error']:.6f}")
        print()

if __name__ == "__main__":
    annotate_apriltag_detection()