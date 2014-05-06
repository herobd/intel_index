#include "wordseparator.h"
#include <limits>   
#include <math.h>

#define INT_POS_INFINITY std::numeric_limits<int>::max()
#define INT_NEG_INFINITY std::numeric_limits<int>::min()
#define DOUBLE_POS_INFINITY std::numeric_limits<double>::max()
#define DOUBLE_NEG_INFINITY std::numeric_limits<double>::min()

#define BLACK 0
#define WHITE 255

WordSeparator::WordSeparator()
{
}

QImage WordSeparator::removeFirstWord(QImage &from)
{
    WordProfile profile(from,true,0);
    QVector<int> mins = profile.getLocalMins();
    /*
      Approaches:
      1. It about in the middle, so sut at middle local min
      2. It's where whitespace is so find the min thats 0.
      3. The mins for a single word will cluster tighter together

      */

    //int selectedMin = 0;
    int selectedCut;

    QVector<int> zeros;
    QVector<int> values;
    int sum=0;
    for (int i=0; i<mins.size(); i++)
    {
        values.append(profile.getValue(mins[i]));
        sum += profile.getValue(mins[i]);
        //printf("val: %d\n",profile.getValue(mins[i]));

        if (profile.getValue(mins[i])==0)
                zeros.append(mins[i]);
    }
    if (zeros.size() > 0)
    {
        int min = abs((from.width()/2)-zeros[0]);
        selectedCut = zeros[0];
        for (int i=1; i<zeros.size(); i++)
        {
            if (abs((from.width()/2)-zeros[i]) < min)
            {
                min = abs((from.width()/2)-zeros[i]);
                selectedCut = zeros[i];
            }
        }
    }
    else
    {
        selectedCut=windowScanWidestMin(profile,5);
        /*double avg = sum/(double)mins.size();
        //I don't know if this is really the best way to go about this
        int min = abs((from.width()/2)-mins[0]);
        selectedCut = mins[0];
        for (int i=1; i<mins.size(); i++)
        {
            if (values[i] <= avg && abs((from.width()/2)-mins[i]) < min)
            {
                min = abs((from.width()/2)-mins[i]);
                selectedCut = mins[i];
            }
        }*/
    }

    printf("Cut at %d\n",selectedCut);
    QImage toReturn = from.copy(0,0,selectedCut,from.height());
    from = from.copy(selectedCut,0,from.width()-selectedCut,from.height());
    return toReturn;
}

int WordSeparator::windowScanWidestMin(WordProfile &profile, int size)
{
    QVector<int> mins = profile.getLocalMins();
    int minSum = std::numeric_limits<int>::max();
    int index = 0;
    foreach (int min, mins)
    {
        int lastLocalSum = std::numeric_limits<int>::max();
        for (int i=min; i>=0; i--)
        {
            int sum = 0;
            for (int j=0; j<size; j++)
                sum +=profile.getValue(i+j);
            if (sum < minSum)
            {
                minSum = sum;
                index = i;
            }
            if (sum > lastLocalSum)
                break;
            lastLocalSum=sum;
        }
    }
    return index + size/2;
}

