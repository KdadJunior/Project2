#include <iostream>
#include <fstream>
#include <vector>

struct RGB {
    int r, g, b;
};

// Function to load a PPM image (supports both P3 and P6)
bool loadPPM(const std::string& filename, std::vector<std::vector<RGB>>& image, int& width, int& height, std::string& format) {
    std::ifstream file(filename, std::ios::binary); // Open in binary mode
    if (!file) {
        std::cerr << "Error: Cannot open " << filename << "\n";
        return false;
    }

    file >> format;  // Read PPM format (P3 or P6)
    
    if (format != "P3" && format != "P6") {
        std::cerr << "Error: Unsupported PPM format! Expected P3 or P6.\n";
        return false;
    }

    file >> width >> height;
    int maxVal;
    file >> maxVal;
    file.ignore(); // Ignore the newline character before pixel data

    image.resize(height, std::vector<RGB>(width));

    if (format == "P3") {
        std::cout << "Reading P3 PPM (plain text)\n";
        // Read plain-text pixel values
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                file >> image[i][j].r >> image[i][j].g >> image[i][j].b;
            }
        }
    } else if (format == "P6") {
        std::cout << "Reading P6 PPM (binary)\n";
        // Read binary pixel values
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                unsigned char rgb[3];
                file.read(reinterpret_cast<char*>(rgb), 3);
                image[i][j] = {rgb[0], rgb[1], rgb[2]};
            }
        }
    }

    file.close();
    return true;
}

// Function to print the PPM image pixels
void printPPMPixels(const std::vector<std::vector<RGB>>& image) {
    std::cout << "PPM Image Pixels (R G B format):\n";
    
    for (const auto& row : image) {
        for (const auto& pixel : row) {
            std::cout << "(" << pixel.r << ", " << pixel.g << ", " << pixel.b << ") ";
        }
        std::cout << "\n";  // Move to the next row for formatting
    }
    std::cout << "--------------------------\n";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <ppm_file>\n";
        return 1;
    }

    std::string ppmFile = argv[1];
    std::vector<std::vector<RGB>> image;
    int width, height;
    std::string format;

    // Load the PPM file
    if (loadPPM(ppmFile, image, width, height, format)) {
        std::cout << "Loaded " << format << " image: " << width << "x" << height << "\n";
        printPPMPixels(image);  // Print pixel values
    } else {
        std::cerr << "Failed to load PPM image.\n";
    }

    return 0;
}
