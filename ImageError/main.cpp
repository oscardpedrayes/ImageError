// main.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Image.h"

#pragma warning (disable : 4996)

// Compare images
const double compare(const Image &img1,const Image &img2, const unsigned int method ) 
{
	double errorValue = -1;
	switch (method)
	{
	case 1:
		errorValue = img1.compare_ECM(img2);
		break;
	case 2:
		errorValue = img1.compare_PSNR(img2);
		break;
	case 3:
		errorValue = img1.compare_SSIM(img2);
		break;
	default:
		printf("That comparison method is not implemented.\n ");
	}
	return errorValue; // if -1 an error has ocurred
}

const char* outputString(const unsigned int method)
{
	switch (method)
	{
	case 1:
		return "ECM";
	case 2:
		return "PSNR (dB)";
	case 3:
		return "SSIM";
	}
}

void compareDirectory(const char* path1, const char* path2, const unsigned int method)
{
	char tmp1[2048];
	char tmp2[2048];
	sprintf(tmp1, "%s/*.pgm", path1);
	printf("Searching .pgm in %s\n", path1);
	struct _finddata_t pgm_file;
	intptr_t hFile;

	double meanError = 0;
	unsigned int count = 1;

	// Find first .pgm in the path
	if ((hFile = _findfirst(tmp1, &pgm_file)) == -1L)
		printf("No *.pgm files in current directory!\n");
	else
	{
		do
		{
			sprintf(tmp1, "%s/%s", path1, pgm_file.name);
			sprintf(tmp2, "%s/%s", path2, pgm_file.name);
			printf("[%d] Comparing images:\n  * %s\n  * %s\n", count, tmp1, tmp2);

			// Loads both low-resolution and high-resolution images
			Image img1(tmp1);
			Image img2(tmp2);
			// Compare images
			
				const double errValue= compare(img1,img2,method);
				if (errValue == -1)
				{
					printf("An error has ocurred");
					return;
				}
				meanError += errValue;
			++count;
		} while (_findnext(hFile, &pgm_file) == 0);
		_findclose(hFile);
		meanError /= count;
		printf("Mean %s error of the directory = %.6f ",outputString(method),meanError);
	}
}

int main(int argc, char*argv[])
{
	const char *path1 = NULL;
	const char *path2 = NULL;
	bool directory = false;

	unsigned int method = 0;

	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-d") == 0)
		{
			directory = true;
		}
		else if (strcmp(argv[i], "-ecm") == 0)
		{
			method = 1;
		}
		else if (strcmp(argv[i], "-psnr") == 0)
		{
			method = 2;
		}
		else if (strcmp(argv[i], "-ssim") == 0)
		{
			method = 3;
		}
		else if (path1==NULL)
		{
			path1 = argv[i];
		}
		else if (path2==NULL)
		{
			path2 = argv[i];
		}
		else
		{
			printf("Correct usage: ImageError [-d] [-ecm/-psnr/-ssim] path1 path2\n");
			return -1;
		}
	}

	printf("-- Image Error --\n");
	if (directory)
		compareDirectory(path1,path2,method);
	else
	{
		// Load both images
		Image img1(path1);
		Image img2(path2);

		// Compare images
	
		const double errValue = compare(img1, img2, method);
		if (errValue == -1)
		{
			printf("An error has ocurred");
			return -1;
		}
		printf("%s error =%.6f\n ",outputString(method), errValue);
	}

}