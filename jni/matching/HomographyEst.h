#ifndef _HOMOGRAPHY_EST_H
#define _HOMOGRAPHY_EST_H

#include <cxcore.h>
#include <cv.h>

#include "kpmtypes.h"

#define EPS 0.5
#define T_SQUARE 100 // t = sqrt(6)* sigma and set sigma = sqrt(6)

void HomograhyEstimation(std::vector<corsp2i>& inlierMap, CvMat* H);
void RansacHomograhyEstimation(std::vector<corsp2i>& corspMap, std::vector<corsp2i>& inlierMap, CvMat* H);
void ComputeHomography(point2f* pt1, point2f* pt2, int num, CvMat* H);
void ComputeHomographyOpenCV(point2f* pt1, point2f* pt2, int num, CvMat* H);
void DataNormalization(int num, point2f* points, CvMat* T);
void CalculateDistance(CvMat* H, std::vector<corsp2i>& corspMap, std::vector<corsp2i>& inlierMap);
bool IsGoodSample(point2f* points, int num);
bool IsColinear(CvMat* A, CvMat* B, CvMat* C);
void ComputeCameraMat(CvMat* H, CvMat* M);

void Array2CvMat(float *arr, CvMat *cvArr, int row, int col);
void CvMat2Array(CvMat *cvArr, float *arr, int row, int col);

void UpdateCorrespMap(std::vector<corsp2i>& corspMap, int x1, int y1, int x2, int y2);

#endif
