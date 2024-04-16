// #include "qdbmp.hpp"

// int main(int argc, char* argv[]) {
//   // TODO: write your sequential version of the blur program in this file.
//   // You can mimic negative.cpp to get started.
// }

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include "qdbmp.hpp"

using std::cerr;
using std::endl;
using std::string;

constexpr UCHAR MAX_COLOR_VALUE = 255U;

/**
 * This program takes a .bmp image and generates its corresponding "negative"
 * image to a new .bmp file on disk.
 */
int main(int argc, char* argv[]) {
  // Check input commands
  if (argc != 4) {
    cerr << "Usage: " << argv[0] << " <input file> <output file> <block_size>"
         << endl;
    return EXIT_FAILURE;
  }

  string input_fname{argv[1]};
  string output_fname{argv[2]};
  int block_size{std::stoi(argv[3])};  // std::stoi converts string to int

  // Construct a BitMap object using the input file specified
  BitMap image(input_fname);

  // Check the command above succeed
  if (image.check_error() != BMP_OK) {
    perror("ERROR: Failed to open BMP file.");
    return EXIT_FAILURE;
  }

  // Create a new BitMap for output the negative image
  const unsigned int height = image.height();
  const unsigned int width = image.width();
  BitMap blurred(width, height);

  // Check the command above succeed
  if (blurred.check_error() != BMP_OK) {
    perror("ERROR: Failed to open BMP file.");
    return EXIT_FAILURE;
  }

  // setup timer
  auto start = std::chrono::high_resolution_clock::now();

  // Loop through each pixel and turn into blurred
  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      int cnt = 0;                       // reset the count of pixels
      int red = 0, green = 0, blue = 0;  // reset the sum of RGB values

      // Loop through each pixel in the block
      for (int i = -block_size; i <= block_size; i++) {
        for (int j = -block_size; j <= block_size; j++) {
          // Check if the pixel is within the image
          if (x + i >= 0 && x + i < width && y + j >= 0 && y + j < height) {
            // Read the current pixel RGB color
            RGB color = image.get_pixel(x + i, y + j);
            red += color.red;
            green += color.green;
            blue += color.blue;
            cnt++;
          }
        }
      }

      // Calculate the blurred RGB color
      RGB blur_color{
          static_cast<UCHAR>(red / cnt),
          static_cast<UCHAR>(green / cnt),
          static_cast<UCHAR>(blue / cnt),
      };

      // // Read the current pixel RGB color
      // RGB color = image.get_pixel(x, y);

      // // Calculate the blurred RGB color
      // RGB blur_color{
      //     static_cast<UCHAR>(MAX_COLOR_VALUE - color.red),
      //     static_cast<UCHAR>(MAX_COLOR_VALUE - color.green),
      //     static_cast<UCHAR>(MAX_COLOR_VALUE - color.blue),
      // };

      // Set the blurred color
      blurred.set_pixel(x, y, blur_color);
    }
  }

  // end timer
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;

  std::cout << "Elapsed time: " << elapsed.count() << " s\n";

  // Output the blurred image to disk
  blurred.write_file(output_fname);

  if (image.check_error() != BMP_OK) {
    perror("ERROR: Failed to open BMP file.");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}