QImage WordSeparator::trimBoundaries(QImage &img)
{   
    int trimTop=0;
    int trimBottom=img.height();
    int trimLeft=0;
    int trimRight=img.width();
    
    int PROFILE_HORZ_THRESH = img.width()*.65;
    int RUN_HORZ_THRESH = img.width()*.35;
    int PROFILE_HORZ_THRESH_E = img.width()*.8;
    int RUN_HORZ_THRESH_E = img.width()*.5;
    int PROFILE_VERT_THRESH = img.height()*.85;
    int RUN_VERT_THRESH = img.height()*.7;
    int i;
    int j;
    bool cont = true;
    
    
//    //top
//    for (j=0; j<img.height()/2 && cont; j++)
//    {
//        cont = false;
//        int profile = 0;
//        for (int i=0; i<img.width(); i++)
//        {
//            if (qGray(img.pixel(i,j)) == BLACK)
//                profile++;
//        }
//        if (profile > PROFILE_HORZ_THRESH)
//            cont = true;
//    }
//    trimTop = j-1;
    
//    //bottom
//    cont = true;
//    for (j=img.height()-1; j>img.height()/2 && cont; j--)
//    {
//        cont = false;
//        int profile = 0;
//        for (int i=0; i<img.width(); i++)
//        {
//            if (qGray(img.pixel(i,j)) == BLACK)
//                profile++;
//        }
//        if (profile > PROFILE_HORZ_THRESH)
//            cont = true;
//    }
//    trimBottom = j+2;
    
//    int i;
//    //left
//    cont = true;
//    for (i=0; i<img.width()/2 && cont; i++)
//    {
//        cont = false;
//        int profile = 0;
//        for (int j=0; j<img.height(); j++)
//        {
//            if (qGray(img.pixel(i,j)) == BLACK)
//                profile++;
//        }
//        if (profile > PROFILE_VERT_THRESH)
//            cont = true;
//    }
//    trimLeft = i-1;
    
    
//    //right
//    cont = true;
//    for (i=img.width()-1; i>img.width()/2 && cont; i--)
//    {
//        cont = false;
//        int profile = 0;
//        for (int j=0; j<img.height(); j++)
//        {
//            if (qGray(img.pixel(i,j)) == BLACK)
//                profile++;
//        }
//        if (profile > PROFILE_VERT_THRESH)
//            cont = true;
//    }
//    trimRight = i+1;
    
     QImage ret = img.copy(trimLeft,trimTop,trimRight-trimLeft,trimBottom-trimTop);
     
     //veritcle
     cont=true;
     for (i=0; cont && i<ret.width()/2; i++)
     {
         if (i>2)
            cont=false;
         int profile = 0;
         int runLength=0;
         for (j=0; j<ret.height(); j++)
         {
             if (qGray(ret.pixel(i,j)) == BLACK)
             {
                 profile++;
                 runLength++;
             }
             else 
             {
                 if (runLength>RUN_VERT_THRESH)
                 {
                     cont=true;
                     for (;runLength>0; runLength--)
                         ret.setPixel(i,j-runLength,WHITE);
                 }
                 runLength=0;
             }
         }
         if (runLength>RUN_VERT_THRESH)
         {
             cont=true;
             for (;runLength>0; runLength--)
                 ret.setPixel(i,j-runLength,WHITE);
         }
         if (profile > PROFILE_VERT_THRESH)
         {
             cont=true;
             for (int j=0; j<ret.height(); j++)
                 ret.setPixel(i,j,WHITE);
         }
     }
     
     cont=true;
     for (i=ret.width()-1; cont && i>ret.width()/2; i--)
     {
         if (i>2)
            cont=false;
         int profile = 0;
         int runLength=0;
         for (j=0; j<ret.height(); j++)
         {
             if (qGray(ret.pixel(i,j)) == BLACK)
             {
                 profile++;
                 runLength++;
             }
             else 
             {
                 if (runLength>RUN_VERT_THRESH)
                 {
                     cont=true;
                     for (;runLength>0; runLength--)
                         ret.setPixel(i,j-runLength,WHITE);
                 }
                 runLength=0;
             }
         }
         if (runLength>RUN_VERT_THRESH)
         {
             cont=true;
             for (;runLength>0; runLength--)
                 ret.setPixel(i,j-runLength,WHITE);
         }
         
         if (profile > PROFILE_VERT_THRESH)
         {
             cont=true;
             for (int j=0; j<ret.height(); j++)
                 ret.setPixel(i,j,WHITE);
         }
     }
     
     //horizontal
     //left
     cont = true;
     for (j=0; cont && j<ret.height()/2; j++)
     {
         cont=false;
         int profile = 0;
         int runLength=0;
         for (i=0; i<ret.width(); i++)
         {
             if (qGray(ret.pixel(i,j)) == BLACK)
             {
                 profile++;
                 runLength++;
             }
             else 
             {
                 if (runLength>RUN_HORZ_THRESH)
                 {
                     cont=true;
                     for (;runLength>0; runLength--)
                         ret.setPixel(i-runLength,j,WHITE);
                 }
                 runLength=0;
             }
         }
         if (runLength>RUN_HORZ_THRESH)
         {
             cont=true;
             for (;runLength>0; runLength--)
                 ret.setPixel(i-runLength,j,WHITE);
         }
         
         if (profile > PROFILE_HORZ_THRESH)
         {
             cont=true;
             for (int i=0; i<ret.width(); i++)
                 ret.setPixel(i,j,WHITE);
         }
     }
     //right
     cont = true;
     for (j=ret.height()-1; cont && j>ret.height()/2; j--)
     {
         cont=false;
         int profile = 0;
         int runLength=0;
         for (i=0; i<ret.width(); i++)
         {
             if (qGray(ret.pixel(i,j)) == BLACK)
             {
                 profile++;
                 runLength++;
             }
             else 
             {
                 if (runLength>RUN_HORZ_THRESH)
                 {
                     cont=true;
                     for (;runLength>0; runLength--)
                         ret.setPixel(i-runLength,j,WHITE);
                 }
                 runLength=0;
             }
         }
         if (runLength>RUN_HORZ_THRESH)
         {
             cont=true;
             for (;runLength>0; runLength--)
                 ret.setPixel(i-runLength,j,WHITE);
         }
         
         if (profile > PROFILE_HORZ_THRESH)
         {
             cont=true;
             for (int i=0; i<ret.width(); i++)
                 ret.setPixel(i,j,WHITE);
         }
     }
     
     //EVERYWHERE
     for (j=0; j<ret.height(); j++)
     {
         int profile = 0;
         int runLength=0;
         for (i=0; i<ret.width(); i++)
         {
             if (qGray(ret.pixel(i,j)) == BLACK)
             {
                 profile++;
                 runLength++;
             }
             else 
             {
                 if (runLength>RUN_HORZ_THRESH_E)
                 {
                     for (;runLength>0; runLength--)
                         ret.setPixel(i-runLength,j,WHITE);
                 }
                 runLength=0;
             }
         }
         if (runLength>RUN_HORZ_THRESH_E)
         {
             for (;runLength>0; runLength--)
                 ret.setPixel(i-runLength,j,WHITE);
         }
         
         if (profile > PROFILE_HORZ_THRESH_E)
         {
             for (int i=0; i<ret.width(); i++)
                 ret.setPixel(i,j,WHITE);
         }
     }
     
    
     
     
    
//    int LINE_THRESH = 25;
    
//    //top
//    int currentRun = 0;
//    cont = true;
//    for (j=0; j<3 && cont; j++)
//    {
//        cont = false;
//        for (int i=0; i<ret.width(); i++)
//        {
//            if (qGray(ret.pixel(i,j)) != BLACK)
//            {
//                    if (currentRun > 0)
//                    {
//                        if (currentRun > LINE_THRESH)
//                        {   
//                            cont = true;
//                            for (int ri=1; ri<currentRun+1; ri++)
//                            {
//                                ret.setPixel(i-ri,j,WHITE);
//                            }
//                        }
                        
//                        currentRun=0;
//                    }
//            }
//            else
//            {
//                currentRun++;
//            }
//        }
//        if (currentRun > 0)
//        {
//            if (currentRun > LINE_THRESH)
//            {   
//                for (int ri=1; ri<currentRun+1; ri++)
//                {
//                    ret.setPixel(ret.width()-ri,j,WHITE);
//                }
//            }
            
//            currentRun=0;
//        }
//    }

    
//    //bottom
//    currentRun=0;
//    cont = true;
//    for (j=ret.height()-1; j>ret.height()-4 && cont; j--)
//    {
//        cont = false;
//        for (int i=0; i<ret.width(); i++)
//        {
//            if (qGray(ret.pixel(i,j)) != BLACK)
//            {
//                    if (currentRun > 0)
//                    {
//                        if (currentRun > LINE_THRESH)
//                        {   
//                            cont = true;
//                            for (int ri=1; ri<currentRun+1; ri++)
//                            {
//                                ret.setPixel(i-ri,j,WHITE);
//                            }
//                        }
                        
//                        currentRun=0;
//                    }
//            }
//            else
//            {
//                currentRun++;
//            }
//        }
//        if (currentRun > 0)
//        {
//            if (currentRun > LINE_THRESH)
//            {   
//                for (int ri=1; ri<currentRun+1; ri++)
//                {
//                    ret.setPixel(ret.width()-ri,j,WHITE);
//                }
//            }
            
//            currentRun=0;
//        }
//    }
    
//    //left
//    currentRun = 0;
//    cont = true;
//    for (i=0; i<3 && cont; i++)
//    {
//        cont = false;
//        for (int j=0; j<ret.height(); j++)
//        {
//            if (qGray(ret.pixel(i,j)) != BLACK)
//            {
//                    if (currentRun > 0)
//                    {
//                        if (currentRun > LINE_THRESH)
//                        {   
//                            cont = true;
//                            for (int rj=1; rj<currentRun+1; rj++)
//                            {
//                                ret.setPixel(i,j-rj,WHITE);
//                            }
//                        }
                        
//                        currentRun=0;
//                    }
//            }
//            else
//            {
//                currentRun++;
//            }
//        }
//        if (currentRun > 0)
//        {
//            if (currentRun > LINE_THRESH)
//            {   
//                for (int rj=1; rj<currentRun+1; rj++)
//                {
//                    ret.setPixel(i,ret.height()-rj,WHITE);
//                }
//            }
            
//            currentRun=0;
//        }
//    }
    
//    //left
//    currentRun = 0;
//    cont = true;
//    for (i=ret.width()-1; i>ret.width()-4 && cont; i--)
//    {
//        cont = false;
//        for (int j=0; j<ret.height(); j++)
//        {
//            if (qGray(ret.pixel(i,j)) != BLACK)
//            {
//                    if (currentRun > 0)
//                    {
//                        if (currentRun > LINE_THRESH)
//                        {   
//                            cont = true;
//                            for (int rj=1; rj<currentRun+1; rj++)
//                            {
//                                ret.setPixel(i,j-rj,WHITE);
//                            }
//                        }
                        
//                        currentRun=0;
//                    }
//            }
//            else
//            {
//                currentRun++;
//            }
//        }
//        if (currentRun > 0)
//        {
//            if (currentRun > LINE_THRESH)
//            {   
//                for (int rj=1; rj<currentRun+1; rj++)
//                {
//                    ret.setPixel(i,ret.height()-rj,WHITE);
//                }
//            }
            
//            currentRun=0;
//        }
//    }
    
    
    
    return ret;
    //return img.copy(trimLeft,trimTop,img.width()-(trimLeft+trimRight),img.height()-(trimTop+trimBottom));
}

