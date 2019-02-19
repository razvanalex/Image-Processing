//------------------------------------------------------------------------
//   image_processing.c
//   Smadu Razvan-Alexandru  315CB
//------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "bmp_header.h"

#define MIN_PX_VAL 		0
#define MAX_PX_VAL 		255
#define EXTENTION_LENGTH 	3
#define FILTERS_NUM 		3
#define DEFAULT_TMP_MEMORY	512
#define SIZEOF_CHUNK 		7
#define ERROR			-1
#define MAX_IMG_HEIGHT		2500

typedef struct bmp_fileheader T_bmp_fileheader;
typedef struct bmp_infoheader T_bmp_infoheader;

typedef struct {
	unsigned char b, g, r;
} T_pixel;

typedef struct {
	short X, Y;
} T_point;

// Count characters of current line
int sizeof_line(FILE *file)
{
	int size = 0;
	char c = fgetc(file);

	while (c != '\n' && c != EOF)
	{
		size++;
		c = fgetc(file);
	}
	return ++size;
}

// Open a file for read. It returns -1 for errors, 
// and 0 for success
int open_file_read(FILE **input, char *name, char mode)
{
	if (mode == 't')
		*input = fopen(name, "rt");
	else if (mode == 'b')
		*input = fopen(name, "rb");
	else
	{
		fprintf(stderr, "Invalid mode!");
		return ERROR;
	}

	if (*input == NULL)
	{
		fprintf(stderr, "File not found!");
		return ERROR;
	}

	return 0;
}

// Concatenates the prefix to the image_filename
char* generate_new_name(char *image_filename, char *perfix)
{
	int length = strlen(perfix) + strlen(image_filename) -
		EXTENTION_LENGTH;
	char *img_name = malloc(length * sizeof(char));
	if (img_name == NULL)
		return NULL;

	strcpy(img_name, image_filename);
	char *new_image_filename = strstr(img_name, ".bmp");
	strcpy(new_image_filename, perfix);
	img_name[strlen(img_name)] = '\0';

	return img_name;
}

// Alloc memory from HEAP
T_pixel** Initialize_Image(int width, int height)
{
	int i, j;
	T_pixel **image = (T_pixel**)malloc(height * sizeof(T_pixel*));

	if (image == NULL)
		return NULL;

	for (i = 0; i < height; i++)
	{
		image[i] = (T_pixel*)calloc(width, sizeof(T_pixel));
		if (image[i] == NULL)
		{
			for (j = 0; j < i; j++)
				free(image[j]);
			free(image);
			return NULL;
		}
	}

	return image;
}

// Dealloc memory from HEAP
void Free_Image(T_pixel ***image, int height)
{
	int i;
	for (i = 0; i < height; i++)
	{
		free((*image)[i]);
	}
	free(*image);
	*image = NULL;
}

// Copy the image matrix to a new block of memory, on HEAP
// It returns -1 for allocation errors
int Copy_Image(T_pixel ***source, T_pixel **target,
	int width, int height)
{
	int i, j;

	if (*source != NULL)
		Free_Image(source, height);

	*source = Initialize_Image(width, height);
	if (*source == NULL)
		return ERROR;

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			(*source)[i][j].r = target[i][j].r;
			(*source)[i][j].g = target[i][j].g;
			(*source)[i][j].b = target[i][j].b;
		}
	}
	return 0;
}

// Free memory from HEAP. %c is for char* and %b is for T_pixel**.
// Pass all variables by reference, whitout Data_Types.
// It returns number of pointers succesfully released.
int Free_All_Memory(const char *Data_Types, ...)
{
	va_list vars;
	int i, count = 0, _tmp_i;
	char type;
	T_pixel ***_tmp_b;
	char **_tmp_c;

	va_start(vars, Data_Types);
	int size_vars = strlen(Data_Types);
	for (i = 0; i < size_vars; i++)
	{
		if (Data_Types[i] == '%')
		{
			type = Data_Types[++i];

			if (type == 'c')
			{
				_tmp_c = va_arg(vars, char**);
				if (_tmp_c != NULL)
				{
					free(*_tmp_c);
					*_tmp_c = NULL;
					count++;
				}
			}
			if (type == 'b')
			{
				_tmp_b = va_arg(vars, T_pixel***);
				_tmp_i = va_arg(vars, int);
				if (*_tmp_b != NULL && 
					_tmp_i <= MAX_IMG_HEIGHT)
				{
					Free_Image(_tmp_b, _tmp_i);
					count++;
				}
			}
		}
	}
	va_end(vars);

	return count;
}

