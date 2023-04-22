#pragma once

#include <stdint.h>
#include <stddef.h>

enum EncodeFormat
{
  PNG
};

struct ImageInfo
{
  int artboardID;
  int quality; // image quality, 100 is the best.
  EncodeFormat format;
};

struct Image
{
  size_t size;
  uint8_t* data;
};

extern "C"
{
  int init(int width, int height);

  void shutdown();

  /**
   * @brief Load sketch file
   *
   * @param filename
   * local sketch file path
   * @return int
   * non-zero indicates errors occurs
   */
  int loadSketchFile(const char* filename);

  int loadContent(const char* content);

  int loadFile(const char* filename);


  /**
   * @brief Render the input file as images by the given infos array
   *
   * @param width
   * image width
   * @param height
   * image height
   * @param infos
   * Array for the description of image
   * @param count
   * The count of info array
   * @param out
   * Output image data
   * @return int
   *
   * Non-zero indicates error occurs. If return -2, the data pointed by \a out must be release by
   * \sa releaseImage
   */
  int renderAsImages(int width, int height, const ImageInfo* infos, int count, Image* out);

  /**
   * @brief Release the memory allocated by \sa renderAsImages
   *
   * @param image
   * Array of \sa Image output by renderAsImages
   * @param count
   * The number of array of \a image
   */
  void releaseImage(Image* image, int count);
}