QImage WordSeparator::removePixelNoise(QImage &img)
{
    int NOISE_BUFF = 6;
    
    QImage ret = img.copy(0,0,img.width(),img.height());
    
    //corners
    if (qGray(ret.pixel(0,1)) != BLACK && qGray(ret.pixel(1,0)) != BLACK && qGray(ret.pixel(1,1)) != BLACK)
        ret.setPixel(0,0,WHITE);
    if (qGray(ret.pixel(0,ret.height()-2)) != BLACK && qGray(ret.pixel(1,ret.height()-1)) != BLACK && qGray(ret.pixel(1,ret.height()-2)) != BLACK)
        ret.setPixel(0,ret.height()-1,WHITE);
    if (qGray(ret.pixel(ret.width()-1,1)) != BLACK && qGray(ret.pixel(ret.width()-2,0)) != BLACK && qGray(ret.pixel(ret.width()-2,1)) != BLACK)
        ret.setPixel(ret.width()-1,0,WHITE);
    if (qGray(ret.pixel(ret.width()-1,ret.height()-2)) != BLACK && qGray(ret.pixel(ret.width()-2,ret.height()-1)) != BLACK && qGray(ret.pixel(ret.width()-2,ret.height()-2)) != BLACK)
        ret.setPixel(ret.width()-1,ret.height()-1,WHITE);
    
    
    //top and bottom
    for (int i=1; i<ret.width()-1; i++)
    {
        if (qGray(ret.pixel(i,0)) == BLACK && 
                qGray(ret.pixel(i-1,1)) != BLACK && qGray(ret.pixel(i,1)) != BLACK && qGray(ret.pixel(i+1,1)) != BLACK)
            ret.setPixel(i,0,WHITE);
        if (qGray(ret.pixel(i,img.height()-1)) == BLACK && 
                qGray(ret.pixel(i-1,img.height()-2)) != BLACK && qGray(ret.pixel(i,img.height()-2)) != BLACK && qGray(ret.pixel(i+1,img.height()-2)) != BLACK)
            ret.setPixel(i,img.height()-1,WHITE);
    }
    
    for (int j=1; j<ret.height()-1; j++)
    {
        for (int i=1; i<ret.width()-1; i++)
        {
            if (qGray(ret.pixel(i,j)) == BLACK && 
                    qGray(ret.pixel(i-1,j+1)) != BLACK && qGray(ret.pixel(i,j+1)) != BLACK && qGray(ret.pixel(i+1,j+1)) != BLACK && 
                    qGray(ret.pixel(i-1,j-1)) != BLACK && qGray(ret.pixel(i,j-1)) != BLACK && qGray(ret.pixel(i+1,j-1)) != BLACK)
                ret.setPixel(i,j,WHITE);
        }
    }
    
    //left and right
    for (int j=1; j<ret.height()-1; j++)
    {
        if (qGray(ret.pixel(0,j)) == BLACK && qGray(ret.pixel(1,j-1)) != BLACK && qGray(ret.pixel(1,j)) != BLACK && qGray(ret.pixel(1,j+1)) != BLACK)
            ret.setPixel(0,j,WHITE);
        if (qGray(ret.pixel(img.width()-1,j)) == BLACK && qGray(ret.pixel(img.width()-2,j-1)) != BLACK && qGray(ret.pixel(img.width()-2,j)) != BLACK && qGray(ret.pixel(img.width()-2,j+1)) != BLACK)
            ret.setPixel(img.width()-1,j,WHITE);
    }
    
    for (int i=1; i<ret.width()-1; i++)
    {
        for (int j=1; j<ret.height()-1; j++)
        {
            if (qGray(ret.pixel(i,j)) == BLACK && 
                    qGray(ret.pixel(i+1,j-1)) != BLACK && qGray(ret.pixel(i+1,j)) != BLACK && qGray(ret.pixel(i+1,j+1)) != BLACK && 
                    qGray(ret.pixel(i-1,j-1)) != BLACK && qGray(ret.pixel(i-1,j)) != BLACK && qGray(ret.pixel(i-1,j+1)) != BLACK)
                ret.setPixel(i,j,WHITE);
        }
    }
    
    
    //Flood fill filter
    int BLOB_THRESH = 45;
    QImage mark = ret.copy(0,0,ret.width(),ret.height());
    QVector<QPoint> workingStack;
    QVector<QPoint> toClearStack;
    
    for (int i=0; i<mark.width(); i++)
    {
        for (int j=0; j<mark.height(); j++)
        {
            if (qGray(mark.pixel(i,j)) == BLACK)
            {
                int num=0;
                QPoint p(i,j);
                workingStack.push_back(p);
                while (!workingStack.isEmpty())
                {   
                    QPoint cur = workingStack.back();
                    workingStack.pop_back();
                    toClearStack.push_back(cur);
                    
                    mark.setPixel(cur,WHITE);
                    num++;
                    if (cur.x()<mark.width()-1 && qGray(mark.pixel(cur.x()+1,cur.y())) == BLACK)
                    {
                        QPoint pp(cur.x()+1,cur.y());
                        workingStack.push_back(pp);
                    }
                    if (cur.y()<mark.height()-1 && qGray(mark.pixel(cur.x(),cur.y()+1)) == BLACK)
                    {
                        QPoint pp(cur.x(),cur.y()+1);
                        workingStack.push_back(pp);
                    }
                    if (cur.x()>0 && qGray(mark.pixel(cur.x()-1,cur.y())) == BLACK)
                    {
                        QPoint pp(cur.x()-1,cur.y());
                        workingStack.push_back(pp);
                    }
                    if (cur.y()>0 && qGray(mark.pixel(cur.x(),cur.y()-1)) == BLACK)
                    {
                        QPoint pp(cur.x(),cur.y()-1);
                        workingStack.push_back(pp);
                    }
                    //diagonals
                    if (cur.x()<mark.width()-1 && cur.y()<mark.height()-1 && qGray(mark.pixel(cur.x()+1,cur.y()+1)) == BLACK)
                    {
                        QPoint pp(cur.x()+1,cur.y()+1);
                        workingStack.push_back(pp);
                    }
                    if (cur.y()<mark.height()-1 && cur.x()>0 && qGray(mark.pixel(cur.x()-1,cur.y()+1)) == BLACK)
                    {
                        QPoint pp(cur.x()-1,cur.y()+1);
                        workingStack.push_back(pp);
                    }
                    if (cur.x()>0 && cur.y()<mark.height()-1 && qGray(mark.pixel(cur.x()-1,cur.y()+1)) == BLACK)
                    {
                        QPoint pp(cur.x()-1,cur.y()+1);
                        workingStack.push_back(pp);
                    }
                    if (cur.y()>0 && cur.x()>0 && qGray(mark.pixel(cur.x()-1,cur.y()-1)) == BLACK)
                    {
                        QPoint pp(cur.x()-1,cur.y()-1);
                        workingStack.push_back(pp);
                    }
                }
                if (num>BLOB_THRESH)
                {
                    toClearStack.clear();
                }
                
                while (!toClearStack.isEmpty())
                {
                    QPoint r = toClearStack.back();
                    toClearStack.pop_back();
                    
                    //printf("remove p(%d,%d)\n",r.x(),r.y());
                    ret.setPixel(r,WHITE);
                }
            }
        }
    }
    
    return ret;
}

