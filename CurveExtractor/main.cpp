//#include <QtCore/QCoreApplication>
#include "/home/brian/intel_index/nameseparation/bimage.h"

/*from < http://rosettacode.org/wiki/Polynomial_regression#C >*/
#include <gsl/gsl_multifit.h>
#include <stdbool.h>
#include <math.h>
bool polynomialfit(int obs, int degree, 
		   double *dx, double *dy, double *store) /* n, p */
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
 
  gsl_multifit_linear_free(ws);
  gsl_matrix_free(X);
  gsl_matrix_free(cov);
  gsl_vector_free(y);
  gsl_vector_free(c);
  return true; /* we do not "analyse" the result (cov matrix mainly)
		  to know if the fit is "good" */
}

void evalCCs(const QVector<QVector<QPoint>* > &CCs, bool top)
{
    QVector<double> linear0;
    QVector<double> linear1;
    QVector<double> quadratic0;
    QVector<double> quadratic1;
    QVector<double> quadratic2;
    QVector<double> cubic0;
    QVector<double> cubic1;
    QVector<double> cubic2;
    QVector<double> cubic3;
    double linearMean[2];
    double quadraticMean[3];
    double cubicMean[4];
    
    
    
    for (int i=0; i<CCs.size(); i++)
//    foreach (QVector<QPoint>* cc, CCs)
    {
//        if (cc->size() ==0)
//        {
//            printf("ERROR, 0 CC\n");
//            continue;
//        }
//        printf("cc:%d\n",cc->size());
        
        QVector<QPoint>* cc = CCs[i];
        QPoint* ccData = cc->data();
        
        double x[cc->size()];
        double y[cc->size()];
        double linear[2];
        double quadratic[3];
        double cubic[4];
        
        int xNorm = cc->at(0).x();
        int yNorm = cc->at(0).y();
        
        for (int i=0; i<cc->size(); i++)
        {
            y[i] = cc->at(i).x()-xNorm;
            x[i] = cc->at(i).y()-yNorm;
        }
        
        polynomialfit(cc->size(),2,x,y,linear);
        polynomialfit(cc->size(),3,x,y,quadratic);
        polynomialfit(cc->size(),4,x,y,cubic);
        
//        printf("obs l:(%f,%f)\tq:(%f,%f,%f)\tc:(%f,%f,%f,%f)\n",linear[0],linear[1],quadratic[0],quadratic[1],quadratic[2],cubic[0],cubic[1],cubic[2],cubic[3]);
        linear0.append(linear[0]);
        linear1.append(linear[1]);
        quadratic0.append(quadratic[0]);
        quadratic1.append(quadratic[1]);
        quadratic2.append(quadratic[2]);
        cubic0.append(cubic[0]);
        cubic1.append(cubic[1]);
        cubic2.append(cubic[2]);
        cubic3.append(cubic[3]);
        linearMean[0]+=linear[0];
        linearMean[1]+=linear[1];
        quadraticMean[0]+=quadratic[0];
        quadraticMean[1]+=quadratic[1];
        quadraticMean[2]+=quadratic[2];
        cubicMean[0]+=cubic[0];
        cubicMean[1]+=cubic[1];
        cubicMean[2]+=cubic[2];
        cubicMean[3]+=cubic[3];
        
        
//        if (top && quadratic[2]<0 )
//        {
//            printf("top exception on instance %d (%f)\n",i,quadratic[2]);
//        }
//        else if (!top && quadratic[2]>0 )
//        {
//            printf("bottom exception on instance %d (%f)\n",i,quadratic[2]);
//        }
        if (top && fabs(linear[1]+0.667002) > 2*0.414845)
        {
            printf("top, 2 std_dev on instance %d (%f)\n",i,quadratic[2]);
        }
        else if (!top && fabs(linear[1]+0.695677) > 2*406538)
        {
            printf("bottom 2 std_dev on instance %d (%f)\n",i,quadratic[2]);
        }
    }
    
    linearMean[0]/=CCs.size();
    linearMean[1]/=CCs.size();
    quadraticMean[0]/=CCs.size();
    quadraticMean[1]/=CCs.size();
    quadraticMean[2]/=CCs.size();
    cubicMean[0]/=CCs.size();
    cubicMean[1]/=CCs.size();
    cubicMean[2]/=CCs.size();
    cubicMean[3]/=CCs.size();
    
    double linearSqrSum[2];
    double quadraticSqrSum[3];
    double cubicSqrSum[4];
    for (int i=0; i<CCs.size(); i++)
    {
        linearSqrSum[0] +=  pow(linear0[i]-linearMean[0],2);
        linearSqrSum[1] +=  pow(linear1[i]-linearMean[1],2);
        quadraticSqrSum[0] +=  pow(quadratic0[i]-quadraticMean[0],2);
        quadraticSqrSum[1] +=  pow(quadratic1[i]-quadraticMean[1],2);
        quadraticSqrSum[2] +=  pow(quadratic2[i]-quadraticMean[2],2);
        cubicSqrSum[0] +=  pow(cubic0[i]-cubicMean[0],2);
        cubicSqrSum[1] +=  pow(cubic1[i]-cubicMean[1],2);
        cubicSqrSum[2] +=  pow(cubic2[i]-cubicMean[2],2);
        cubicSqrSum[3] +=  pow(cubic3[i]-cubicMean[3],2);
    }
    linearSqrSum[0]/=CCs.size();
    linearSqrSum[1]/=CCs.size();
    quadraticSqrSum[0]/=CCs.size();
    quadraticSqrSum[1]/=CCs.size();
    quadraticSqrSum[2]/=CCs.size();
    cubicSqrSum[0]/=CCs.size();
    cubicSqrSum[1]/=CCs.size();
    cubicSqrSum[2]/=CCs.size();
    cubicSqrSum[3]/=CCs.size();
    
    double linear[2];
    double quadratic[3];
    double cubic[4];
    linear[0]=pow(linearSqrSum[0],.5);
    linear[1]=pow(linearSqrSum[1],.5);
    quadratic[0]=pow(quadraticSqrSum[0],.5);
    quadratic[1]=pow(quadraticSqrSum[1],.5);
    quadratic[2]=pow(quadraticSqrSum[2],.5);
    cubic[0]=pow(cubicSqrSum[0],.5);
    cubic[1]=pow(cubicSqrSum[1],.5);
    cubic[2]=pow(cubicSqrSum[2],.5);
    cubic[3]=pow(cubicSqrSum[3],.5);
    
//    printf("avg l:(%f,%f)\tq:(%f,%f,%f)\tc:(%f,%f,%f,%f)\n",linearMean[0],linearMean[1],quadraticMean[0],quadraticMean[1],quadraticMean[2],cubicMean[0],cubicMean[1],cubicMean[2],cubicMean[3]);
//    printf("std l:(%f,%f)\tq:(%f,%f,%f)\tc:(%f,%f,%f,%f)\n",linear[0],linear[1],quadratic[0],quadratic[1],quadratic[2],cubic[0],cubic[1],cubic[2],cubic[3]);
}

