#include "descenderpath.h"

DescenderPath::DescenderPath(const BlobSkeleton* skeleton)
{
    this->skeleton=skeleton;
    holdScore=-1;
    divideIndex=-1;
}

DescenderPath::DescenderPath(const DescenderPath &other)
{
    this->skeleton = other.skeleton;
    this->holdScore = other.holdScore;
//    this->upperPath = other.upperPath;
//    this->lowerPath = other.lowerPath;
    this->path = other.path;
    this->divideIndex = other.divideIndex;
}

void DescenderPath::append(unsigned int next)//0x7fffffff7bd0
{
//    if (lowerPath.size()>0 && (upperPath.size()>0 || (*skeleton)[next].y < (*skeleton)[lowerPath.last()].y))
//    {
//        upperPath.append(next);
//    }
//    else
//    {
//        lowerPath.append(next);
//    }
    
    if (divideIndex<0 && path.size()>0 && (*skeleton)[next].y < (*skeleton)[path.last()].y)
    {
        divideIndex=path.size()-1;
    }
    
    path.append(next);
    
    double lowerScore = this->computeHalfScore(false);
    
    if (divideIndex>0)
    {
        double upperScore = this->computeHalfScore(true);
        if (upperScore!=0 && lowerScore!=0)
            holdScore=(upperScore+lowerScore)/2;
        else
            holdScore=std::max(upperScore, lowerScore);
    }
    else
    {
        holdScore=lowerScore;
    }
    
    //bias complete loops
    if (next==path.first())
        holdScore *= .75;
}

unsigned int DescenderPath::at(unsigned int index) const
{
//    if (index<lowerPath.size())
//        return lowerPath[index];
    
//    return upperPath[index-lowerPath.size()];
    return path[index];
}

QPoint DescenderPath::pointAt(unsigned int index) const
{
//    if (index<lowerPath.size())
//    {
//        QPoint ret((*skeleton)[lowerPath[index]].x,(*skeleton)[lowerPath[index]].y);
//        return ret;
//    }
    
//    QPoint ret((*skeleton)[upperPath[index-lowerPath.size()]].x,(*skeleton)[upperPath[index-lowerPath.size()]].y);
//    return ret;
    QPoint ret((*skeleton)[path[index]].x,(*skeleton)[path[index]].y);
    return ret;
}

unsigned int DescenderPath::size() const
{
//    return upperPath.size() + lowerPath.size();
    return path.size();
}

double DescenderPath::getRelAngle(int indexA, int indexB, int indexC) const
{
    double angle1 = (atan2(((*skeleton)[indexA].y-(*skeleton)[indexB].y),((*skeleton)[indexA].x-(*skeleton)[indexB].x)));
    double angle2 = (atan2(((*skeleton)[indexB].y-(*skeleton)[indexC].y),((*skeleton)[indexB].x-(*skeleton)[indexC].x)));
    double ret = PI-angle2+angle1;
    if (ret<0) ret+=2*PI;
    return ret;
}

/*from < http://rosettacode.org/wiki/Polynomial_regression#C >*/

double DescenderPath::polynomialfit(int obs, int degree, 
		   double *dx, double *dy, double *store, double *covarience) const /* n, p */
{
  gsl_multifit_linear_workspace *ws;
  gsl_matrix *cov, *X;
  gsl_vector *y, *c;
  double chisq;
 
  int i, j;
 
  X = gsl_matrix_alloc(obs, degree);
  y = gsl_vector_alloc(obs);
  c = gsl_vector_alloc(degree);
  cov = gsl_matrix_alloc(degree, degree);
 
  for(i=0; i < obs; i++) {
    gsl_matrix_set(X, i, 0, 1.0);
    for(j=0; j < degree; j++) {
      gsl_matrix_set(X, i, j, pow(dx[i], j));
    }
    gsl_vector_set(y, i, dy[i]);
  }
 
  ws = gsl_multifit_linear_alloc(obs, degree);
  gsl_multifit_linear(X, y, c, cov, &chisq, ws);
 
  /* store result ... */
  for(i=0; i < degree; i++)
  {
    store[i] = gsl_vector_get(c, i);
  }
  
  for(i=0; i < degree; i++)
  {
      for(j=0; j < degree; j++)
        covarience[i+j*degree] = gsl_matrix_get(cov, i, j);
  }
 
  gsl_multifit_linear_free(ws);
  gsl_matrix_free(X);
  gsl_matrix_free(cov);
  gsl_vector_free(y);
  gsl_vector_free(c);
  return chisq; /* we do not "analyse" the result (cov matrix mainly)
		  to know if the fit is "good" */
}

