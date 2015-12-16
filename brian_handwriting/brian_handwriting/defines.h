#ifndef DEFINES_H
#define DEFINES_H

#define USE_CODEBOOK_2 1

inline int mod(int a, int b)
{
    while (a<0) a+=b;
    return a%b;
}
inline int xDelta(int direction)
{
    int xDelta=1;
    if (direction>2 && direction<6) xDelta=-1;
    else if (direction==2 || direction==6) xDelta=0;
    return xDelta;
}

inline int yDelta(int direction)
{
    int yDelta=0;
    if (direction>0 && direction<4) yDelta=-1;
    else if (direction>4 && direction<8) yDelta=1;
    return yDelta;
}

#endif // DEFINES_H
