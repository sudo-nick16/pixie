#include <stdio.h> 
#include <stdbool.h> 
#include <time.h> 
#include <unistd.h> 

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define RED 0
#define GREEN 1
#define BLUE 2

typedef struct {
	unsigned char red; 
	unsigned char green; 
	unsigned char blue; 
} Pixel;

void print_pixel(Pixel *p) {
	printf("Pixel {R: %d, G: %d, B: %d}\n", p->red, p->green, p->blue);
}

typedef struct {
	Pixel **pixels;
	int width;
	int height;
	int channels;
} Image;

void print_image(Image *img, bool px) {
	printf("Image {Width: %d, Height: %d, Channels: %d}\n", img->width, img->height, img->channels);
	if (px) {
		for (int i = 0; i < img->height; ++i) {
			for (int j = 0; j < img->width; ++j) {
				print_pixel(&img->pixels[i][j]);
			}
		}
	}
}

char *generate_name() {
	char *new_filename = (char*) malloc(20);
	int t = time(NULL);
	sprintf(new_filename, "%d.png", t);
	return  new_filename;
}

unsigned char *image_to_unsigned_char(Image *img) {
	unsigned char *buf = (unsigned char*) malloc(img->width * img->height * img->channels);
	int idx = 0;
	for (int i = 0; i < img->height; ++i) {
		for (int j = 0; j < img->width; ++j) {
			buf[idx++] = img->pixels[i][j].red;
			buf[idx++] = img->pixels[i][j].green;
			buf[idx++] = img->pixels[i][j].blue;
		}
	}
	return buf;
}

Pixel **alloc_pixel_matrix(int width, int height) {
	Pixel **p = (Pixel**) malloc(sizeof(Pixel*) * height);
	for (int i = 0; i < height; ++i) {
		p[i] = (Pixel*) malloc(sizeof(Pixel) * width);
	}
	return p;
}

void init_pixel_matrix(Image *img, unsigned char *data) {
	for (int i = 0; i < img->height; ++i) {
		for (int j = 0; j < img->width; ++j) {
			int offset = i * img->width * img->channels + j * img->channels;
			Pixel p = {
				.red = data[offset],
				.green = data[offset + 1],
				.blue = data[offset + 2]
			};
			img->pixels[i][j] = p;
		}
	}
}

void init_img(Image *img, unsigned char *data, int width, int height, int channels) {
	img->channels = channels;
	img->width = width;
	img->height = height;
	img->pixels = alloc_pixel_matrix(width, height);
	init_pixel_matrix(img, data);
}

int write_png(Image *img) {
	char *new_filename = generate_name();
	int ret = stbi_write_png(new_filename, img->width, img->height, img->channels, image_to_unsigned_char(img), img->width * img->channels);
	free(new_filename);
	if (ret < 1) {
		return 0;
	}
	return 1;
}

Image *downscale_image(Image *img, int px_level) {
	Image *px_img = malloc(sizeof(Image));
	px_img->width = img->width / px_level;
	px_img->height = img->height / px_level;
	px_img->channels = img->channels;
	px_img->pixels = alloc_pixel_matrix(px_img->width, px_img->height);

	int px_sq = px_level * px_level;
	int r = 0, c = 0;
	for (int i = 0; i < img->height - px_level; i += px_level)  {
		for (int j = 0; j < img->width - px_level; j += px_level) {
			int ar = 0, ag = 0, ab = 0;
			for (int ii = i; ii < i + px_level; ++ii) {
				for (int jj = j; jj < j + px_level; ++jj) {
					if (ii < img->height && jj < img->width) {
						ar += img->pixels[ii][jj].red;
						ag += img->pixels[ii][jj].green;
						ab += img->pixels[ii][jj].blue;
					}
				}
			}
			ar /= px_sq;
			ag /= px_sq;
			ab /= px_sq;
			Pixel p = {
				.red = ar,
				.green = ag,
				.blue = ab
			};
			px_img->pixels[r][c] = p;
			c++;
		}
		c = 0;
		r++;
	}
	return px_img;
}

Image *pixelate_img(Image *img, int px_level) {
	Image *px_img = malloc(sizeof(Image));
	px_img->width = img->width;
	px_img->height = img->height;
	px_img->channels = img->channels;
	px_img->pixels = alloc_pixel_matrix(px_img->width, px_img->height);
	int px_sq = px_level * px_level;

	for (int i = 0; i < img->height - px_level; i += px_level)  {
		for (int j = 0; j < img->width - px_level; j += px_level) {
			int ar = 0, ag = 0, ab = 0;
			for (int ii = i; ii < i + px_level; ++ii) {
				for (int jj = j; jj < j + px_level; ++jj) {
					if (ii < img->height && jj < img->width) {
						ar += img->pixels[ii][jj].red;
						ag += img->pixels[ii][jj].green;
						ab += img->pixels[ii][jj].blue;
					}
				}
			}
			ar /= px_sq;
			ag /= px_sq;
			ab /= px_sq;

			Pixel p = {
				.red = ar,
				.green = ag,
				.blue = ab
			};
			for (int ii = i; ii < i + px_level; ++ii) {
				for (int jj = j; jj < j + px_level; ++jj) {
					px_img->pixels[ii][jj] = p;
				}
			}
		}
	}
	return px_img;
}

void free_image(Image *img) {
	free(img->pixels);
	free(img);
}

void usage() {
	printf("Usage: ./pixie <path-to-image> [OPTIONS]\n");
	printf("OPTIONS:\n");
	printf("     -l: pixelation level (default 10)\n");
	printf("     -d: downscale image by l factor\n");
	printf("     -p: pixelate image by l factor\n");
}

int main (int argc, char *argv[]) {
	if (argc < 2) {
		usage();
		return 1;
	}
	int px_level = 10;
	int opt = getopt(argc, argv, "l:pd");
	if (opt == 'l') {
		px_level = atoi(optarg);
	}

	char *filename = argv[1];
	int width,height,channels;
	unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);
	if (data == NULL) {
		printf("ERROR: Could not load image.\n");
		return 1;
	}
	printf("INFO: Width: %d, Height: %d, Channels: %d\n", width, height, channels);

	int img_size = width * height * 3;

	unsigned char *image = (unsigned char*) malloc(img_size);
	memset(image, '\0', img_size + 1);

	Image *img = (Image *) malloc(sizeof(Image));
	init_img(img, data, width, height, channels);

	Image *new_img = NULL;
	if ((opt = getopt(argc, argv, "l:pd")) != -1) {
		if (opt == 'd') {
			new_img = downscale_image(img, px_level);
		} else if (opt == 'p') {
			new_img = pixelate_img(img, px_level);
		}
	} else {
		new_img = pixelate_img(img, px_level);
	}

	if (new_img == NULL || !write_png(new_img)) {
		printf("ERROR: Could not generate pixelated image.");
		return 1;
	}
	printf("INFO: Pixelated image generated!.");

	stbi_image_free(data);
	free(image);
	free_image(img);
	free_image(new_img);
	return 0;
}