// Check if the pixels are the same
int check_colors(const T_pixel *pixel1, const T_pixel *pixel2)
{
	if (pixel1->r == pixel2->r &&
		pixel1->g == pixel2->g &&
		pixel1->b == pixel2->b)
	{
		return 1;
	}

	return 0;
}

// This returns the value of padding
int calculate_padding(T_bmp_fileheader bmp_fh, T_bmp_infoheader bmp_ih)
{
	int line_length = (bmp_fh.bfSize - bmp_fh.imageDataOffset) /
		bmp_ih.height;
	int data_length_size = bmp_ih.width * sizeof(T_pixel);

	return line_length - data_length_size;
}

// Read bmp file header from a file
void read_bmp_fh(FILE *file, T_bmp_fileheader *bmp_fh)
{
	fread(&bmp_fh->fileMarker1, sizeof(unsigned char), 1, file);
	fread(&bmp_fh->fileMarker2, sizeof(unsigned char), 1, file);
	fread(&bmp_fh->bfSize, sizeof(unsigned int), 1, file);
	fread(&bmp_fh->unused1, sizeof(unsigned short), 1, file);
	fread(&bmp_fh->unused2, sizeof(unsigned short), 1, file);
	fread(&bmp_fh->imageDataOffset, sizeof(unsigned int), 1, file);
}

// Read bmp info header from a file
void read_bmp_ih(FILE *file, T_bmp_infoheader *bmp_ih)
{
	fread(&bmp_ih->biSize, sizeof(unsigned int), 1, file);
	fread(&bmp_ih->width, sizeof(signed int), 1, file);
	fread(&bmp_ih->height, sizeof(signed int), 1, file);
	fread(&bmp_ih->planes, sizeof(unsigned short), 1, file);
	fread(&bmp_ih->bitPix, sizeof(unsigned short), 1, file);
	fread(&bmp_ih->biCompression, sizeof(unsigned int), 1, file);
	fread(&bmp_ih->biSizeImage, sizeof(unsigned int), 1, file);
	fread(&bmp_ih->biXPelsPerMeter, sizeof(int), 1, file);
	fread(&bmp_ih->biYPelsPerMeter, sizeof(int), 1, file);
	fread(&bmp_ih->biClrUsed, sizeof(unsigned int), 1, file);
	fread(&bmp_ih->biClrImportant, sizeof(unsigned int), 1, file);
}

// Read bmp's pixel array
// Return -1 for allocation errors
int read_bit_map(FILE *file, T_pixel ***image, int padding,
	int width, int height)
{
	int i, j;

	*image = Initialize_Image(width, height);
	if (*image == NULL)
	{
		fprintf(stderr, "Memory allocation failed!");
		return ERROR;
	}

	for (i = height - 1; i >= 0; i--)
	{
		for (j = 0; j < width; j++)
		{
			fread(&(*image)[i][j].b, sizeof(unsigned char), 1,
				file);
			fread(&(*image)[i][j].g, sizeof(unsigned char), 1,
				file);
			fread(&(*image)[i][j].r, sizeof(unsigned char), 1,
				file);
		}

		//Exclude padding from reading
		fseek(file, padding, SEEK_CUR);
	}
	return 0;
}

// Read block of bytes between info header and pixel array
// Return -1 for allocation errors
int read_gap_1(char **gap, int size, FILE *file)
{
	*gap = malloc(size * sizeof(char));
	if (*gap == NULL)
		return ERROR;

	fread(*gap, sizeof(char), size, file);

	return 0;
}