QVector<QImage> WordSeparator::minCut(QImage &img)
{
    int num_pix = img.width()*img.height();
    //double pix_vals[num_pix];
    int invDistMap[num_pix];
    /*int index = 0;
    for (int j=0; j<img.height(); j++)
    {
        for (int i=0; i<img.width(); i++)
        {
            pix_vals[index++]=img.pixel(i,j);
        }
    }*/
    computeInverseDistanceMap(img,invDistMap);
    QVector<int> firstImgBlackPixelIndexes;
    int maxflow = pixelsOfSeparation(invDistMap,img.width(),img.height(),img,firstImgBlackPixelIndexes);
    
    int firstFarthestRightPixel = 0;
    int secondFarthestLeftPixel = 0;
    
    QImage firstImg = img.copy(0,0,img.width(),img.height());
    firstImg.fill(255);
    QImage secondImg = img.copy(0,0,img.width(),img.height());
    
    foreach(int index, firstImgBlackPixelIndexes)
    {
        int x = index%img.width();
        int y = index/img.width();
        firstImg.setPixel(x,y,0);
        secondImg.setPixel(x,y,255);
        
        if (x>firstFarthestRightPixel)
            firstFarthestRightPixel=x;
    }
    bool notFound = true;
    for (int i=0; i<img.width() && notFound; i++)
    {
        for (int j=0; j<img.height() && notFound; j++)
        {
            if (qGray(secondImg.pixel(i,j))==0)
            {
                secondFarthestLeftPixel=i;
                notFound = false;
            }
        }
    }
    
    printf("maxflow= %d, first right border:%d, second left border:%d\n",maxflow,firstFarthestRightPixel,secondFarthestLeftPixel);
    
    firstImg = firstImg.copy(0,0,firstFarthestRightPixel+1,firstImg.height());
    secondImg = secondImg.copy(secondFarthestLeftPixel, 0, secondImg.width()-secondFarthestLeftPixel, secondImg.height());
   
    QVector<QImage> ret;
    ret.append(firstImg);
    ret.append(secondImg);
    return ret;
}

