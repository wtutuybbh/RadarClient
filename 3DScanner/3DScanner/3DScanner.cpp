// 3DScanner.cpp : Defines the entry point for the console application.
//

#define STEPS 100

#include "stdafx.h"

void generate_test_input(unsigned char *data, int width, int height);
void get_distances(unsigned char *data, int width, int height, double a, float *distances);

float d = 10.0f;
float w = 4.0f;
float h = 3.0f;
float r = 1.0f;




float a0 = acos(((- r)*((r / 2 - w / 2) - r)) / (sqrt(r * r)*sqrt(((r / 2 - w / 2) - r) * ((r / 2 - w / 2) - r) + (h / 2) * (h / 2) + (d) * (d))));

int main()
{
	std::cout << "d = " << d << "; w = " << w << "; h = " << h << "; r = " << r << "; a0 = " << a0 << std::endl;

	int width = 800;
	int height = 600;

	unsigned char *data = new unsigned char[width * height]();

	double tmp;
	const clock_t begin_time = clock();

	generate_test_input(data, width, height);

	double a = 0.1;

	float *distances = new float[height * 3]();

	int n_frames = 10000;

	for (auto i=0; i<= n_frames; i++)
	{
		get_distances(data, width, height, a, distances);
	}

	std::cout << "speed = " << n_frames / (float(clock() - begin_time / CLOCKS_PER_SEC) / 1000.0) << " frames per second" << std::endl;
}

void generate_test_input(unsigned char *data, int width, int height)
{
	for (auto x = 0; x < width; x++)
	{
		for (auto y=0; y<height; y++)
		{
			if (x == 500)
			{
				data[y * width + x] = UCHAR_MAX;
			}
			else
			{
				data[y * width + x] = 0;
			}
		}
	}
}

void get_distances(unsigned char *data, int width, int height, double a, float *distances)
{
	double a0a = a0 + a;
	double ctg = cos(a0a) / sin(a0a);
	int i = 0;
	for (auto y = 0; y < height; y ++)
	{
		for (auto x = 0; x < width; x ++)
		{
			if (data[y*width + x] >= 255)
			{
				float S_x = w * x / width + r/2.0f - w/2.0f;
				float z = r / (S_x / d + ctg);
				distances[y + i + 2] = z;
				distances[y + i + 1] = z * (h * y / height - h / 2.0f) / d;
				distances[y + i] = z * S_x / d;
			}
		}
	}
}