// Generate a BMP file
void Create_BMP_file(T_bmp_fileheader bmp_fh, T_bmp_infoheader bmp_ih,
	char **gap, T_pixel **image, FILE *file)
{
	int i, j;
	int padding = calculate_padding(bmp_fh, bmp_ih);

	//Append File Header
	fwrite(&bmp_fh.fileMarker1, sizeof(unsigned char), 1, file);
	fwrite(&bmp_fh.fileMarker2, sizeof(unsigned char), 1, file);
	fwrite(&bmp_fh.bfSize, sizeof(unsigned int), 1, file);
	fwrite(&bmp_fh.unused1, sizeof(unsigned short), 1, file);
	fwrite(&bmp_fh.unused2, sizeof(unsigned short), 1, file);
	fwrite(&bmp_fh.imageDataOffset, sizeof(unsigned int), 1, file);

	//Append Info Header
	fwrite(&bmp_ih.biSize, sizeof(unsigned int), 1, file);
	fwrite(&bmp_ih.width, sizeof(signed int), 1, file);
	fwrite(&bmp_ih.height, sizeof(signed int), 1, file);
	fwrite(&bmp_ih.planes, sizeof(unsigned short), 1, file);
	fwrite(&bmp_ih.bitPix, sizeof(unsigned short), 1, file);
	fwrite(&bmp_ih.biCompression, sizeof(unsigned int), 1, file);
	fwrite(&bmp_ih.biSizeImage, sizeof(unsigned int), 1, file);
	fwrite(&bmp_ih.biXPelsPerMeter, sizeof(int), 1, file);
	fwrite(&bmp_ih.biYPelsPerMeter, sizeof(int), 1, file);
	fwrite(&bmp_ih.biClrUsed, sizeof(unsigned int), 1, file);
	fwrite(&bmp_ih.biClrImportant, sizeof(unsigned int), 1, file);

	// Append gap
	fwrite(*gap, sizeof(char), bmp_fh.imageDataOffset - ftell(file), file);

	// Append Matrix 
	for (i = bmp_ih.height - 1; i >= 0; i--)
	{
		for (j = 0; j < bmp_ih.width; j++)
		{
			fwrite(&image[i][j].b, sizeof(unsigned char), 1, file);
			fwrite(&image[i][j].g, sizeof(unsigned char), 1, file);
			fwrite(&image[i][j].r, sizeof(unsigned char), 1, file);
		}
		for (j = 0; j < padding; j++)
			fwrite("0", sizeof(unsigned char), 1, file);
	}

}

// Apply black-white effect to the image
void make_black_white(T_pixel **image, int width, int height)
{
	int i, j;
	T_pixel image_pixel;

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			image_pixel.r = (image[i][j].r +
				image[i][j].g +
				image[i][j].b) / 3;
			image_pixel.g = image_pixel.r;
			image_pixel.b = image_pixel.r;

			image[i][j].r = image_pixel.r;
			image[i][j].g = image_pixel.g;
			image[i][j].b = image_pixel.b;
		}
	}
}