//Meijster distance <http://fab.cba.mit.edu/classes/S62.12/docs/Meijster_distance.pdf>
//This can be parallelized. Should probably flip from column to row first
void WordSeparator::computeInverseDistanceMap(QImage &img, int* out)
{
    int maxDist=0;
    int g[img.width()*img.height()];
    for (int x=0; x<img.width(); x++)
    {
        if (qGray(img.pixel(x,0))==0)
        {
            g[x+0*img.width()]=0;
        }
        else
        {
            g[x+0*img.width()]=INT_POS_INFINITY;//img.width()*img.height();
        }
        
        for (int y=1; y<img.height(); y++)
        {
            if (qGray(img.pixel(x,y))==BLACK)
            {
                g[x+y*img.width()]=0;
            }
            else
            {
                if (g[x+(y-1)*img.width()] != INT_POS_INFINITY)
                    g[x+y*img.width()]=1+g[x+(y-1)*img.width()];
                else
                    g[x+y*img.width()] = INT_POS_INFINITY;
            }
        }
        
        for (int y=img.height()-2; y>=0; y--)
        {
            if (g[x+(y+1)*img.width()]<g[x+y*img.width()])
            {
                if (g[x+(y+1)*img.width()] != INT_POS_INFINITY)
                    g[x+y*img.width()]=1+g[x+(y+1)*img.width()];
                else
                    g[x+y*img.width()] = INT_POS_INFINITY;
            }
        }
        
        /*if(x==20)
        {
            for (int y=0; y<img.height(); y++)
            {
                printf("%d .. %d\n",qGray(img.pixel(x,y)),g[x+y*img.width()]);
            }
        }*/
    }
    
    int q;
    int s[img.width()];
    int t[img.width()];
    int w;
    for (int y=0; y<img.height(); y++)
    {
        q=0;
        s[0]=0;
        t[0]=0;
        for (int u=1; u<img.width();u++)
        {
            while (q>=0 && f(t[q],s[q],y,img.width(),g) > f(t[q],u,y,img.width(),g))
            {
                q--;
            }
            
            if (q<0)
            {
                q=0;
                s[0]=u;
            }
            else
            {
                w = SepPlusOne(s[q],u,y,img.width(),g);
                if (w<img.width())
                {
                    q++;
                    s[q]=u;
                    t[q]=w;
                }
            }
        }
        
        for (int u=img.width()-1; u>=0; u--)
        {
            out[u+y*img.width()]= f(u,s[q],y,img.width(),g);
            if (out[u+y*img.width()] > maxDist)
                maxDist = out[u+y*img.width()];
            if (u==t[q])
                q--;
        }
        
       /* if(y==0 || y==1 || y==2)
        {
            for (int x=260; x<img.width(); x++)
            {
                printf("%d .. %d\n",qGray(img.pixel(x,y)),out[x+y*img.width()]);
            }
            printf("===========-----\n");
        }*/
    }
    
    //invert
    //printf("maxDist=%d\n",maxDist);
    maxDist++;
    double normalizer = (20.0/pow(maxDist,7));
    int newmax = 0;
    for (int q = 0; q < img.width()*img.height(); q++)
    {
        
        /*if (q%img.width()==0)
        {
            printf("\n");
        }
        if (q%img.width() < 20)
        {
            printf ("%d,",out[q]);
        }*/
        
        out[q] = pow(maxDist - out[q],7)*normalizer;
        
        if (out[q]>newmax)
            newmax=out[q];
    }
    
    QImage debug=img.copy(0,0,img.width(),img.height());//(img.width(),img.height(),img.format());
    for (int i=0; i<debug.width(); i++)
    {
        for (int j=0; j<debug.height(); j++)
            debug.setPixel(i,j,(int)(out[i+j*debug.width()]/((double)newmax)*255));
        
    }
    debug.save("./inv_dist_map.pgm");
}

