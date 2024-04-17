// g++ main.cc

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define DIM 1024
#define DM1 (DIM-1)
#define _sq(x) ((x)*(x)) // square
#define _cb(x) abs((x)*(x)*(x)) // absolute value of cube
#define _cr(x) (unsigned char)(pow((x),1.0/3.0)) // cube root

static FILE * fp = NULL;

unsigned char GR(int,int);
unsigned char BL(int,int);

#if 0
unsigned char RD(int i,int j){
	return 0; // YOUR CODE HERE
}
unsigned char GR(int i,int j){
	return 0; // YOUR CODE HERE
}
unsigned char BL(int i,int j){
	return 0; // YOUR CODE HERE
}

#else

const int center_x = 1024/2;
const int center_y = 1024/2;

unsigned char RD(int i, int j) {
	return BL(i, j);
}

unsigned char GR(int i, int j) {
	return BL(i, j);
}

unsigned char BL(int i, int j) {
	const int n = 3;

	float r = sqrt(0.f+_sq(i-center_y)+_sq(j-center_x));
	float s = r/((DIM/2)/2);

	if(s < 1) {
		i -= center_y;
		j -= center_x;

		float k = (
			sqrt(1.-_sq(s)) * sqrt(1.-_sq(s/n)) +
			_sq(s)/n
		) * n;

		i = i/k + center_y;
		j = j/k + center_x;
	}

	return (i/((DIM/2)/8)+j/((DIM/2)/8))%2*255;
}
#endif

void pixel_write(int i, int j){
	static unsigned char color[3];
	color[0] = RD(i,j)&255;
	color[1] = GR(i,j)&255;
	color[2] = BL(i,j)&255;
	fwrite(color, 1, 3, fp);
}

int main() {
	fp = fopen("MathPic.ppm","wb");
	fprintf(fp, "P6\n%d %d\n255\n", DIM, DIM);
	for(int j=0;j<DIM;j++) {
		for(int i=0;i<DIM;i++) {
			pixel_write(i,j);
		}
	}
	fclose(fp);
	return 0;
}