// Apply a specific filter to the image
// Return -1 for allocation errors
int apply_filter(T_pixel ***image, int width, int height, int filter[3][3])
{
	int i, j;
	int R, G, B;
	T_pixel **tmp_img = Initialize_Image(width, height);

	if (*image == NULL)
	{
		fprintf(stderr, "Memory allocation failed!");
		return ERROR;
	}

	T_pixel **aux_img = *image;

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			// Initialize colors
			R = 0;
			G = 0;
			B = 0;

			// Calculate colors
			R += (*image)[i][j].r * filter[1][1];
			G += (*image)[i][j].g * filter[1][1];
			B += (*image)[i][j].r * filter[1][1];

			if (i > 0)
			{
				R += (*image)[i - 1][j].r * filter[0][1];
				G += (*image)[i - 1][j].g * filter[0][1];
				B += (*image)[i - 1][j].b * filter[0][1];

				if (j > 0)
				{
					R += (*image)[i - 1][j - 1].r *
						filter[0][0];
					G += (*image)[i - 1][j - 1].g *
						filter[0][0];
					B += (*image)[i - 1][j - 1].b *
						filter[0][0];
				}
				if (j < width - 1)
				{
					R += (*image)[i - 1][j + 1].r *
						filter[0][2];
					G += (*image)[i - 1][j + 1].g *
						filter[0][2];
					B += (*image)[i - 1][j + 1].b *
						filter[0][2];
				}
			}
			if (i < height - 1)
			{
				R += (*image)[i + 1][j].r * filter[2][1];
				G += (*image)[i + 1][j].g * filter[2][1];
				B += (*image)[i + 1][j].b * filter[2][1];

				if (j > 0)
				{
					R += (*image)[i + 1][j - 1].r *
						filter[2][0];
					G += (*image)[i + 1][j - 1].g *
						filter[2][0];
					B += (*image)[i + 1][j - 1].b *
						filter[2][0];
				}
				if (j < width - 1)
				{
					R += (*image)[i + 1][j + 1].r *
						filter[2][2];
					G += (*image)[i + 1][j + 1].g *
						filter[2][2];
					B += (*image)[i + 1][j + 1].b *
						filter[2][2];
				}
			}
			if (j > 0)
			{
				R += (*image)[i][j - 1].r * filter[1][0];
				G += (*image)[i][j - 1].g * filter[1][0];
				B += (*image)[i][j - 1].b * filter[1][0];
			}
			if (j < width - 1)
			{
				R += (*image)[i][j + 1].r * filter[1][2];
				G += (*image)[i][j + 1].g * filter[1][2];
				B += (*image)[i][j + 1].b * filter[1][2];
			}

			// Fix min value
			if (R < MIN_PX_VAL)
				R = MIN_PX_VAL;
			if (G < MIN_PX_VAL)
				G = MIN_PX_VAL;
			if (B < MIN_PX_VAL)
				B = MIN_PX_VAL;

			// Fix max value
			if (R > MAX_PX_VAL)
				R = MAX_PX_VAL;
			if (G > MAX_PX_VAL)
				G = MAX_PX_VAL;
			if (B > MAX_PX_VAL)
				B = MAX_PX_VAL;

			// Apply pixel color
			tmp_img[i][j].r = R;
			tmp_img[i][j].g = G;
			tmp_img[i][j].b = B;
		}
	}

	*image = tmp_img;
	tmp_img = aux_img;
	Free_Image(&tmp_img, height);

	return 0;
}

// Check if two pixels are in the same area
int check_threshold(const T_pixel *pixel1, const T_pixel *pixel2,
	int threshold)
{
	if (abs(pixel1->r - pixel2->r) +
		abs(pixel1->g - pixel2->g) +
		abs(pixel1->b - pixel2->b) <= threshold)
	{
		return 1;
	}	
	return 0;
}

// This increases the memory for queue with double size
// Return -1 for allocation errors
int increase_memory(T_point **queue, int size_queue)
{
	T_point *queue_bk = *queue;
	T_point *new_queue;
	int i;

	new_queue = (T_point*)malloc(size_queue * 2 * sizeof(T_point));

	if (new_queue == NULL)
	{
		fprintf(stderr, "Memory allocation failed!");
		return ERROR;
	}

	for (i = 0; i < size_queue; i++)
		new_queue[i] = (*queue)[i];

	*queue = new_queue;

	free(queue_bk);
	return 0;
}

