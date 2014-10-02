#include "ngramtrainingexample.h"

NGramInstance::NGramInstance(const BPixelCollection& pixels, int n)
{
    this->n = n;
    img=pixels;
}

NGramInstance::NGramInstance(const BImage& pixels, int n)
{
    this->n = n;
    img=pixels;
}