int WordSeparator::f(int x, int i, int y, int m, int* g)
{
    if (g[i+y*m]==INT_POS_INFINITY || x==INT_POS_INFINITY)
        return INT_POS_INFINITY;
    return (x-i)*(x-i) + g[i+y*m]*g[i+y*m];
}

int WordSeparator::SepPlusOne(int i, int u, int y, int m, int* g)
{
    if (g[u+y*m] == INT_POS_INFINITY)// && g[i+y*m] != INT_POS_INFINITY)
    {
        //printf("Sep is inginite\n");
        return INT_POS_INFINITY;
    }
    return 1 + ((u*u)-(i*i)+g[u+y*m]*g[u+y*m]-(g[i+y*m]*g[i+y*m])) / (2*(u-i));
}

/*int G(int i, int y, int n, QImage &img)
{
    int min = INT_POS_INFINITY;
    for (int j=0; j<n; j++)
    {
        int val = abs(y-j);
        if (img.pixel(i,j)==0 && val < min)
        {
            min = val; 
        }
    }
    return min;
}*/

/*void WordSeparator::computeInverseDistanceMap(double* in, double* out, int width, int height)
{
    double maxDist = 0;
    
    for (int )
    
    //invert
    maxDist++;
    for (int q = 0; q < width*height; q++)
    {
        out[q] = maxDist - out[q];
    }
}

//Felzenszwald & Huttenlocher's 1D squared euclidean distance transfrom
double WordSeparator::distanceTransform1D(double* in, double* out, int size)
{
    double maxDist = 0;
    
    int n = size;
    
    int k = 0;
    int v[n];
    double z[n + 1];

    v[0] = 0;
    z[0] = DOUBLE_NEG_INFINITY;
    z[1] = DOUBLE_POS_INFINITY;

    double s;

    for (int q = 1; q < n; q++)
    {
        while (true)
        {
            s = (((in[q] + q * q) - (in[v[k]] + v[k] * v[k])) / (2.0 * q - 2.0 * v[k]));

            if (s <= z[k])
            {
                k--;
            }
            else
            {
                break;
            }
        }

        k++;

        v[k] = q;
        z[k] = s;
        z[k + 1] = DOUBLE_POS_INFINITY;
    }

    k = 0;

    for (int q = 0; q < n; q++)
    {
        while (z[k + 1] < q)
        {
            k++;
        }

        out[q] = ((q - v[k]) * (q - v[k]) + in[v[k]]);
        
        if (out[q] > maxDist)
            maxDist = out[q];
    }
    
    return maxDist;
}*/