// Compress image for a given pixel
void flood_fill(T_pixel **image, int width, int height, int threshold,
	int X, int Y, T_pixel initial_color, int **visited)
{
	int n, index, index_x, queue_no, west, east, mem_factor = 1;
	T_point *queue;
	n = 1;

	if (!check_threshold(&image[Y][X], &initial_color, threshold))
		return;

	// Create a queue
	queue = (T_point*)malloc(DEFAULT_TMP_MEMORY * sizeof(T_point));
	queue[n - 1].X = X;
	queue[n - 1].Y = Y;

	for (queue_no = 0; queue_no < n; queue_no++)
	{
		west = east = queue[queue_no].X;

		// Move to west
		while (west < width - 1 &&
			check_threshold(&image[queue[queue_no].Y][west + 1],
			&initial_color, threshold) &&
			!visited[queue[queue_no].Y][west + 1])
		{
			west++;
		}

		// Move to east
		while (east > 0 &&
			check_threshold(&image[queue[queue_no].Y][east - 1],
			&initial_color, threshold) &&
			!visited[queue[queue_no].Y][east - 1])
		{
			east--;
		}

		// Find north and south pixels for every pixel from the line
		for (index_x = east; index_x <= west; index_x++)
		{
			if (visited[queue[queue_no].Y][index_x])
				continue;

			image[queue[queue_no].Y][index_x] = initial_color;
			visited[queue[queue_no].Y][index_x] = 1;

			if (queue[queue_no].Y > 0 &&
				!visited[queue[queue_no].Y - 1][index_x] &&
				check_threshold(
				&image[queue[queue_no].Y - 1][index_x],
				&initial_color, threshold))
			{
				n++;
				//Increase memory if it's necessary
				if (n % (DEFAULT_TMP_MEMORY * mem_factor) == 0)
				{
					increase_memory(&queue, n);
					mem_factor *= 2;
				}

				queue[n - 1].Y = queue[queue_no].Y - 1;
				queue[n - 1].X = index_x;
			}

			if (queue[queue_no].Y < height - 1 &&
				!visited[queue[queue_no].Y + 1][index_x] &&
				check_threshold(
				&image[queue[queue_no].Y + 1][index_x],
				&initial_color, threshold))
			{

				n++;
				//Increase memory if it's necessary
				if (n % (DEFAULT_TMP_MEMORY * mem_factor) == 0)
				{
					increase_memory(&queue, n);
					mem_factor *= 2;
				}

				queue[n - 1].Y = queue[queue_no].Y + 1;
				queue[n - 1].X = index_x;
			}

		}

		for (index = 0; index < n; index++)
			queue[index] = queue[index + 1];
		n--;
		queue_no--;

	}
	free(queue);
}

// Apply compression algorithm
// Return -1 for allocation errors
int compress_image(T_pixel **image, int width, int height, int threshold)
{
	int i, j;

	// Create a matrix which stores if a pixel was visited 
	// while compressing
	int **visited = (int**)malloc(height * sizeof(int*));
	for (i = 0; i < height; i++)
	{
		visited[i] = (int*)calloc(width, sizeof(int));
		if (visited[i] == NULL)
		{
			for (j = 0; j < i; j++)
				free(visited[j]);
			free(visited);
		}
	}

	// If there is no memory allocated for visited, 
	// the program will not compress the image
	if (visited == NULL)
		return ERROR;

	// Compress image
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			if (!visited[i][j])
				flood_fill(image, width, height, threshold,
					j, i, image[i][j], visited);
		}
	}

	// Releaase unnecesary memory
	for (i = 0; i < height; i++)
		free(visited[i]);
	free(visited);

	return 0;
}

int count_neighbors(T_pixel **image, int width, int height, T_point point)
{
	int no_neighbors = 0;

	if (point.X > 0)
	{
		if (check_colors(&image[point.Y][point.X - 1],
			&image[point.Y][point.X]))
		{
			no_neighbors++;
		}
	}
	if (point.X < width - 1)
	{
		if (check_colors(&image[point.Y][point.X + 1],
			&image[point.Y][point.X]))
		{
			no_neighbors++;
		}
	}
	if (point.Y > 0)
	{
		if (check_colors(&image[point.Y - 1][point.X],
			&image[point.Y][point.X]))
		{
			no_neighbors++;
		}
	}
	if (point.Y < height - 1)
	{
		if (check_colors(&image[point.Y + 1][point.X],
			&image[point.Y][point.X]))
		{
			no_neighbors++;
		}
	}

	return no_neighbors;
}

