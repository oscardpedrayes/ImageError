#include <stdio.h>
#include <errno.h>
#include <algorithm> 
#include<Math.h>

#include"Patch.h"
#include "Image.h"


Image::Image(const unsigned int w, const unsigned int h)
	: image_(nullptr)
	, width_(w)
	, height_(h)
	, size_(w*h)
{
	initializeImage();
}

Image::Image(const char* dir)
	: image_(nullptr)
	, width_(0)
	, height_(0)
	, size_(0)
{
	readImagePGM(dir);
}

Image::~Image()
{
	if (image_ != nullptr)
	{
		for (unsigned int i = 0; i < height_; ++i)
			delete[] image_[i];
		delete[] image_;
	}
}

void Image::initializeImage()
{
	if((width_ > 0) && (height_ > 0))
	{
		image_ = new double*[height_];
		for (unsigned int y = 0; y < height_; ++y)
		{
			image_[y] = new double[width_];
			for (unsigned int x = 0; x < width_; ++x)
			{
				image_[y][x] = 0;
			}
		}
	}
}

double** Image::getImage() const
{
	return image_;
}

unsigned int const Image::getWidth() const
{
	return width_;
}

unsigned int const Image::getHeigth() const
{
	return height_;
}

unsigned int const Image::getSize()
{
	return size_;
}

void Image::setValue(const double value,const unsigned int i, const unsigned int j)
{
	if(i < height_ && j < width_ )
		image_[i][j]= value;
}

void Image::addValue(const double value,const unsigned int i,const unsigned int j)
{
	if (i < height_ && j < width_)
		image_[i][j] += value;
}

void Image::addAllValue(const double value)
{
	for (unsigned int y = 0; y < height_; ++y)	
		for (unsigned int x = 0; x < width_; ++x)
		
			image_[y][x] += value;		
}

void Image::diffImages(Image &img2)
{
	for (unsigned int y = 0; y < height_; ++y)
		for (unsigned int x = 0; x < width_; ++x)
			image_[y][x] -= img2.image_[y][x];
}

void Image::readImagePGM(const char* dir)
{
	unsigned char *imgBuffer=nullptr;
	FILE *fich;
	errno_t err;

	// Opens the image file
	if ((err = fopen_s(&fich, dir, "rb")) != 0)
	{
		printf("ERROR: cannot open file '%s'\n", dir);
	}
	else
	{
		//Gets width and height
		if (fscanf_s(fich, "P5\n%u %u\n255\n", &width_, &height_) == 2)
		{
			size_ = width_ * height_;
			initializeImage();
			//Se reserva espacio para la imagen
			imgBuffer = new unsigned char[width_*height_];

			// Read from image
			fread(imgBuffer, size_, 1, fich);

			// Convert to double
			for (unsigned int i = 0; i < height_; i++)
			{
				for (unsigned int j = 0; j < width_; j++)
				{
					image_[i][j] = imgBuffer[i*width_ + j];
				}
			}	
		}
		else
		{
			printf("ERROR:Unknown image format\n");
		}
		fclose(fich);
	}
	if(imgBuffer!=nullptr)
		delete[] imgBuffer;
}

void Image::readImage(const char* dir)
{
	FILE *fich;
	errno_t err;

	// Opens the image file
	if ((err = fopen_s(&fich, dir, "rb")) != 0)
	{
		printf("ERROR: cannot open file '%s'\n", dir);

	}
	else
	{
		// Gets width and height
		if (fscanf_s(fich, "%u %u\n", &width_, &height_) == 2)
		{
			size_ = width_ * height_;
			initializeImage();
		
			// Reads image
			for (unsigned int i = 0; i < height_; i++)
			{
				fread(image_[i], sizeof(double), width_, fich);
			}
		}
		else
		{
			printf("ERROR:Unknown image format\n");
		}
		fclose(fich);
	}
}

void Image::exportImagePGM(const char* path)
{
	FILE *file = NULL;
	fopen_s(&file, path, "wb");
	if (file)
	{
		unsigned char byte;
		fprintf(file, "P5\n%u %u\n255\n", width_, height_);
		for (unsigned int y = 0; y < height_; ++y)
		{
			for (unsigned int x = 0; x < width_; ++x)
			{
				byte = (unsigned char)(clip(image_[y][x], 0, 255));
				fwrite(&byte, 1, 1, file);
			}

		}
		fclose(file);
	}
}

