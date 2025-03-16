#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <filesystem>

// Structure to represent an RGB pixel
struct RGB
{
    unsigned char r, g, b;
};

std::vector<std::vector<RGB>> readPPM(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string magic;
    file >> magic;
    if (magic != "P6")
    {
        throw std::runtime_error("Invalid PPM format: " + magic);
    }

    int width, height, max_val;
    std::string line;

    // Read width and height, skipping comments and empty lines
    while (true)
    {
        std::getline(file, line);
        if (!file) throw std::runtime_error("Error reading PPM header.");
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        if (iss >> width >> height) break;
    }

    // Read max color value, skipping comments and empty lines
    while (true)
    {
        std::getline(file, line);
        if (!file) throw std::runtime_error("Error reading max color value.");
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        if (iss >> max_val) break;
    }

    if (max_val != 255)
    {
        throw std::runtime_error("Unsupported max value: " + std::to_string(max_val));
    }

    // Consume any additional whitespace (e.g., newline) before binary data.
    while (std::isspace(file.peek()))
    {
        file.get();
    }

    std::cout << "PPM File: " << filename << "\n";
    std::cout << "Width: " << width << ", Height: " << height << ", Max Value: " << max_val << "\n";

    // Read binary pixel data
    std::vector<std::vector<RGB>> image(height, std::vector<RGB>(width));
    for (int i = 0; i < height; ++i)
    {
        file.read(reinterpret_cast<char *>(image[i].data()), width * 3);
        if (file.gcount() != width * 3)
        {
            std::cout << "Error reading pixel data at row " << i << "\n";
            std::cout << "Bytes read so far: " << file.gcount() << "\n";
            throw std::runtime_error("Error reading pixel data at row " + std::to_string(i));
        }
    }

    std::cout << "First few pixels (R, G, B): ";
    for (int i = 0; i < std::min(5, height); ++i)
    {
        for (int j = 0; j < std::min(5, width); ++j)
        {
            std::cout << "(" << static_cast<int>(image[i][j].r) << ", " 
                      << static_cast<int>(image[i][j].g) << ", " 
                      << static_cast<int>(image[i][j].b) << ") ";
        }
    }
    std::cout << "\n";

    return image;
}

void writePPM(const std::string &filename, const std::vector<std::vector<RGB>> &image) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    int height = image.size();
    int width = (height > 0) ? image[0].size() : 0;

    if (width == 0 || height == 0) {
        throw std::runtime_error("Empty image data.");
    }

    // Write PPM header
    file << "P6\n" << width << " " << height << "\n255\n";

    std::cout << "Writing PPM file: " << filename << "\n";

    std::cout << "First few pixels before writing:\n";
    for (int i = 0; i < std::min(5, height); ++i) {
        for (int j = 0; j < std::min(5, width); ++j) {
            std::cout << "(" << static_cast<int>(image[i][j].r) << ", "
                      << static_cast<int>(image[i][j].g) << ", "
                      << static_cast<int>(image[i][j].b) << ") ";
        }
        std::cout << "\n";
    }

    // Write pixel data row-by-row
    for (int i = 0; i < height; ++i) {
        file.write(reinterpret_cast<const char*>(image[i].data()), width * 3);
        if (!file) {
            throw std::runtime_error("Error writing pixel data at row " + std::to_string(i));
        }
    }

    file.flush();
    file.close();

    // Verify successful file write by checking that the file size is reasonable.
    std::ifstream testFile(filename, std::ios::binary | std::ios::ate);
    if (!testFile.is_open()) {
        throw std::runtime_error("Error verifying written file: " + filename);
    }
    std::streamsize size = testFile.tellg();
    testFile.close();

    // We expect at least (width * height * 3) bytes of pixel data plus a small overhead for the header.
    const std::streamsize expectedMinSize = (width * height * 3) + 10; // 15 bytes as an approximate header size
    if (size < expectedMinSize) {
        throw std::runtime_error("File size mismatch: Possible truncation. Expected at least " +
                                     std::to_string(expectedMinSize) + " bytes, got " + std::to_string(size) + " bytes.");
    }

    std::cout << "PPM file successfully written: " << filename << "\n";
}


// Function to convert image to grayscale
void grayscale(std::vector<std::vector<RGB>> &image)
{
    for (auto &row : image)
    {
        for (auto &pixel : row)
        {
            unsigned char gray = (pixel.r + pixel.g + pixel.b) / 3;
            pixel.r = pixel.g = pixel.b = gray;
        }
    }
}

// Function to invert colors of the image
void invert(std::vector<std::vector<RGB>> &image)
{
    for (auto &row : image)
    {
        for (auto &pixel : row)
        {
            pixel.r = 255 - pixel.r;
            pixel.g = 255 - pixel.g;
            pixel.b = 255 - pixel.b;
        }
    }
}

// Function to adjust contrast
void contrast(std::vector<std::vector<RGB>> &image, float factor)
{
    for (auto &row : image)
    {
        for (auto &pixel : row)
        {
            pixel.r = std::min(255, std::max(0, static_cast<int>((pixel.r - 128) * factor + 128)));
            pixel.g = std::min(255, std::max(0, static_cast<int>((pixel.g - 128) * factor + 128)));
            pixel.b = std::min(255, std::max(0, static_cast<int>((pixel.b - 128) * factor + 128)));
        }
    }
}

