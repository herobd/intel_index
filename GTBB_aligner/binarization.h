
//A combined approach for the binarization of handwritten document images
//Ntirogiannis, Gatos, Pratikakis
//2014

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <list>
#include <functional>
#include <tuple>

using namespace std;
using namespace cv;

namespace Binarization
{
    Mat ntirogiannisBinarization(const Mat& src, int off=255, int on=0, bool visualize=false);

    void imshowB(string window, const Mat& img, int off, int on);

    void test();

    Mat niblackBinarization(const Mat& src, int size, double k);

    Mat dilate(const Mat& src, int size);

    Mat inpainting(const Mat& src, const Mat& mask, Mat* prime=NULL, double* avg=NULL, double* std=NULL, bool show=false);

    Mat img_normalize(const Mat& src, const Mat& bg);

    Mat otsuBinarization(const Mat& src);



    Mat postProcessing(const Mat& bin);

    unsigned char ToByte(bool b[8]);
    Mat LeeChenSkel(const Mat& bin);


    double computeStrokeWidth(int row, int col, const Mat& bin);
    double strokeWidth(const Mat& bin, const Mat& skel);

    void extract_feat(const Mat& src, const Mat* skel, double* average, double* std);

    Mat combine(const Mat& o_bin, const Mat& n_bin, double C);
}