// Generate a compressed BMP file
void compress_BMP_file(T_bmp_fileheader bmp_fh, T_bmp_infoheader bmp_ih,
	char **gap, T_pixel **image, FILE *file)
{
	short i, j;
	T_point point;

	// Append File Header
	fwrite(&bmp_fh.fileMarker1, sizeof(unsigned char), 1, file);
	fwrite(&bmp_fh.fileMarker2, sizeof(unsigned char), 1, file);
	fwrite(&bmp_fh.bfSize, sizeof(unsigned int), 1, file);
	fwrite(&bmp_fh.unused1, sizeof(unsigned short), 1, file);
	fwrite(&bmp_fh.unused2, sizeof(unsigned short), 1, file);
	fwrite(&bmp_fh.imageDataOffset, sizeof(unsigned int), 1, file);

	// Append Info Header
	fwrite(&bmp_ih.biSize, sizeof(unsigned int), 1, file);
	fwrite(&bmp_ih.width, sizeof(signed int), 1, file);
	fwrite(&bmp_ih.height, sizeof(signed int), 1, file);
	fwrite(&bmp_ih.planes, sizeof(unsigned short), 1, file);
	fwrite(&bmp_ih.bitPix, sizeof(unsigned short), 1, file);
	fwrite(&bmp_ih.biCompression, sizeof(unsigned int), 1, file);
	fwrite(&bmp_ih.biSizeImage, sizeof(unsigned int), 1, file);
	fwrite(&bmp_ih.biXPelsPerMeter, sizeof(int), 1, file);
	fwrite(&bmp_ih.biYPelsPerMeter, sizeof(int), 1, file);
	fwrite(&bmp_ih.biClrUsed, sizeof(unsigned int), 1, file);
	fwrite(&bmp_ih.biClrImportant, sizeof(unsigned int), 1, file);

	// Append gap
	fwrite(*gap, sizeof(char), bmp_fh.imageDataOffset - ftell(file), file);

	// Append compressed image
	for (i = 0; i < bmp_ih.height; i++)
	{
		for (j = 0; j < bmp_ih.width; j++)
		{
			point.X = j;
			point.Y = i;

			if (count_neighbors(image, bmp_ih.width, bmp_ih.height,
				point) != 4)
			{
				point.X++;
				point.Y++;
				fwrite(&point.Y, sizeof(short), 1, file);
				fwrite(&point.X, sizeof(short), 1, file);
				fwrite(&image[i][j].r, sizeof(unsigned char),
					1, file);
				fwrite(&image[i][j].g, sizeof(unsigned char),
					1, file);
				fwrite(&image[i][j].b, sizeof(unsigned char),
					1, file);
			}
		}
	}
}

// Read a chunk of pixel data
void read_compressed_pixel(FILE *file, short *X, short *Y, T_pixel *pixel)
{
	fread(Y, sizeof(short), 1, file);
	fread(X, sizeof(short), 1, file);
	fread(&pixel->r, sizeof(unsigned char), 1, file);
	fread(&pixel->g, sizeof(unsigned char), 1, file);
	fread(&pixel->b, sizeof(unsigned char), 1, file);

	// Make a corection here
	(*X)--;
	(*Y)--;
}

// Decompress a BMP file
// Return -1 for allocation errors
int decompress_BMP_file(T_bmp_fileheader *bmp_fh, T_bmp_infoheader *bmp_ih,
	char **gap, T_pixel ***image, FILE *file)
{
	int difference;
	int current_pos;
	int num_bmp_chunk;
	T_point point1, point2;
	T_pixel pixel1, pixel2;

	read_bmp_fh(file, bmp_fh);
	read_bmp_ih(file, bmp_ih);
	if (read_gap_1(gap, bmp_fh->imageDataOffset - ftell(file),
			file) == ERROR)
		return ERROR;

	current_pos = ftell(file);
	fseek(file, 0, SEEK_END);
	num_bmp_chunk = (ftell(file) - current_pos) / SIZEOF_CHUNK;
	fseek(file, current_pos, SEEK_SET);

	*image = Initialize_Image(bmp_ih->width, bmp_ih->height);
	if (image == NULL)
	{
		fprintf(stderr, "Memory allocation failed!");
		return ERROR;
	}

	// Store first pixel from compressed image.
	// It should be pixel[0][0].
	if (num_bmp_chunk >= 1)
	{
		read_compressed_pixel(file, &point1.X, &point1.Y, &pixel1);
		(*image)[point1.Y][point1.X] = pixel1;
		num_bmp_chunk--;
	}

	// Read compressed image
	while (num_bmp_chunk != 0)
	{
		read_compressed_pixel(file, &point2.X, &point2.Y, &pixel2);
		num_bmp_chunk--;
		difference = point2.X - point1.X;
		if (point1.Y == point2.Y)
		{
			// Add missing pixels
			while (difference > 1)
			{
				(*image)[point1.Y][++point1.X] = pixel1;
				difference = point2.X - point1.X;
			}
		}
		(*image)[point2.Y][point2.X] = pixel2;

		point1.X = point2.X;
		point1.Y = point2.Y;
		pixel1 = pixel2;
	}
	return 0;
}

