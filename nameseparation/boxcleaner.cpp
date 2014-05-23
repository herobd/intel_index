#include "boxcleaner.h"
#include <math.h>
#include <stdio.h>

BoxCleaner::BoxCleaner()
{
}

QImage BoxCleaner::trimBoundaries(QImage &img)
{   
    int trimTop=0;
    int trimBottom=img.height();
    int trimLeft=0;
    int trimRight=img.width();
    
    int PROFILE_HORZ_THRESH = img.width()*.75;
    int RUN_HORZ_THRESH = img.width()*.55;   
    int PROFILE_HORZ_THRESH_E = img.width()*.8;
    int RUN_HORZ_THRESH_E = img.width()*.65;
    int PROFILE_VERT_THRESH = img.height()*.85;
    int RUN_VERT_THRESH = img.height()*.7;
    int i;
    int j;
    bool cont = true;
    
     QImage ret = img.copy(trimLeft,trimTop,trimRight-trimLeft,trimBottom-trimTop);
     
     
     //veritcle lines
     
     //left side
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
     
     //right side
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
     //top
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
//                     for (;runLength>0; runLength--)
//                         ret.setPixel(i-runLength,j,WHITE);
                     cond_clear_line(runLength,i,j,ret);
                 }
                 runLength=0;
             }
         }
         if (runLength>RUN_HORZ_THRESH)
         {
             cont=true;
//             for (;runLength>0; runLength--)
//                 ret.setPixel(i-runLength,j,WHITE);
             cond_clear_line(runLength,i,j,ret);
         }
         
         if (profile > PROFILE_HORZ_THRESH)
         {
             cont=true;
//             for (int i=0; i<ret.width(); i++)
//                 ret.setPixel(i,j,WHITE);
             cond_clear_line(ret.width()-1,ret.width(),j,ret);
         }
     }
     //bottom
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
//                     for (;runLength>0; runLength--)
//                         ret.setPixel(i-runLength,j,WHITE);
                     cond_clear_line(runLength,i,j,ret);
                 }
                 runLength=0;
             }
         }
         if (runLength>RUN_HORZ_THRESH)
         {
             cont=true;
//             for (;runLength>0; runLength--)
//                 ret.setPixel(i-runLength,j,WHITE);
             cond_clear_line(runLength,i,j,ret);
         }
         
         if (profile > PROFILE_HORZ_THRESH)
         {
             cont=true;
//             for (int i=0; i<ret.width(); i++)
//                 ret.setPixel(i,j,WHITE);
             cond_clear_line(ret.width()-1,ret.width(),j,ret);
         }
     }
     
     
     //edge/line filter, this clears horizontal lines, but we only examine the area around the edges
     
     for (j=0; j<9; j++)
     {
         lineFilterAtJ(j,ret);
     }
     for (j=ret.height()-1; j>ret.height()-10; j--)
     {
         lineFilterAtJ(j,ret);
     }
     
     
     //EVERYWHERE
     //If there happend to be any big, obvious lines out of our range before, we remove them
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
//                     for (;runLength>0; runLength--)
//                         ret.setPixel(i-runLength,j,WHITE);
                     cond_clear_line(runLength,i,j,ret);
                 }
                 runLength=0;
             }
         }
         if (runLength>RUN_HORZ_THRESH_E)
         {
//             for (;runLength>0; runLength--)
//                 ret.setPixel(i-runLength,j,WHITE);
             cond_clear_line(runLength,i,j,ret);
         }
         
         if (profile > PROFILE_HORZ_THRESH_E)
         {
//             for (int i=0; i<ret.width(); i++)
//                 ret.setPixel(i,j,WHITE);
             cond_clear_line(ret.width()-1,ret.width(),j,ret);
         }
     }
     
  
    return ret;
}