double DescenderPath::computeHalfScore(bool upper, bool print) const
{
    double meanCurve;
    double stdDevCurve;
                      
    double meanSlope;
    double stdDevSlope;
    int startIndex;
    int endIndex;
//    const QVector<unsigned int>* path;
    if (upper)
    {
        meanCurve = NEW_UPPER_MEAN_CURVE;
        stdDevCurve = NEW_UPPER_STD_DEV_CURVE;
        
        meanSlope = NEW_UPPER_MEAN_SLOPE;
        stdDevSlope = NEW_UPPER_STD_DEV_SLOPE;
//        path = &upperPath;
        startIndex=divideIndex;
        endIndex=path.size()-1;
        if (print)
            printf("Upper:\t");
    }
    else
    {
        meanCurve = NEW_LOWER_MEAN_CURVE;
        stdDevCurve = NEW_LOWER_STD_DEV_CURVE;
        
        meanSlope = NEW_LOWER_MEAN_SLOPE;
        stdDevSlope = NEW_LOWER_STD_DEV_SLOPE;
//        path = &lowerPath;
        startIndex=0;
        endIndex=divideIndex>0?divideIndex:path.size()-1;
        if (print)
            printf("Lower:\t");
    }
    
    //        double relativeAngle = getRelAngle(skeleton, currentPath->at(currentPath->size()-2), currentPath->last(), nextIndex);
    //        double distance = getDist(skeleton, currentPath->last(), nextIndex);
    //        double newClockwiseScore = (clockwiseScore/1.5)+std::min(PI-relativeAngle,PI/3)*distance;
    double avgAngle;
    
    int sampleSize = 1+endIndex-startIndex;
    
    double x[sampleSize];
    double y[sampleSize];
    for (int i=startIndex; i<=endIndex; i++)
    {
        x[i-startIndex]=(*skeleton)[path[i]].x;
        y[i-startIndex]=(*skeleton)[path[i]].y;
        
        if (i-startIndex>1)
        {
//            std::min(PI-relativeAngle,PI/3)
            avgAngle += std::max(getRelAngle(path[i-2], path[i-1], path[i]),PI*0.6666);
        }
    }
    if (sampleSize>2)
    {
        avgAngle /= sampleSize-2;
    }
    else
    {
        avgAngle = 0;
    }
    
    double halfScore=0;

    if (sampleSize>2)
    {
        double tss = gsl_stats_tss(x,1,sampleSize);
        double cov[9];
        double linOut[2];
        double chisqSlope= polynomialfit(sampleSize,2,y,x,linOut,cov);
        double slope=linOut[1];
        double rsqSlope=1-chisqSlope/tss;
        
        double quadOut[3];
        double chisqCurve = polynomialfit(sampleSize,3,y,x,quadOut,cov);
        double curvature=quadOut[2];
        double rsqCurve=1-chisqCurve/tss;
        
        double yOfVertex = quadOut[1]/(2*quadOut[2]);
        
//        double curveScore = (copysign(1.0, curvature) == copysign(1.0, meanCurve) ||
//                             fabs(curvature)<0.001) ? std::max(fabs(curvature-meanCurve),2*stdDevCurve) :
//                                                               std::max(fabs(curvature-meanCurve),2*stdDevCurve);
        halfScore = 10*(1/std::max(rsqCurve,0.1))*std::max(fabs(curvature-meanCurve),2*stdDevCurve)/(2*stdDevCurve) + .1*std::max(fabs(slope-meanSlope),2*stdDevSlope)/(2*stdDevSlope);
        
        
    }
    
    if(print)
        printf("fit=%f\tangle=%f\t",halfScore,.1*avgAngle);
    
    halfScore += .1*avgAngle;
    
    if(print)
        printf("total=%f\n",halfScore);
    
    return halfScore;
}

void DescenderPath::printScore() const
{
    computeHalfScore(true,true);
    computeHalfScore(false,true);
}

double DescenderPath::score() const
{
    
    return holdScore;
}

bool DescenderPath::hasTop() const
{
    return divideIndex>0 && path.size()-divideIndex>2 && holdScore>0;
}

unsigned int DescenderPath::last() const
{
//    if (upperPath.size()>0)
//    {
//        return upperPath.last();
//    }
//    return lowerPath.last();
    return path.last();
}

bool DescenderPath::contains(unsigned int i) const
{
//    return upperPath.contains(i) || lowerPath.contains(i);
    return path.contains(i);
}
