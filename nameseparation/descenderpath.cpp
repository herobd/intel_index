#include "descenderpath.h"

DescenderPath::DescenderPath(const BlobSkeleton* skeleton)
{
    this->skeleton=skeleton;
    holdScore=-1;
    divideIndex=-1;
    upperLength=0;
    lowerLength=0;
}

DescenderPath::DescenderPath(const DescenderPath &other)
{
    this->skeleton = other.skeleton;
    this->holdScore = other.holdScore;
    this->upperLength = other.upperLength;
    this->lowerLength = other.lowerLength;
//    this->upperPath = other.upperPath;
//    this->lowerPath = other.lowerPath;
    this->path = other.path;
    this->divideIndex = other.divideIndex;
}

inline double DescenderPath::computeLenRatioBias() const
{
    return upperLength < lowerLength*(2/3.0) || upperLength*(2/3.0) > lowerLength ? 
                    .5+.5*fabs(lowerLength-upperLength)/std::max(lowerLength*(1/3.0),upperLength*(1/3.0)) : 1;
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
    
    //recalculate score
    
    double lowerScore = this->computeHalfScore(false);
    
    if (divideIndex>0)
    {
        upperLength += (*skeleton)[next].distanceBetween(path[path.size()-2]);
        
        double upperScore = this->computeHalfScore(true);
        if (upperScore!=0 && lowerScore!=0)
        {
//            double lenRatioBias = upperLength > lowerLength/2 && upperLength < lowerLength*2 ? 
//                        .8+.2*fabs(lowerLength-upperLength)/std::max(lowerLength/2,upperLength/2) : 1;
            double lenRatioBias = computeLenRatioBias();
            
            holdScore=lenRatioBias*(upperScore+lowerScore)/2;
        }
        else
            holdScore=std::max(upperScore, lowerScore);
        
        
    }
    else
    {
        if (path.size()>1)
        {
            lowerLength += (*skeleton)[next].distanceBetween(path[path.size()-2]);
        }
        
        holdScore=lowerScore;
        
       
    }
    
    //bias complete loops
    if (path.indexOf(next)>-1 && path.indexOf(next)<= divideIndex)
        holdScore *= .75;//1 - .25*(divideIndex-path.indexOf(next))/divideIndex;
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
//    QVector<double> x;
//    QVector<double> y;
    for (int i=startIndex; i<=endIndex; i++)
    {
        x[i-startIndex]=(*skeleton)[path[i]].x;
        y[i-startIndex]=(*skeleton)[path[i]].y;
        
//        foreach (QPoint p, (*skeleton).getRegion(path[i]))
//        {
//            if ((*skeleton).pixel(p.x(),p.y()))
//            {
//                x.append((double)p.x());
//                y.append((double)p.y());
//            }
//        }
//        if (i>startIndex)
//        {
//            int curX=(*skeleton)[path[i-1]].x;
//            int curY=(*skeleton)[path[i-1]].y;
//            int nextX=(*skeleton)[path[i]].x;
//            int nextY=(*skeleton)[path[i]].y;
//            QVector<QPoint> line;
//            QPoint start(curX,curY);
//            line.append(start);
//            if (curX==nextX || fabs((curY-nextY)/((double)curX-nextX)) > 1)
//            {
//                double slope = ((double)curX-nextX)/(curY-nextY);
//                double intersect = curX-curY*slope;
//                int inc = copysign(1.0, nextY-curY);
//                for (int y=curY+inc; inc*y<inc*nextY; y+=inc)
//                {
//                    QPoint toAdd(y*slope+intersect,y);
//                    if (((BPixelCollection*)skeleton)->pixel(toAdd))
//                        line.append(toAdd);
//                    else
//                    {
//        //                return;
//                    }
//                }
//            }
//            else
//            {
//                double slope = (curY-nextY)/((double)curX-nextX);
//                double intersect = curY-curX*slope;
//                int inc = copysign(1.0, nextX-curX);
//                for (int x=curX+inc; inc*x<inc*nextX; x+=inc)
//                {
//                    QPoint toAdd(x,slope*x+intersect);
//                    if (((BPixelCollection*)skeleton)->pixel(toAdd))
//                        line.append(toAdd);
//                    else
//                    {
//        //                return;
//                    }
//                }
//            }
//            QPoint end(nextX,nextY);
//            line.append(end);
            
//            foreach (QPoint p, line)
//            {

//                x.append((double)p.x());
//                y.append((double)p.y());
//            }
//        }
        
        if (i-startIndex>1)
        {
//            std::min(PI-relativeAngle,PI/3)
            avgAngle += std::max(getRelAngle(path[i-2], path[i-1], path[i]),PI*0.6666);
        }
    }
    
//    int sampleSize = x.size();
    
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
        
        halfScore = (copysign(1.0, curvature) == copysign(1.0, meanCurve) ||
                             fabs(curvature)<0.001) ? 
                     10*(1/std::max(rsqCurve,0.1))*std::max(fabs(curvature-meanCurve),2*stdDevCurve)/(2*stdDevCurve) :
                     15*(1/std::max(rsqCurve,0.1))*std::max(fabs(curvature-meanCurve),2*stdDevCurve)/(2*stdDevCurve);
//        halfScore = 10*(1/std::max(rsqCurve,0.1))*std::max(fabs(curvature-meanCurve),2*stdDevCurve)/(2*stdDevCurve) + .1*std::max(fabs(slope-meanSlope),2*stdDevSlope)/(2*stdDevSlope);
        
        
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
    if (hasTop())
        computeHalfScore(true,true);
    
    computeHalfScore(false,true);
    double lenRatioBias = computeLenRatioBias();
    printf("lenRatioBis=%f,\ttotal score=%f\n",lenRatioBias,holdScore);
}

double DescenderPath::score() const
{
    return holdScore;
}

bool DescenderPath::hasTop() const
{
    return divideIndex>0 && path.size()-divideIndex>2 && holdScore>0 /**/ && (path.indexOf(path.last())>-1 && path.indexOf(path.last())<= divideIndex);
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
