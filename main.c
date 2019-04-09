/*
Copyright 2019 Nira Tubert

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

void histogram(char *addr, int length)
{
    int histR[32];
    int histG1[32];
    int histG2[32];
    int histB[32];

    //initialize histograms to 0(might be trash in memory)
    for (int i = 0; i < 32; i++) {
	histR[i] = 0;
	histG1[i] = 0;
	histG2[i] = 0;
	histB[i] = 0;
    }

    //number of bytes read
    int nb = 0;
    //number of pixels read
    int np = 0;

    while (nb < 4096 * 3072 * 12 / 8) {
	unsigned char First8bits = addr[nb];
	unsigned char Second8bits = addr[nb + 1];
	unsigned char Third8bits = addr[nb + 2];

	if ((np / 4096) % 2 == 0) {
	    //even row, RG
	    unsigned int RedChannel =
		First8bits << 4 | (Second8bits & 0xF0) >> 4;
	    unsigned int GreenChannel =
		(Second8bits & 0x0F) << 8 | Third8bits;
	    histR[RedChannel >> 7] += 1;
	    histG1[GreenChannel >> 7] += 1;
	} else {
	    //odd row, GB
	    unsigned int GreenChannel =
		First8bits << 4 | (Second8bits & 0xF0) >> 4;
	    unsigned int BlueChannel =
		(int) (Second8bits & 0x0F) << 8 | (int) Third8bits;
	    histG2[GreenChannel >> 7] += 1;
	    histB[BlueChannel >> 7] += 1;
	}

	//process 3 bytes(2 px) at a time
	nb += 3;
	np += 2;
    }

    //print histograms in 5 columns
    printf("  Bucket        R       G1       G2        B\n");
    for (int n = 0; n < 32; n++)
	printf("%8d %8d %8d %8d %8d\n", n * 128, histR[n], histG1[n],
	       histG2[n], histB[n]);
}

long long timeInMilliseconds(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (((long long) tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

int main(int argc, char *argv[])
{
    struct stat sb;
    int length;

    if (argc != 2) {
	printf("Usage: %s <filename>\n", argv[0]);
	exit(-1);
    }
    char *filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
	printf("Error while opening file for reading\n");
	exit(-1);
    }
    if (fstat(fd, &sb) == -1) {
	printf("Error getting file size\n");
	exit(-1);
    }
    length = sb.st_size;

    char *addr = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
	printf("Error while mapping file into memory\n");
	exit(-1);
    }
    long long start = timeInMilliseconds();
    histogram(addr, length);
    long long end = timeInMilliseconds();
    printf("Time: %lld ms\n", end - start);

    munmap(addr, length);
    close(fd);
}
