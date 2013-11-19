#pragma once

#include <vector>

#define MAX_KEY_POINTS 1000

#define ZM_ORDER    7
#define ZM_NUMBER   20  // Depends on ZM_ORDER

#define	PATCH_RADIUS   17
#define	PATCH_SIZE     35  // ZM_RADIUS*2 + 1
//#define PATCH_MEMSIZE  11  // PATCH_RADIUS + 1

// 저니키 모멘트의 경우, 전체 20개 중에서 (0,0) 차에 대한거 빼서 19.
// PCA의 경우, 36개 고유벡터 중에서 19개만 선별해서 사용!!!
#define	FEATURE_DIM			19
#define SURF_FEATURE_DIM	64

template<class T> class Image
{
private:
	IplImage* imgp;
public:
	Image(IplImage* img=0) {imgp=img;}
	~Image(){imgp=0;}
	void operator=(IplImage* img) {imgp=img;}
	inline T* operator[](const int rowIndx) {
		return ((T *)(imgp->imageData + rowIndx*imgp->widthStep));}
};

typedef struct{
	unsigned char b,g,r;
} RgbPixel;

typedef struct{
	float b,g,r;
} RgbPixelFloat;

typedef struct{
	double b,g,r;
} RgbPixelDbl;

typedef struct{
	unsigned char b,g,r,a;
} ARgbPixel;

typedef struct{
	double b,g,r,a;
} ARgbPixelDbl;

typedef Image<RgbPixel>			RgbImage;
typedef Image<RgbPixelFloat>	RgbImageFloat;
typedef Image<RgbPixelDbl>		RgbImageDbl;
typedef Image<unsigned char>	BwImage;
typedef Image<float>			BwImageFloat;
typedef Image<double>			BwImageDbl;
typedef Image<ARgbPixel>		ARgbImage;
typedef Image<ARgbPixelDbl>		ARgbImageDbl;

typedef unsigned char BYTE;

//-------------------------------------------------------------------------
// Corner points
//-------------------------------------------------------------------------

class point2i 
{
public:
	int x;
	int y;

public:
	point2i(int p = 0, int q = 0) : x(p), y(q) {};
};

class point2f
{
public:
	float x;
	float y;

public:
	point2f(float p = 0.f, float q = 0.f) : x(p), y(q) {};
};

class CornerMap
{
public:
	int map[16];

	CornerMap() { memset(map, 0, sizeof(int)*16); };
};

//-------------------------------------------------------------------------
// Correspondence
//-------------------------------------------------------------------------

class corsp2i {
public:
	int x1, y1, x2, y2;

public:
	corsp2i(int p1 = 0, int q1 = 0, int p2 = 0, int q2 = 0) : x1(p1), y1(q1), x2(p2), y2(q2) {};
};

//-------------------------------------------------------------------------
// Zernike feature vector
//-------------------------------------------------------------------------

class ZernikeMoment {
public:
	double re[ZM_NUMBER];
	double im[ZM_NUMBER];

public:
	ZernikeMoment() { 
		memset(re, 0, sizeof(double)*ZM_NUMBER);
		memset(im, 0, sizeof(double)*ZM_NUMBER);
	}
};

class FeatureVector{
public:
	double v[FEATURE_DIM];

public:
	FeatureVector() { memset(v, 0, sizeof(double)*FEATURE_DIM); }
};

//-------------------------------------------------------------------------
// SURF feature vector
//-------------------------------------------------------------------------

class SURFfV{
public:
	double v[SURF_FEATURE_DIM];

public:
	SURFfV() { memset(v, 0, sizeof(double)*SURF_FEATURE_DIM); }
};