QVector<QPoint>* getConnectedComponent(const QImage &image, BImage &mark, QRgb color, int startX, int startY)
{
    QVector<QPoint>* ret = new QVector<QPoint>();
    QPoint start(startX,startY);
    QVector<QPoint> stack;
    stack.push_back(start);
    mark.setPixel(startX,startY,false);
    ret->append(start);
    while (!stack.empty())
    {
        QPoint cur = stack.front();
        stack.pop_front();
        
        for (int tableIndex=0; tableIndex<9; tableIndex++)
        {
            if (tableIndex==4)
                continue;
            
            int xDelta=(tableIndex%3)-1;
            int yDelta=(tableIndex/3)-1;
            int x = cur.x()+xDelta;
            int y = cur.y()+yDelta;
            if (x>=0 && x<mark.width() && y>=0 && y<mark.height())
            {
                if (mark.pixel(x,y) && image.pixel(x,y)==color)
                {
                    QPoint p(x,y);
                    stack.push_back(p);
                    mark.setPixel(p,false);
                    ret->append(p);
                }
            }
        }
    }
    return ret;
}

int main(int argc, char *argv[])
{
//    QVector<QVector<QPoint>* >* topCCs = new QVector<QVector<QPoint>* >();
//    QVector<QVector<QPoint>* >* bottomCCs = new QVector<QVector<QPoint>* >();
    
    for (int i=1; i<argc; i++)
    {
        QVector<QVector<QPoint>* >* topCCs = new QVector<QVector<QPoint>* >();
        QVector<QVector<QPoint>* >* bottomCCs = new QVector<QVector<QPoint>* >();
        
        QString fileName(argv[i]);
        QImage image(fileName);
        BImage mark(image.width(),image.height());
        for (int y=0; y<image.height(); y++)
            for (int x=0; x<image.width(); x++)
                mark.setPixel(x,y,true);
        QRgb top = image.pixel(0,0);
        QRgb bottom = image.pixel(1,0);
        
        
        
        for (int y=1; y<image.height(); y++)
        {
            for (int x=image.width()-1; x>=0; x--)
            {
                if (image.pixel(x,y)==top && mark.pixel(x,y))
                {
                    QVector<QPoint>* cc = getConnectedComponent(image,mark,top,x,y);
                    topCCs->append(cc);
                }
                else if (image.pixel(x,y)==bottom && mark.pixel(x,y))
                {
                    QVector<QPoint>* cc = getConnectedComponent(image,mark,bottom,x,y);
                    bottomCCs->append(cc);
                }
            }
        }
        printf("Sheet %d\n", i);
//        printf("top\n");
        evalCCs(*topCCs,true);
//        printf("bottom\n");
        evalCCs(*bottomCCs,false);
        foreach (QVector<QPoint>* cc, (*topCCs)+(*bottomCCs))
            delete cc;
    }
    
//    printf("top\n");
//    evalCCs(*topCCs);
//    printf("bottom\n");
//    evalCCs(*bottomCCs);
    
//    foreach (QVector<QPoint>* cc, (*topCCs)+(*bottomCCs))
//        delete cc;
    return 0;
}