void apply_all_filters(T_bmp_fileheader *bmp_fh, T_bmp_infoheader *bmp_ih,
	char **gap, T_pixel **BitMap_BW, char **image_filename)
{
	char *perfix_F[] = { "_f1.bmp", "_f2.bmp", "_f3.bmp" };
	char *new_img_name;
	int f;
	FILE *output;
	T_pixel **BitMap_BW_F = NULL;

	// Filters
	int F[FILTERS_NUM][3][3] = {
		{ { -1, -1, -1 }, { -1, 8, -1 }, { -1, -1, -1 } },
		{ { 0, 1, 0 }, { 1, -4, 1 }, { 0, 1, 0 } },
		{ { 1, 0, -1 }, { 0, 0, 0 }, { -1, 0, 1 } }
	};

	for (f = 0; f < FILTERS_NUM; f++)
	{
		printf("Applying filter %d...\n", f);

		// Prepare image for filters
		if (Copy_Image(&BitMap_BW_F, BitMap_BW,
			bmp_ih->width, bmp_ih->height) == ERROR)
		{
			fprintf(stderr, "Memory allocation failed!");
			Free_All_Memory("%c%b", &new_img_name,
				&BitMap_BW_F, bmp_ih->height);
			return;
		}

		new_img_name = generate_new_name(*image_filename, perfix_F[f]);

		// Check memory allocation
		if (new_img_name == NULL)
		{
			fprintf(stderr, "Memory allocation failed!");
			Free_All_Memory("%c%b", &new_img_name, 
				&BitMap_BW_F, bmp_ih->height);
			return;
		}

		// Open file
		output = fopen(new_img_name, "wb");

		// Apply filter
		if (apply_filter(&BitMap_BW_F, bmp_ih->width, bmp_ih->height, 
			F[f]) == ERROR)
		{
			Free_All_Memory("%c%b", &new_img_name,
				&BitMap_BW_F, bmp_ih->height);
			fclose(output);
			return;
		}

		// Create image
		Create_BMP_file(*bmp_fh, *bmp_ih, gap, BitMap_BW_F, output);

		// Close file and release unnecesary memory
		fclose(output);
		free(new_img_name);
	}

	Free_Image(&BitMap_BW_F, bmp_ih->height);
}