void BoxCleaner::lineFilterAtJ(int j, QImage &ret)
{
    int FILTER_RUN_HORZ_THRESH = ret.width()*.05;
    int runLength=0;
    int i;
    for (i=0; i<ret.width(); i++)
    {
        if (qGray(ret.pixel(i,j)) == BLACK)
        {
            //this applies a filter that is [-2,-2,1,2,1,-2,-2] vertically
            //this is to identify if we are looking at a piece of a line as opposed to some other shape.
            //If the result is positive, we say it is part of a line (maybe)
            //This resticts the line to be at most 4 pixels thick
            //If enought pixels in a row test positive, we assume it is a line and clear it
            
            int filterSum = 2;
            if (j > 3 && qGray(ret.pixel(i,j-3)) == BLACK)
                filterSum -= 2;
            if (j > 2 && qGray(ret.pixel(i,j-2)) == BLACK)
                filterSum -= 2;
            if (j > 1 && qGray(ret.pixel(i,j-1)) == BLACK)
                filterSum += 1;
            if (j < ret.height()-1 && qGray(ret.pixel(i,j+1)) == BLACK)
                filterSum += 1;
            if (j < ret.height()-2 && qGray(ret.pixel(i,j+2)) == BLACK)
                filterSum -= 2;
            if (j < ret.height()-3 && qGray(ret.pixel(i,j+3)) == BLACK)
                filterSum -= 2;
            
            if (filterSum > 0)
            {
                runLength++;
            }
            else
            {
                if (runLength>FILTER_RUN_HORZ_THRESH)
                {
//                    for (;runLength>0; runLength--)
//                        ret.setPixel(i-runLength,j,WHITE);
                    cond_clear_line(runLength,i,j,ret);
                }
                runLength=0;
            }
        }
        else
        {
            if (runLength>FILTER_RUN_HORZ_THRESH)
            {
                cond_clear_line(runLength,i,j,ret);

            }
            runLength=0;
        }
    }
}

void BoxCleaner::cond_clear_line(int runLength, int i, int j, QImage &ret)
{
    for (;runLength>0; runLength--)
        ret.setPixel(i-runLength,j,WHITE);
    
//    bool dontclearnext = false;
//    for (;runLength>0; runLength--)
//    {
//        int num_pixels_below = 0;
//        for (int counter=0; counter<9; counter++)
//        {
//            if (j+counter<ret.height() && qGray(ret.pixel(i-runLength,j+counter)) == BLACK)
//            {
//                num_pixels_below++;
//            }
//            else
//            {
//                break;
//            }
//        }
//        int num_pixels_above = 0;
//        for (int counter=0; counter<9; counter++)
//        {
//            if (j-counter>=0 && qGray(ret.pixel(i-runLength,j-counter)) == BLACK)
//            {
//                num_pixels_above++;
//            }
//            else
//            {
//                break;
//            }
//        }
////        if (num_pixels_below > 4 || num_pixels_above > 4)
//        if (num_pixels_below + num_pixels_above > 8)
//        {
//            if (i-(runLength+1)<ret.width())
//                ret.setPixel(i-(runLength+1),j,BLACK);
//            dontclearnext=true;
//        }
//        else
//        {
//            if (dontclearnext)
//                dontclearnext=false;
//            else
//                ret.setPixel(i-runLength,j,WHITE);
//        }
//    }
}

