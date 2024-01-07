#include <float.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void usage() {
	printf("Usage: ./art <path-to-image> <colors>\n");
}

float rand_float() {
	return rand()/(float)RAND_MAX;
}

typedef struct {
	float x;
	float y;
	float z;
	int cluster;
} Point;

typedef struct {
	Point *points;
	int size;
} Points;

void print_point(Point p){
	printf("r: %f, g: %f, b: %f\n", p.x, p.y, p.z);
}

float distance(Point p1, Point p2) {
	return sqrt(pow((p2.x - p1.x), 2) + pow((p2.y - p1.y), 2) + pow((p2.z - p1.z), 2));
}

void update_points(Points *Pixels, Points *Centroids) {
	for (int i = 0; i < Pixels->size; ++i) {
		Point *p = &Pixels->points[i];
		float closest = FLT_MAX;
		int closest_ind = 0;
		for (int c = 0; c < Centroids->size; ++c) {
			float d = distance(*p, Centroids->points[c]);
			if (d < closest) {
				closest = d;
				closest_ind = c;
			}
		}
		p->cluster = closest_ind;
	};
}

void adjust_centroids(Points *Pixels, Points *Centroids) {
	Point mean[Centroids->size];
	int mean_n[Centroids->size];
	for (int i = 0; i < Centroids->size; ++i) {
		mean[i].x = 0;
		mean[i].y = 0;
		mean[i].z = 0;
		mean_n[i] = 0;
	}
	for (int i = 0; i < Pixels->size; ++i) {
		Point p = Pixels->points[i];
		mean_n[p.cluster] += 1;
		mean[p.cluster].x += p.x;
		mean[p.cluster].y += p.y;
		mean[p.cluster].z += p.z;
	}
	for (int i = 0; i < Centroids->size; ++i) {
		if (mean_n[i] == 0) {
			continue;
		}
		Centroids->points[i].x = mean[i].x/mean_n[i];
		Centroids->points[i].y = mean[i].y/mean_n[i];
		Centroids->points[i].z = mean[i].z/mean_n[i];
	}
}

int main(int argc, char **argv) {
	srand(time(NULL));
	if (argc < 3) {
		usage();
		return 1;
	}
	char *filename = argv[1];
	int n_colors = atoi(argv[2]);

	printf("Palette colors: %d\n", n_colors);

	int width, height, channels;
	unsigned char *image_data = stbi_load(filename, &width, &height, &channels, 3);
	if (image_data == NULL) {
		printf("[ERROR] Could not load image.\n");
		return 1;
	}
	printf("[INFO] Width: %d, Height: %d, Channels: %d\n", width, height, channels);

	Points Centroids = {0};
	Centroids.size = n_colors;
	Centroids.points = (Point*) malloc(n_colors * sizeof(Point));

	for (int i = 0; i < n_colors; ++i) {
		Point p = {
			.x = rand_float() * 255,
			.y = rand_float() * 255,
			.z = rand_float() * 255
		};
		Centroids.points[i] = p;
	};


	Points Pixels = {0};
	Pixels.size = width * height;
	Pixels.points =  (Point*) malloc(width * height * sizeof(Point));

	int ind = 0;
	for (int i = 0; i < width * height * channels; i += channels) {
		Point p = {
			.x = image_data[i],
			.y = image_data[i + 1],
			.z = image_data[i + 2],
		};
		float closest = FLT_MAX;
		int closest_ind = 0;
		for (int c = 0; c < Centroids.size; ++c) {
			float d = distance(p, Centroids.points[c]);
			if (d < closest) {
				closest = d;
				closest_ind = c;
			}
		}
		p.cluster = closest_ind;
		Pixels.points[ind++] = p;
	};

	int iterations = 30;

	while (iterations--) {
		adjust_centroids(&Pixels, &Centroids);
		update_points(&Pixels, &Centroids);
	}

	for (int i = 0; i < Centroids.size; ++i) {
		print_point(Centroids.points[i]);
	}

	for(int i = 0; i < Pixels.size; ++i) {
		Point *p = &Pixels.points[i];
		Point c = Centroids.points[p->cluster]; 
		p->x = c.x;
		p->y = c.y;
		p->z = c.z;
	}

	unsigned char *final_img = malloc(width * height * channels);
	ind = 0;
	for (int i = 0; i < width * height * channels; i += channels) {
		final_img[i] = Pixels.points[ind].x;
		final_img[i + 1] = Pixels.points[ind].y;
		final_img[i + 2] = Pixels.points[ind].z;
		// print_point(Pixels.points[ind]);
		ind++;
	}

	char *output = malloc(10);
	sprintf(output, "%zu.png", time(NULL));
	int ret = stbi_write_png(output, width, height, channels, final_img, width * channels);
	if (ret < 1) {
		return 0;
	}
	return 0;
}
