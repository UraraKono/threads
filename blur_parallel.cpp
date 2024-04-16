#include <pthread.h>

// #include "./qdbmp.hpp"

// int main(int argc, char* argv[]) {
//  // TODO: write your parallel version of the blur program in this file.
//  // You should write your sequential version first
// }

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "qdbmp.hpp"

using std::cerr;
using std::endl;
using std::string;
using std::vector;

constexpr UCHAR MAX_COLOR_VALUE = 255U;

/**
 * This program takes a .bmp image and generates its corresponding "negative"
 * image to a new .bmp file on disk.
 */

// function to return the x and y given the index
vector<int> get_x_y(int index, int width) {
  vector<int> x_y(2);
  x_y.at(0) = static_cast<int>(index % width);
  x_y.at(1) = static_cast<int>(index / width);
  return x_y;
}

struct ThreadData {
  int start_index;
  int end_index;
  BitMap* image;
  BitMap* blurred;
  int block_size;
};

void* blur_thread(void* arg) {
  ThreadData* data = (ThreadData*)arg;
  BitMap* image = data->image;
  BitMap* blurred = data->blurred;
  int block_size = data->block_size;

  for (int index = data->start_index; index < data->end_index; index++) {
    vector<int> x_y = get_x_y(index, image->width());
    int x = x_y.at(0);
    int y = x_y.at(1);
    int cnt = 0;                       // reset the count of pixels
    int red = 0, green = 0, blue = 0;  // reset the sum of RGB values

    // Loop through each pixel in the block
    for (int i = -block_size; i <= block_size; i++) {
      for (int j = -block_size; j <= block_size; j++) {
        // Check if the pixel is within the image
        if (x + i >= 0 && x + i < static_cast<int>(image->width()) && y + j >= 0 &&
            y + j < static_cast<int>(image->height())) {
          // Read the current pixel RGB color
          RGB color = image->get_pixel(x + i, y + j);
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

    // Set the blurred color
    blurred->set_pixel(x, y, blur_color);
  }

  return NULL;
}

int main(int argc, char* argv[]) {
  // Check input commands
  if (argc != 5) {
    cerr << "Usage: " << argv[0]
         << " <input file> <output file> <block_size> <thread_count>" << endl;
    return EXIT_FAILURE;
  }

  string input_fname{argv[1]};
  string output_fname{argv[2]};
  int block_size{std::stoi(argv[3])};    // std::stoi converts string to int
  int thread_count{std::stoi(argv[4])};  // std::stoi converts string to int

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

  int image_size = height * width;
  vector<int> start_indices(thread_count);
  vector<int> end_indices(thread_count);
  for (int i = 0; i < thread_count; i++) {
    start_indices.at(i) = static_cast<int>(i * image_size / thread_count);
  }
  for (int i = 0; i < thread_count - 1; i++) {
    end_indices.at(i) = static_cast<int>((i + 1) * image_size / thread_count);
  }
  end_indices.at(thread_count - 1) = image_size;

  // Create threads
  // pthread_t blur_threads[thread_count];
  // ThreadData thread_data[thread_count];
  pthread_t* blur_threads = new pthread_t[thread_count];
  ThreadData* thread_data = new ThreadData[thread_count];

  for (int i = 0; i < thread_count; i++) {
    thread_data[i].start_index =
        static_cast<int>(i * image_size / thread_count);
    thread_data[i].end_index =
        static_cast<int>((i + 1) * image_size / thread_count);
    if (i == thread_count - 1) {
      thread_data[i].end_index = image_size;
    }
    thread_data[i].image = &image;
    thread_data[i].blurred = &blurred;
    thread_data[i].block_size = block_size;

    pthread_create(&blur_threads[i], NULL, blur_thread, &thread_data[i]);
  }

  // Join threads
  for (int i = 0; i < thread_count; i++) {
    pthread_join(blur_threads[i], NULL);
  }
  delete[] blur_threads;
  delete[] thread_data;

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