QImage BoxCleaner::removePixelNoise(QImage &img)
{
    //int NOISE_BUFF = 6;
    
    QImage ret = img.copy(0,0,img.width(),img.height());
    
    //This first part picks up any stray singel pixel lines that might be floating around
    
    //corners
    if (qGray(ret.pixel(0,1)) != BLACK && 
            qGray(ret.pixel(1,0)) != BLACK && 
            qGray(ret.pixel(1,1)) != BLACK)
        ret.setPixel(0,0,WHITE);
    if (qGray(ret.pixel(0,ret.height()-2)) != BLACK && 
            qGray(ret.pixel(1,ret.height()-1)) != BLACK && 
            qGray(ret.pixel(1,ret.height()-2)) != BLACK)
        ret.setPixel(0,ret.height()-1,WHITE);
    if (qGray(ret.pixel(ret.width()-1,1)) != BLACK && 
            qGray(ret.pixel(ret.width()-2,0)) != BLACK && 
            qGray(ret.pixel(ret.width()-2,1)) != BLACK)
        ret.setPixel(ret.width()-1,0,WHITE);
    if (qGray(ret.pixel(ret.width()-1,ret.height()-2)) != BLACK &&
            qGray(ret.pixel(ret.width()-2,ret.height()-1)) != BLACK &&
            qGray(ret.pixel(ret.width()-2,ret.height()-2)) != BLACK)
        ret.setPixel(ret.width()-1,ret.height()-1,WHITE);
    
    
    //top and bottom
    for (int i=1; i<ret.width()-1; i++)
    {
        if (qGray(ret.pixel(i,0)) == BLACK && 
                qGray(ret.pixel(i-1,1)) != BLACK && 
                qGray(ret.pixel(i,1)) != BLACK && 
                qGray(ret.pixel(i+1,1)) != BLACK)
            ret.setPixel(i,0,WHITE);
        if (qGray(ret.pixel(i,img.height()-1)) == BLACK && 
                qGray(ret.pixel(i-1,img.height()-2)) != BLACK && 
                qGray(ret.pixel(i,img.height()-2)) != BLACK && 
                qGray(ret.pixel(i+1,img.height()-2)) != BLACK)
            ret.setPixel(i,img.height()-1,WHITE);
    }
    
    for (int j=1; j<ret.height()-1; j++)
    {
        for (int i=1; i<ret.width()-1; i++)
        {
            if (qGray(ret.pixel(i,j)) == BLACK && 
                    qGray(ret.pixel(i-1,j+1)) != BLACK && 
                    qGray(ret.pixel(i,j+1)) != BLACK && 
                    qGray(ret.pixel(i+1,j+1)) != BLACK && 
                    qGray(ret.pixel(i-1,j-1)) != BLACK && 
                    qGray(ret.pixel(i,j-1)) != BLACK && 
                    qGray(ret.pixel(i+1,j-1)) != BLACK)
                ret.setPixel(i,j,WHITE);
        }
    }
    
    //left and right
    for (int j=1; j<ret.height()-1; j++)
    {
        if (qGray(ret.pixel(0,j)) == BLACK && 
                qGray(ret.pixel(1,j-1)) != BLACK && 
                qGray(ret.pixel(1,j)) != BLACK && 
                qGray(ret.pixel(1,j+1)) != BLACK)
            ret.setPixel(0,j,WHITE);
        
        if (qGray(ret.pixel(img.width()-1,j)) == BLACK && 
                qGray(ret.pixel(img.width()-2,j-1)) != BLACK && 
                qGray(ret.pixel(img.width()-2,j)) != BLACK && 
                qGray(ret.pixel(img.width()-2,j+1)) != BLACK)
            ret.setPixel(img.width()-1,j,WHITE);
    }
    
    for (int i=1; i<ret.width()-1; i++)
    {
        for (int j=1; j<ret.height()-1; j++)
        {
            if (qGray(ret.pixel(i,j)) == BLACK && 
                    qGray(ret.pixel(i+1,j-1)) != BLACK && 
                    qGray(ret.pixel(i+1,j)) != BLACK && 
                    qGray(ret.pixel(i+1,j+1)) != BLACK && 
                    qGray(ret.pixel(i-1,j-1)) != BLACK && 
                    qGray(ret.pixel(i-1,j)) != BLACK && 
                    qGray(ret.pixel(i-1,j+1)) != BLACK)
                ret.setPixel(i,j,WHITE);
        }
    }
    
    
    //Now we remove any connected components that are below a certain size.
    int BLOB_THRESH = 35;
    int HORZ_MARK_THRESH = 900;
    int LONGNESS_RATIO = 2.4;
    int STD_DEV_LIMIT = 38;
    blobFilter(ret,0,ret.height()-1,BLOB_THRESH,HORZ_MARK_THRESH,LONGNESS_RATIO,STD_DEV_LIMIT);
    
    return ret;
}

