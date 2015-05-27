#ifndef GATOSBINARIZE_H
#define GATOSBINARIZE_H

#include "opencv2/core/core.hpp"

using namespace cv;

class GatosBinarize
{
public:
    GatosBinarize();
    Mat binarize(const Mat &src, int dx, int dy, double dpi);
    Mat wienerFIlter(const Mat &src);
    Mat sauvolaThresh(const Mat &src, double dpi, double k);
    Mat approxBackground(const Mat &I, const Mat &S, int dx, int dy);
    Mat finalThresh(const Mat &I, const Mat &S, const Mat &B, double q, double p1, double p2);
};

#endif // GATOSBINARIZE_H
