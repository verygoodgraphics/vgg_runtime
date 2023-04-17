#pragma once

#include <stdint.h>
#include <stddef.h>

enum EncodeFormat{
	PNG
};

struct ImageInfo{
	int width;
	int height;
	int quality;
	int format;
};

struct Image{
	size_t size;
	uint8_t* data;
};

extern "C"{

int init(int width, int height);

int shutdown();

int loadSketchFile(const char * filename);

int loadDocument(const char * doc);

int renderAsImage(int artboardID, const ImageInfo * info,  Image* out);

void releaseImage(Image * image);

}