void Image::exportImage(const char* path)
{
	FILE *file = NULL;
	fopen_s(&file, path, "wb");
	if (file)
	{
		fprintf(file, "%u %u\n", width_, height_);
		for (unsigned int y = 0; y < height_; ++y)
		{
			fwrite(image_[y], 1, width_, file);
		}
		fclose(file);
	}
}

double Image::clip(const double v,const double min,const double max)
{
	if (v >= max)
		return max;
	if (v <= min)
		return min;
	return v;
}

const double Image::compare_ECM(const Image &img) const
{
	// Get max width and height possible by getting the smaller ones
	unsigned int minX = std::min(width_, img.width_);
	unsigned int minY = std::min(height_, img.height_);
	
	// Initialize average and deviation
	double avg = 0;
	double dev = 0;

	for (unsigned int y = 0; y < minY; ++y)
		for (unsigned int x = 0; x < minX; ++x)
		{
			double tmp = image_[y][x] - img.image_[y][x];
			dev += tmp * tmp;
			avg += tmp;
		}
		
	unsigned int size = minY * minX;
	avg /= size;
	dev = (dev / size) - avg*avg;
	if (dev < 0) dev = 0; // fix for square root (round error)
	return sqrt(dev);

}

const double Image::compare_PSNR(const Image &img) const
{
	const double ecm= compare_ECM(img);
	return (10 * log(255 * 255 / ecm)) / log(10);
}

const double Image::compare_SSIM(const Image &img) const
{
	// Structural Similarity (SSIM) Index
	// https://ece.uwaterloo.ca/~z70wang/publications.htm
	// Source: https://ece.uwaterloo.ca/~z70wang/research/ssim/
	unsigned int minX = std::min(width_, img.width_);
	unsigned int minY = std::min(height_, img.height_);

	// Default model parameters
	constexpr double K1 = 0.01;
	constexpr double K2 = 0.03;
	constexpr double L = 255;    // Dynamic range of the pixels
	constexpr unsigned int WINDOW_SIZE = 11;
	Patch<WINDOW_SIZE> gaussianWindow;
	gaussianWindow.initGaussian(1.5, true);

	// Main loop
	constexpr double C1 = (K1 * L) * (K1 * L);
	constexpr double C2 = (K2 * L) * (K2 * L);
	double sumSSIM = 0;
	minX -= WINDOW_SIZE;  // filter2(window, img1, 'valid')  =>  Avoid zero padding, so limit image to patch movement
	minY -= WINDOW_SIZE;  // filter2(window, img1, 'valid')  =>  Avoid zero padding, so limit image to patch movement
	for (unsigned int y = 0; y <= minY; ++y)
	{
		for (unsigned int x = 0; x <= minX; ++x)
		{
			double sumA = 0;
			double sumB = 0;
			double sumAA = 0;
			double sumAB = 0;
			double sumBB = 0;
			for (unsigned int i = 0; i < WINDOW_SIZE; ++i)
			{
				const double *rowA = image_[y + i];
				const double *rowB = img.image_[y + i];
				const double *rowGaussian = gaussianWindow.getRow(i);
				for (unsigned int j = 0; j < WINDOW_SIZE; ++j)
				{
					sumA += rowGaussian[j] * rowA[x + j];
					sumB += rowGaussian[j] * rowB[x + j];
					sumAA += rowGaussian[j] * rowA[x + j] * rowA[x + j];
					sumAB += rowGaussian[j] * rowA[x + j] * rowB[x + j];
					sumBB += rowGaussian[j] * rowB[x + j] * rowB[x + j];
				}
			}
			sumAA -= sumA * sumA;
			sumAB -= sumA * sumB;
			sumBB -= sumB * sumB;

			const double ssim = ((2 * sumA * sumB + C1) * (2 * sumAB + C2)) / ((sumA * sumA + sumB * sumB + C1) * (sumAA + sumBB + C2));
			sumSSIM += ssim;
		}
	}
	sumSSIM /= (minX + 1) * (minY + 1);
	return sumSSIM;

}