//Uses Boykov graph cut
int WordSeparator::pixelsOfSeparation(int* invDistMap, int width, int height, QImage &img, QVector<int> &out)
{
    typedef Graph<int,int,int> GraphType;
    GraphType *g = new GraphType(width*height, 2*(width-1)*(height-1)-(height+width)); 
    
    for (int i=0; i<width*height; i++)
    {
        g->add_node();
    }
    
    float ARCHOR_M = 1.25;
    
    //find source pixels
    bool finish = false;
    int count_source = height*ARCHOR_M;
    for (int i=0; count_source>0 && i<width; i++)
    {
        for (int j=0; count_source>0 && j<height; j++)
        {
            if (qGray(img.pixel(i,j))==BLACK)
            {
                finish=true;
                int index = i+width*j;
                g -> add_tweights(index, INT_POS_INFINITY,0);//invDistMap[index], 0);
                count_source--;
            }
        }
//        if (finish)
//        {
////            for (int j=0; j<height; j++)
////            {
////                int index = i+width*j;
////                g -> add_tweights(index, INT_POS_INFINITY,0);//invDistMap[index], 0);
////            }
//            break;
//        }
    }
    
    int count_sink=height*ARCHOR_M;
    
    //find sink pixels
    finish = false;
    for (int i=width-1; count_sink>0 && i>=0; i--)
    {
        for (int j=height-1; count_sink>0 && j>=0; j--)
        {
            if (qGray(img.pixel(i,j))==0)
            {
                finish=true;
                int index = i+width*j;
                g -> add_tweights(index, 0, INT_POS_INFINITY);//invDistMap[index]);
                count_sink--;
            }
        }
//        if (finish)
//        {
////            for (int j=0; j<height; j++)
////            {
////                int index = i+width*j;
////                g -> add_tweights(index, 0, INT_POS_INFINITY);//invDistMap[index]);
////            }
//            break;
//        }
    }
    
    //printf("num source:%d, num sink:%d\n",count_source,count_sink);
    
    for (int i=0; i<width; i++)
    {
        for (int j=0; j<height; j++)
        {
//            if (i+1<width)
//                g -> add_edge(i+j*width, (i+1)+j*width, invDistMap[(i+1)+j*width], invDistMap[i+j*width]);
//            if (j+1<height)
//                g -> add_edge(i+j*width, i+(j+1)*width, invDistMap[i+(j+1)*width], invDistMap[i+j*width]);
            if (i+1<width)
                g -> add_edge(i+j*width, (i+1)+j*width, (invDistMap[(i+1)+j*width]+invDistMap[i+j*width])/2, (invDistMap[i+j*width]+invDistMap[(i+1)+j*width])/2);
            if (j+1<height)
                g -> add_edge(i+j*width, i+(j+1)*width, (invDistMap[i+(j+1)*width]+invDistMap[i+j*width])/2, (invDistMap[i+j*width]+invDistMap[i+(j+1)*width])/2);
        }
    }
    
    //I don't know if this needs called
    int ret = g -> maxflow();
    
    //add all black pixels which
    for (int index=0; index<width*height; index++)
    {
        if (g->what_segment(index) == GraphType::SOURCE && qGray(img.pixel(index%width,index/width))==BLACK)
        {
            out.append(index);
        }
    }
    delete g;
    return ret;
}

