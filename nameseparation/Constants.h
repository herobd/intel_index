#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <limits>  
#define INT_POS_INFINITY std::numeric_limits<int>::max()
#define INT_NEG_INFINITY std::numeric_limits<int>::min()
#define DOUBLE_POS_INFINITY std::numeric_limits<double>::max()
#define DOUBLE_NEG_INFINITY std::numeric_limits<double>::min()

#define BLACK 0
#define WHITE 255

#define SPLIT_HORZ 1
#define SPLIT_VERT 2
#define CHOP_TOP 3

#define PI      3.14159265358
#define HALF_PI 1.57079632679

#define SLOPE_DIF_TOLERANCE .16666 //bins, + or -

#define ANCHOR_L 3.05
#define ANCHOR_R 2.4

#endif // CONSTANTS_H