// Function to apply box blur
void blur(std::vector<std::vector<RGB>> &image)
{
    std::vector<std::vector<RGB>> temp = image;
    int height = image.size();
    int width = image[0].size();

    for (int i = 1; i < height - 1; ++i)
    {
        for (int j = 1; j < width - 1; ++j)
        {
            int sum_r = 0, sum_g = 0, sum_b = 0;
            for (int x = -1; x <= 1; ++x)
            {
                for (int y = -1; y <= 1; ++y)
                {
                    sum_r += temp[i + x][j + y].r;
                    sum_g += temp[i + x][j + y].g;
                    sum_b += temp[i + x][j + y].b;
                }
            }
            image[i][j].r = sum_r / 9;
            image[i][j].g = sum_g / 9;
            image[i][j].b = sum_b / 9;
        }
    }
}

// Function to mirror the image horizontally
void mirror(std::vector<std::vector<RGB>> &image) {
    int height = image.size();
    int width = image[0].size();

    std::cout << "Applying mirroring using std::reverse...\n";

    // Print the first and last few pixels before mirroring for better visibility
    std::cout << "Before Mirroring (First and Last 5 pixels of first row):\n";
    for (int j = 0; j < std::min(5, width); ++j) {
        std::cout << "(" << (int)image[0][j].r << ", " 
                  << (int)image[0][j].g << ", " 
                  << (int)image[0][j].b << ") ";
    }
    std::cout << " ... ";
    for (int j = width - 5; j < width; ++j) {
        std::cout << "(" << (int)image[0][j].r << ", " 
                  << (int)image[0][j].g << ", " 
                  << (int)image[0][j].b << ") ";
    }
    std::cout << "\n";

    // Reverse each row to flip the image horizontally
    for (int i = 0; i < height; ++i) {
        std::reverse(image[i].begin(), image[i].end());
    }

    // Print the first and last few pixels after mirroring
    std::cout << "After Mirroring (First and Last 5 pixels of first row):\n";
    for (int j = 0; j < std::min(5, width); ++j) {
        std::cout << "(" << (int)image[0][j].r << ", " 
                  << (int)image[0][j].g << ", " 
                  << (int)image[0][j].b << ") ";
    }
    std::cout << " ... ";
    for (int j = width - 5; j < width; ++j) {
        std::cout << "(" << (int)image[0][j].r << ", " 
                  << (int)image[0][j].g << ", " 
                  << (int)image[0][j].b << ") ";
    }
    std::cout << "\n";
}


// Function to compress the image
void compress(std::vector<std::vector<RGB>> &image)
{
    int height = image.size();
    int width = image[0].size();

    // Number of kept rows/columns is roughly half
    // If the image height/width is odd, integer division will floor it,
    // leaving room for the "odd" indices to be kept.
    int new_height = height / 2;
    int new_width = width / 2;

    std::vector<std::vector<RGB>> compressed(new_height, std::vector<RGB>(new_width));

    // Skip even row/column indices => keep odd row/column indices
    for (int i = 0; i < new_height; ++i)
    {
        for (int j = 0; j < new_width; ++j)
        {
            // Use (2*i + 1, 2*j + 1) to grab odd row/column indices
            compressed[i][j] = image[2 * i + 1][2 * j + 1];
        }
    }

    std::cout << "After Compression: " << new_width << "x" << new_height << "\n";
    image = compressed;
}


// Main function
int main(int argc, char *argv[])
{
    if (argc < 3)
    {     
        std::cerr << "Usage: " << argv[0] << " <input.ppm> <output.ppm> [options]\n";   
  
        return 1;
    }


    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    std::vector<std::string> options;

    for (int i = 3; i < argc; ++i)
    {
        options.push_back(argv[i]);
    }

    try
    {
        auto image = readPPM(inputFile);

        for (const auto &option : options)
{
    if (option == "-g")
    {
        grayscale(image);
        std::cout << "After Grayscale:\n";
    }
    else if (option == "-i")
    {
        invert(image);
        std::cout << "After Inversion:\n";
    }
    else if (option == "-x")
    {
        contrast(image, 1.2f);
        std::cout << "After Contrast:\n";
    }
    else if (option == "-b")
    {
        blur(image);
        std::cout << "After Blur:\n";
    }
    else if (option == "-m")
    {
        std::cout << "Calling mirroring function...\n";
        mirror(image);
        //std::cout << "After Mirroring:\n";
    }
    else if (option == "-c")
    {
        compress(image);
        std::cout << "After Compression:\n";
    }
    else
    {
        std::cerr << "Unknown option: " << option << "\n";
        return 1;
    }

    // Print first few pixels after transformation
    for (int i = 0; i < std::min(5, (int)image.size()); ++i)
    {
        for (int j = 0; j < std::min(5, (int)image[i].size()); ++j)
        {
            std::cout << "(" << (int)image[i][j].r << ", "
                      << (int)image[i][j].g << ", "
                      << (int)image[i][j].b << ") ";
        }
        std::cout << "\n";
    }
}

        writePPM(outputFile, image);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}