/*
  New seperator algorithm
  Compute distance map, track highest distance for pixel
  create graph
    invert distance map
    add highest distance (to all pixels)
    node for each pixel, weight for arc is average of two (new) distances
  use min cut
  cut is seperation of images
  */

QImage WordSeparator::removeFirstCapitalLetter(QImage &from)
{
    WordProfile profile(from,true,0);
    QVector<int> mins = profile.getLocalMins();
    /*
      Approaches:
      1. It about in the middle, so sut at middle local min
      2. It's where whitespace is so find the min thats 0.
      3. The mins for a single word will cluster tighter together

      */

    //int selectedMin = 0;
    int selectedCut;

    QVector<int> values;
    int sum=0;
    for (int i=0; i<mins.size(); i++)
    {
        values.append(profile.getValue(mins[i]));
        sum += profile.getValue(mins[i]);
    }
    double avg = sum/(double)mins.size();
    //I don't know if this is really the best way to go about this
    int min = abs((from.width()/2)-mins[0]);
    selectedCut = mins[0];
    for (int i=1; i<mins.size(); i++)
    {
        if (values[i] <= avg && abs((from.width()/2)-mins[i]) < min)
        {
            min = abs((from.width()/2)-mins[i]);
            selectedCut = mins[i];
        }
    }
    

    printf("Cut at %d\n",selectedCut);
    QImage toReturn = from.copy(0,0,selectedCut,from.height());
    from = from.copy(selectedCut,0,from.width()-selectedCut,from.height());
    return toReturn;
}

/*?? WordSeparator::findCapitalCandidates(QImage &from)
{
    WordProfile profile(from,true,0);
    QVector<int> mins = profile.getLocalMins();
    //Two params: pixel density and area
    
    
}*/
