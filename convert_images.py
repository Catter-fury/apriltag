#!/usr/bin/env python3

import os
import sys
from PIL import Image

def convert_png_to_pgm(input_dir, output_dir):
    """Convert PNG images to PGM format for AprilTag processing"""
    
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    converted_files = []
    
    for filename in os.listdir(input_dir):
        if filename.lower().endswith('.png'):
            input_path = os.path.join(input_dir, filename)
            output_filename = filename.replace('.png', '.pgm')
            output_path = os.path.join(output_dir, output_filename)
            
            try:
                # Open and convert to grayscale
                with Image.open(input_path) as img:
                    # Convert to grayscale
                    gray_img = img.convert('L')
                    
                    # Save as PGM
                    gray_img.save(output_path, 'PPM')
                    
                    print(f"Converted: {filename} -> {output_filename}")
                    converted_files.append(output_path)
                    
            except Exception as e:
                print(f"Error converting {filename}: {e}")
    
    return converted_files

if __name__ == "__main__":
    input_dir = "ros2ws/apriltag"
    output_dir = "converted_images"
    
    print("Converting PNG images to PGM format...")
    converted_files = convert_png_to_pgm(input_dir, output_dir)
    
    print(f"\nConverted {len(converted_files)} files:")
    for filepath in converted_files:
        print(f"  {filepath}")