int main()
{
	// Variables for files
	char input_filename[] = "input.txt";
	char perfix_BW[] = "_black_white.bmp";
	char name_compressed[] = "compressed.bin";
	char name_decompressed[] = "decompressed.bmp";

	char *image_filename, *new_img_name, *name_to_decompress;
	FILE *input, *image, *output;

	// BMP variables
	T_bmp_fileheader bmp_fh;
	T_bmp_infoheader bmp_ih;
	T_pixel **BitMap = NULL, **BitMap_BW = NULL;
	char *gap;
	int threshold;

	// Decompressed file
	T_bmp_fileheader bmp_fh_dec;
	T_bmp_infoheader bmp_ih_dec;
	T_pixel **BitMap_dec;
	char *gap_dec;

	// Preparations for reading image name
	if (open_file_read(&input, input_filename, 't') == ERROR)
	{
		exit(EXIT_FAILURE);
	}
	image_filename = malloc(sizeof_line(input) * sizeof(char));
	sizeof_line(input); // skip second line
	name_to_decompress = malloc(sizeof_line(input) * sizeof(char));
	fseek(input, 0, SEEK_SET);

	// Read image name and threshold
	fscanf(input, "%s", image_filename);
	fscanf(input, "%d", &threshold);
	fscanf(input, "%s", name_to_decompress);
	if (open_file_read(&image, image_filename, 'b') == ERROR)
	{
		Free_All_Memory("%c%c", &image_filename, &name_to_decompress);
		exit(EXIT_FAILURE);
	}
	new_img_name = generate_new_name(image_filename, perfix_BW);

	if (new_img_name == NULL)
	{
		Free_All_Memory("%c%c", &image_filename, &name_to_decompress);
		exit(EXIT_FAILURE);
	}

	// Read headers and image
	read_bmp_fh(image, &bmp_fh);
	read_bmp_ih(image, &bmp_ih);

	if (read_gap_1(&gap, bmp_fh.imageDataOffset - ftell(image), 
		image) == ERROR)
	{
		Free_All_Memory("%c%c%c", &image_filename, &name_to_decompress,
			&new_img_name);
		exit(EXIT_FAILURE);
	}
	if (read_bit_map(image, &BitMap, calculate_padding(bmp_fh, bmp_ih),
		bmp_ih.width, bmp_ih.height) == ERROR)
	{
		Free_All_Memory("%c%c%c", &image_filename, &name_to_decompress,
			&new_img_name);
		exit(EXIT_FAILURE);
	}

	if (Copy_Image(&BitMap_BW, BitMap, bmp_ih.width, bmp_ih.height) 
		== ERROR)
	{
		Free_All_Memory("%c%c%c", &image_filename, &name_to_decompress,
			&new_img_name);
		exit(EXIT_FAILURE);
	}

	// Create black-white BMP image
	printf("Applying black and white...\n");
	output = fopen(new_img_name, "wb");
	make_black_white(BitMap_BW, bmp_ih.width, bmp_ih.height);
	Create_BMP_file(bmp_fh, bmp_ih, &gap, BitMap_BW, output);

	// Close file and remove unnecesary memory
	fclose(output);
	free(new_img_name);

	// Apply filters
	apply_all_filters(&bmp_fh, &bmp_ih, &gap, BitMap_BW, &image_filename);

	// Compressing image
	printf("Compressing...\n");
	output = fopen(name_compressed, "wb");
	compress_image(BitMap, bmp_ih.width, bmp_ih.height, threshold);
	compress_BMP_file(bmp_fh, bmp_ih, &gap, BitMap, output);
	fclose(output);

	// Decompressing image
	printf("Decompressing...\n");
	if (open_file_read(&output, name_to_decompress, 'b') == ERROR)
	{
		Free_All_Memory("%c%c%c%b%b", &image_filename, 
			&name_to_decompress, &gap, &BitMap, bmp_ih.height, 
			&BitMap_BW, bmp_ih.height);
		exit(EXIT_FAILURE);
	}
	if (decompress_BMP_file(&bmp_fh_dec, &bmp_ih_dec,
		&gap_dec, &BitMap_dec, output) == ERROR)
	{
		Free_All_Memory("%c%c%c%b%b", &image_filename,
			&name_to_decompress, &gap, &BitMap, bmp_ih.height,
			&BitMap_BW, bmp_ih.height);
		exit(EXIT_FAILURE);
	}
	fclose(output);

	// Create decpompressed image
	output = fopen(name_decompressed, "wb");
	Create_BMP_file(bmp_fh_dec, bmp_ih_dec,
		&gap_dec, BitMap_dec, output);
	fclose(output);

	// Close files	
	fclose(input);
	fclose(image);
	
	// Remove unnecesary memory
	Free_All_Memory("%c%c%c%c%b%b%b", &gap, &gap_dec, &image_filename, 
		&name_to_decompress, &BitMap, bmp_ih.height,
		&BitMap_BW, bmp_ih.height,
		&BitMap_dec, bmp_ih_dec.height);

	return 0;
}