//
//est_y is an estimate of where the horizontal dividing line is.
//vert_divide will be set to the actual center of the dividing line.
//crossPoints will be set to the middle point of every crossing of the removed dividing line that is
//restored by the close.
QImage BoxCleaner::clearLineAndCloseLetters(QImage &src, int est_y, int* vert_divide, QVector<QPoint>* crossPoints)
{
    int SEARCH_BAND = 10;
    int STRUCT_ELE_SIZE = 6;
    int LINE_THRESH = src.width() * .6;
    
    double STRUCT_ELE_CORNER = 1.5*STRUCT_ELE_SIZE;
    
    QImage ret = src.copy(0,0,src.width(),src.height());
    if (src.height()-est_y>SEARCH_BAND && est_y>SEARCH_BAND)
    {
        QVector<int> profile(SEARCH_BAND*2 + 1);
        bool erasing = false;
        QVector<QPoint> pointsToClose;
        int aboveLine;
        int belowLine;
        int maxProfile=0;
        int maxProfileIndex=0;
        for (int j=0; j<=SEARCH_BAND*2; j++)
        {
            //create profile for row
            for (int i=0; i<src.width(); i++)
            {
                if (qGray(ret.pixel(i,j+(est_y-SEARCH_BAND))) == BLACK)
                    profile[j]++;
            }
            if (profile[j]>maxProfile)
            {
                maxProfile=profile[j];
                maxProfileIndex=j;
            }
        }
        aboveLine = maxProfileIndex-1;
        while (profile[aboveLine]>LINE_THRESH)
            aboveLine--;
        aboveLine+=(est_y-SEARCH_BAND);
        
        belowLine = maxProfileIndex+1;
        while (profile[belowLine]>LINE_THRESH)
            belowLine++;
        belowLine+=(est_y-SEARCH_BAND);
        
        
        for (int i=0; i<src.width(); i++)
        {
            if (qGray(ret.pixel(i,aboveLine)) == BLACK && 
                    (i<=0 || qGray(ret.pixel(i-1,aboveLine-1)) != BLACK) && 
                    qGray(ret.pixel(i,aboveLine-1)) != BLACK && 
                    (i>=src.width()-1 || qGray(ret.pixel(i+1,aboveLine-1)) != BLACK))
            {
                ret.setPixel(i,aboveLine,WHITE);
            }
            else if (qGray(ret.pixel(i,aboveLine-1)) == BLACK && 
                    qGray(ret.pixel(i,aboveLine)) == BLACK)
            {
                QPoint p(i,aboveLine);
                pointsToClose.append(p);
            }
            ret.setPixel(i,aboveLine+1,WHITE);
        }
            
        
        for (int j=aboveLine+2; j<belowLine; j++)
        {
            for (int i=0; i<src.width(); i++)
            {
                ret.setPixel(i,j,WHITE);
            }
        }
        
        
        for (int i=0; i<src.width(); i++)
        {
            if (qGray(ret.pixel(i,belowLine)) == BLACK && 
                    (i<=0 || qGray(ret.pixel(i-1,belowLine+1)) != BLACK) && 
                    qGray(ret.pixel(i,belowLine+1)) != BLACK && 
                    (i>=src.width()-1 || qGray(ret.pixel(i+1,belowLine+1)) != BLACK))
            {
                ret.setPixel(i,belowLine,WHITE);
            }
            else if (qGray(src.pixel(i,belowLine+1)) == BLACK && 
                    qGray(src.pixel(i,belowLine)) == BLACK)
            {
                QPoint p(i,belowLine);
                pointsToClose.append(p);
            }
            ret.setPixel(i,aboveLine+1,WHITE);
        }
            
        if (vert_divide != NULL)
            *vert_divide = (belowLine+aboveLine)/2;
        
        //Flood fill filter
        int BLOB_THRESH = 8;
        int HORZ_MARK_THRESH = 100;
        int LONGNESS_RATIO = 3;
        int STD_DEV_LIMIT = 38;
        blobFilter(ret,aboveLine-1,belowLine+1,BLOB_THRESH,HORZ_MARK_THRESH,LONGNESS_RATIO,STD_DEV_LIMIT);
        
        
        
        //Morphological close
        
        QVector<QPoint> pointsToErrode;
        QImage close_tmp = ret.copy(0,0,ret.width(),ret.height());
        //dialate
        foreach (QPoint p, pointsToClose)
        {
            if (qGray(ret.pixel(p.x(),p.y())) != BLACK)
                continue;
            
            for (int ri=-STRUCT_ELE_SIZE; ri<=STRUCT_ELE_SIZE; ri++)
            {
                for (int rj=-STRUCT_ELE_SIZE; rj<=STRUCT_ELE_SIZE; rj++)
                {
                    if (abs(ri)+abs(rj)<STRUCT_ELE_CORNER)
                    {
                        if (ri+p.x()>=0 && ri+p.x()<ret.width() &&
                                rj+p.y()>=0 && rj+p.y()<ret.height())
                        {
                            QPoint toBlack(ri+p.x(),rj+p.y());
                            close_tmp.setPixel(toBlack,BLACK);
                            pointsToErrode.append(toBlack);
                        }
                    }
                }
            }
        }
        
        //errode
        foreach (QPoint p, pointsToErrode)
        {
            bool allBlack = true;
            for (int ri=-STRUCT_ELE_SIZE; ri<=STRUCT_ELE_SIZE && allBlack; ri++)
            {
                for (int rj=-STRUCT_ELE_SIZE; rj<=STRUCT_ELE_SIZE; rj++)
                {
                    if (abs(ri)+abs(rj)<STRUCT_ELE_CORNER)
                    {
                        if (ri+p.x()>=0 && ri+p.x()<ret.width() &&
                                rj+p.y()>=0 && rj+p.y()<ret.height())
                        {
                            if (qGray(close_tmp.pixel(p.x()+ri,p.y()+rj)) != BLACK)
                            {
                                    allBlack=false;
                                    break;
                            }
                        }
                    }
                }
            }
            if (allBlack)
            {
                ret.setPixel(p,BLACK);
            }
//            else
//            {
//                ret.setPixel(p,WHITE);
//            }
        }
        
    }
    
    if (crossPoints != NULL)
    {
        bool onLine = false;
        int lineStart = -1;
        for (int i=0; i<ret.width(); i++)
        {
            if (qGray(ret.pixel(i,*vert_divide)) == BLACK)
            {
                if (!onLine)
                {
                    onLine=true;
                    lineStart=i;
                }
            }
            else if (onLine)
            {
                onLine=false;
                QPoint keypoint((lineStart+i)/2,*vert_divide);
                crossPoints->append(keypoint);
            }
        }
        if (onLine)
        {
            QPoint keypoint((lineStart+ret.width())/2,*vert_divide);
            crossPoints->append(keypoint);
        }
    }
    
    
    return ret;
}

