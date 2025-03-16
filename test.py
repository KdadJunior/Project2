#!/usr/bin/env python3

import subprocess
import sys
import os

def read_ppm(filename):
    """
    Reads a P6 PPM file and returns width, height, and a 2D list of pixels (tuples of (r, g, b)).
    """
    with open(filename, "rb") as f:
        # Read the magic number (should be P6)
        magic = f.readline().strip()
        if magic != b"P6":
            raise ValueError("Unsupported format: Expected P6, got {}".format(magic))
        
        # Read dimensions, skipping comment lines
        line = f.readline().strip()
        while line.startswith(b"#"):
            line = f.readline().strip()
        dims = line.split()
        if len(dims) != 2:
            raise ValueError("Invalid header: dimensions not found.")
        width = int(dims[0])
        height = int(dims[1])
        
        # Read maximum color value
        max_val_line = f.readline().strip()
        max_val = int(max_val_line)
        if max_val != 255:
            raise ValueError("Unsupported max value: {}".format(max_val))
        
        # Read pixel data (width * height * 3 bytes)
        pixel_data = f.read(width * height * 3)
        if len(pixel_data) != width * height * 3:
            raise ValueError("Pixel data is incomplete.")
        
        # Convert the binary data into a 2D list of pixel tuples
        pixels = []
        for i in range(height):
            row = []
            for j in range(width):
                idx = (i * width + j) * 3
                r = pixel_data[idx]
                g = pixel_data[idx + 1]
                b = pixel_data[idx + 2]
                row.append((r, g, b))
            pixels.append(row)
    return width, height, pixels

def compare_ppm(file1, file2):
    """
    Compares two PPM files pixel-by-pixel.
    Returns True if images are identical, otherwise prints differences and returns False.
    """
    w1, h1, pixels1 = read_ppm(file1)
    w2, h2, pixels2 = read_ppm(file2)
    
    if w1 != w2 or h1 != h2:
        print("Dimension mismatch: {}x{} vs {}x{}".format(w1, h1, w2, h2))
        return False
    
    identical = True
    for i in range(h1):
        for j in range(w1):
            if pixels1[i][j] != pixels2[i][j]:
                print("Difference at pixel ({}, {}): expected {} but got {}".format(
                    i, j, pixels2[i][j], pixels1[i][j]
                ))
                identical = False
    return identical

def main():
    if len(sys.argv) < 4:
        print("Usage: {} <input.ppm> <expected_output.ppm> <proj02 options...>".format(sys.argv[0]))
        print("Example: {} car.ppm car_inverted.ppm -i".format(sys.argv[0]))
        sys.exit(1)
    
    input_file = sys.argv[1]
    expected_file = sys.argv[2]
    proj_options = sys.argv[3:]
    
    output_file = "temp_output.ppm"
    
    # Build the command to run your project executable.
    # The order is: proj02 <input_file> <output_file> <options>
    cmd = ["./proj02", input_file, output_file] + proj_options
    print("Running command:", " ".join(cmd))
    
    # Run the executable and capture its output.
    result = subprocess.run(cmd, capture_output=True, text=True)
    print(result.stdout)
    if result.returncode != 0:
        print("Error running proj02:", result.stderr)
        sys.exit(1)
    
    # Compare the produced output with the expected output.
    print("Comparing output with expected file...")
    if compare_ppm(output_file, expected_file):
        print("Test Passed: Images are identical.")
    else:
        print("Test Failed: Differences found.")
    
    # Clean up the temporary output file.
    try:
        os.remove(output_file)
    except Exception as e:
        print("Warning: could not remove temporary file:", e)

if __name__ == "__main__":
    main()
