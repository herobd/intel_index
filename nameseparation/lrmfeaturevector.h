//http://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=1263256
//Holistic Word Recognition for Handwritten Historical Documents
//Lavrenko, Rath and Manmatha
#ifndef LRMFEATUREVECTOR_H
#define LRMFEATUREVECTOR_H

#include <bimage.h>
#include <array>

#define HEIGHT 0
#define WIDTH 1
#define ASPECT_RATIO 2
#define AREA 3
#define NUM_DESC 4
#define NUM_ASC 5

#define PROJ_PROFILE 6
#define PROJ_PROFILE_REAL(x) PROJ_PROFILE+2*x
#define PROJ_PROFILE_IMAG(x) PROJ_PROFILE+1+2*x

#define UPPER_PROFILE 13
#define UPPER_PROFILE_REAL(x) UPPER_PROFILE+2*x
#define UPPER_PROFILE_IMAG(x) UPPER_PROFILE+1+2*x

#define LOWER_PROFILE 20
#define LOWER_PROFILE_REAL(x) LOWER_PROFILE+2*x
#define LOWER_PROFILE_IMAG(x) LOWER_PROFILE+1+2*x


class LRMFeatureVector
{
public:
    LRMFeatureVector(const BImage &img);
    
    double distance(const LRMFeatureVector &other);
    
private:
    std::array<double,27> v;
};

#endif // LRMFEATUREVECTOR_H