//Delete any connected components below the threshhold limit: blobThresh
//If the connected components appears to be a line (speficied by width/height ratio: horzLonRatio
//and standard deviation limit: stdDevLimit) we allow a larger threshold: horzMarkThresh
void BoxCleaner::blobFilter(QImage &on, int fromY, int toY, int blobThresh, int horzMarkThresh, double horzLongRatio, double stdDevLimit)
{
    
    
    QImage mark = on.copy(0,0,on.width(),on.height());
    QVector<QPoint> workingStack;
    QVector<QPoint> toClearStack;
    
    for (int j=fromY; j<=toY; j++)
    {
        for (int i=0; i<mark.width(); i++)
        {
            if (qGray(mark.pixel(i,j)) == BLACK)
            {
                int num=0;
                QPoint p(i,j);
                workingStack.push_back(p);
                mark.setPixel(p,WHITE);
                int min_x, min_y, max_x, max_y;
                min_x = min_y = INT_POS_INFINITY;
                max_x = max_y = 0;
                while (!workingStack.isEmpty())
                {   
                    QPoint cur = workingStack.back();
                    workingStack.pop_back();
                    toClearStack.push_back(cur);
                    
                    num++;
                    if (cur.x()<min_x)
                        min_x=cur.x();
                    if (cur.y()<min_y)
                        min_y=cur.y();
                    if (cur.x()>max_x)
                        max_x=cur.x();
                    if (cur.y()>max_y)
                        max_y=cur.y();
                    
                    if (cur.x()<mark.width()-1 && qGray(mark.pixel(cur.x()+1,cur.y())) == BLACK)
                    {
                        QPoint pp(cur.x()+1,cur.y());
                        workingStack.push_back(pp);
                        mark.setPixel(pp,WHITE);
                    }
                    if (cur.y()<mark.height()-1 && qGray(mark.pixel(cur.x(),cur.y()+1)) == BLACK)
                    {
                        QPoint pp(cur.x(),cur.y()+1);
                        workingStack.push_back(pp);
                        mark.setPixel(pp,WHITE);
                    }
                    if (cur.x()>0 && qGray(mark.pixel(cur.x()-1,cur.y())) == BLACK)
                    {
                        QPoint pp(cur.x()-1,cur.y());
                        workingStack.push_back(pp);
                        mark.setPixel(pp,WHITE);
                    }
                    if (cur.y()>0 && qGray(mark.pixel(cur.x(),cur.y()-1)) == BLACK)
                    {
                        QPoint pp(cur.x(),cur.y()-1);
                        workingStack.push_back(pp);
                        mark.setPixel(pp,WHITE);
                    }
                    //diagonals
                    if (cur.x()<mark.width()-1 && cur.y()<mark.height()-1 && qGray(mark.pixel(cur.x()+1,cur.y()+1)) == BLACK)
                    {
                        QPoint pp(cur.x()+1,cur.y()+1);
                        workingStack.push_back(pp);
                        mark.setPixel(pp,WHITE);
                    }
                    if (cur.y()<mark.height()-1 && cur.x()>0 && qGray(mark.pixel(cur.x()-1,cur.y()+1)) == BLACK)
                    {
                        QPoint pp(cur.x()-1,cur.y()+1);
                        workingStack.push_back(pp);
                        mark.setPixel(pp,WHITE);
                    }
                    if (cur.x()<mark.width()-1 && cur.y()>0 && qGray(mark.pixel(cur.x()+1,cur.y()-1)) == BLACK)
                    {
                        QPoint pp(cur.x()+1,cur.y()-1);
                        workingStack.push_back(pp);
                        mark.setPixel(pp,WHITE);
                    }
                    if (cur.y()>0 && cur.x()>0 && qGray(mark.pixel(cur.x()-1,cur.y()-1)) == BLACK)
                    {
                        QPoint pp(cur.x()-1,cur.y()-1);
                        workingStack.push_back(pp);
                        mark.setPixel(pp,WHITE);
                    }
                }
                if (num>horzMarkThresh)
                {
                    toClearStack.clear();
                }
                else if (num>blobThresh)
                {
                    //check if horizontal mark, in which case we allow it to be bigger
                    QVector<int> v_profile(1+max_y-min_y);
                    v_profile.fill(0);
                    QVector<int> h_profile(1+max_x-min_x);
                    h_profile.fill(0);
                    foreach (QPoint p , toClearStack)
                    {
                        h_profile[p.x()-min_x]++;
                        v_profile[p.y()-min_y]++;
                    }
                    double v_avg=0;
                    for (int v = 0; v<v_profile.size(); v++)
                        v_avg+=v_profile[v];
                    v_avg/=v_profile.size();
                    double h_avg=0;
                    for (int h = 0; h<h_profile.size(); h++)
                        h_avg+=h_profile[h];
                    h_avg/=h_profile.size();
                    
                    double h_std_dev = 0;
                    for (int h = 0; h<h_profile.size(); h++)
                        h_std_dev += pow(h_profile[h]-h_avg,2);
                    h_std_dev/=h_profile.size();
                    
//                    printf("ratio: %f, std dev: %f\n",v_avg/h_avg,h_std_dev);
                    if (v_avg/h_avg < horzLongRatio || h_std_dev>stdDevLimit)
                        toClearStack.clear();
                }
                
                while (!toClearStack.isEmpty())
                {
                    QPoint r = toClearStack.back();
                    toClearStack.pop_back();
                    
                    //printf("remove p(%d,%d)\n",r.x(),r.y());
                    on.setPixel(r,WHITE);
                }
            }
        }
    }
}
