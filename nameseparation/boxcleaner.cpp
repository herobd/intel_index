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
     
     
     //edge/line filter
     
     for (j=0; j<9; j++)
     {
         lineFilterAtJ(j,ret);
     }
     for (j=ret.height()-1; j>ret.height()-10; j--)
     {
         lineFilterAtJ(j,ret);
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
//                for (;runLength>0; runLength--)
//                    ret.setPixel(i-runLength,j,WHITE);
                cond_clear_line(runLength,i,j,ret);
                
                                
                
                
                //ehhhh
//                int below_white = 0;
//                int below_depth_track=1;
//                for (;runLength>0; runLength--)
//                {
//                    ret.setPixel(i-runLength,j,WHITE);
//                    if (j<ret.height()-3 && qGray(ret.pixel(i-runLength,j+1)) == BLACK)
//                    {
//                        if (below_white>0)
//                        {
//                            below_white = -1;
                            
//                        }
//                        else
//                        {
//                            below_white--;
//                        }
//                        int num_pixels_below = 1;
//                        for (int counter=2; counter<5; counter++)
//                        {
//                            if (j+counter<ret.height() && qGray(ret.pixel(i-runLength,j+counter)) == BLACK)
//                            {
//                                num_pixels_below++;
//                            }
//                            else
//                            {
//                                break;
//                            }
//                        }
//                        if (num_pixels_below > 2)
//                        {
//                            below_depth_track--;
//                            if (below_depth_track==0)
//                            {
//                                //doit
//                                //calc slope
//                                if (below_white>-5)
//                                {
                                    
//                                }
//                                //draw pixels
//                            }
//                        }
                        
//                    }
//                    else
//                    {
//                        below_white++;
//                        if (below_depth_track<=0)
//                        {
//                            //do ti
                            
//                            below_depth_track=1;
//                        }
//                    }
//                }
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
    int BLOB_THRESH = 35;
    int HORZ_MARK_THRESH = 900;
    int LONGNESS_RATIO = 2.4;
    int STD_DEV_LIMIT = 38;
    QImage mark = ret.copy(0,0,ret.width(),ret.height());
    QVector<QPoint> workingStack;
    QVector<QPoint> toClearStack;
    
    for (int j=0; j<mark.height(); j++)
    {
//        if (j > 10 && j < 12)
//            j = mark.height()-12;
        for (int i=0; i<mark.width(); i++)
        {
            if (qGray(mark.pixel(i,j)) == BLACK)
            {
                int num=0;
                QPoint p(i,j);
                workingStack.push_back(p);
                int min_x, min_y, max_x, max_y;
                min_x = min_y = INT_POS_INFINITY;
                max_x = max_y = 0;
                while (!workingStack.isEmpty())
                {   
                    QPoint cur = workingStack.back();
                    workingStack.pop_back();
                    toClearStack.push_back(cur);
                    
                    mark.setPixel(cur,WHITE);
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
                if (num>HORZ_MARK_THRESH)
                {
                    toClearStack.clear();
                }
                else if (num>BLOB_THRESH)
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
                    
                    printf("ratio: %f, std dev: %f\n",v_avg/h_avg,h_std_dev);
                    if (v_avg/h_avg < LONGNESS_RATIO || h_std_dev>STD_DEV_LIMIT)
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

QImage BoxCleaner::clearLineAndCloseLetters(QImage &src, int est_y)
{
    int SEARCH_BAND = 10;
    int STRUCT_ELE_SIZE = 3;
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
        for (int j=0; j<=SEARCH_BAND*2 + 1; j++)
        {
            for (int i=0; i<src.width(); i++)
            {
                if (qGray(ret.pixel(i,j+(est_y-SEARCH_BAND))) == BLACK)
                    profile[j]++;
            }
            if (profile[j] > LINE_THRESH)
            {
                
                if (!erasing)
                {
                    erasing=true;
                    aboveLine=j+(est_y-SEARCH_BAND)-1;
                    for (int i=0; i<src.width(); i++)
                    {
                        if (qGray(ret.pixel(i,j+(est_y-SEARCH_BAND))) == BLACK && 
                                qGray(ret.pixel(i,j+(est_y-SEARCH_BAND)-1)) == BLACK)
                        {
                            QPoint p(i,j+(est_y-SEARCH_BAND)-1);
                            pointsToClose.append(p);
                        }
                        ret.setPixel(i,j+(est_y-SEARCH_BAND),WHITE);
                    }
                }
                else
                {
                    for (int i=0; i<src.width(); i++)
                    {
                        ret.setPixel(i,j+(est_y-SEARCH_BAND),WHITE);
                    }
                }
                
            }
            else if (erasing)
            {
                belowLine=j+(est_y-SEARCH_BAND);
                for (int i=0; i<src.width(); i++)
                {
                    if (qGray(src.pixel(i,j+(est_y-SEARCH_BAND)-1)) == BLACK && 
                            qGray(src.pixel(i,j+(est_y-SEARCH_BAND))) == BLACK)
                    {
                        QPoint p(i,j+(est_y-SEARCH_BAND));
                        pointsToClose.append(p);
                    }
                }
                break;                
            }
        }
        printf("ablove:%d, below:%d\n",aboveLine,belowLine);
        //top and bottom or line clean up
        for (int i=1; i<ret.width()-1; i++)
        {
            if (qGray(ret.pixel(i,aboveLine)) == BLACK && 
                    qGray(ret.pixel(i-1,aboveLine-1)) != BLACK && 
                    qGray(ret.pixel(i,aboveLine-1)) != BLACK && 
                    qGray(ret.pixel(i+1,aboveLine-1)) != BLACK)
                ret.setPixel(i,aboveLine,WHITE);
            
//            if (qGray(ret.pixel(i,belowLine)) == BLACK && 
//                    qGray(ret.pixel(i-1,belowLine+1)) != BLACK && 
//                    qGray(ret.pixel(i,belowLine+1)) != BLACK && 
//                    qGray(ret.pixel(i+1,belowLine+1)) != BLACK)
                ret.setPixel(i,belowLine,WHITE);
        }
        
//        QVector<QPoint> pointsToErrode;
//        QImage close_tmp = ret.copy(0,0,ret.width(),ret.height());
//        //dialate
//        foreach (QPoint p, pointsToClose)
//        {
//            for (int ri=-STRUCT_ELE_SIZE; ri<=STRUCT_ELE_SIZE; ri++)
//            {
//                for (int rj=-STRUCT_ELE_SIZE; rj<=STRUCT_ELE_SIZE; rj++)
//                {
//                    if (abs(ri)+abs(rj)<STRUCT_ELE_CORNER)
//                    {
//                        if (ri+p.x()>=0 && ri+p.x()<ret.width() &&
//                                rj+p.y()>=0 && rj+p.y()<ret.height())
//                        {
//                            QPoint toBlack(ri+p.x(),rj+p.y());
//                            close_tmp.setPixel(toBlack,BLACK);
//                            pointsToErrode.append(toBlack);
//                        }
//                    }
//                }
//            }
//        }
        
//        //errode
//        foreach (QPoint p, pointsToErrode)
//        {
//            bool allBlack = true;
//            for (int ri=-STRUCT_ELE_SIZE; ri<=STRUCT_ELE_SIZE && allBlack; ri++)
//            {
//                for (int rj=-STRUCT_ELE_SIZE; rj<=STRUCT_ELE_SIZE; rj++)
//                {
//                    if (abs(ri)+abs(rj)<STRUCT_ELE_CORNER)
//                    {
//                        if (ri+p.x()>=0 && ri+p.x()<ret.width() &&
//                                rj+p.y()>=0 && rj+p.y()<ret.height())
//                        {
//                            if (qGray(close_tmp.pixel(p.x()+ri,p.y()+rj)) != BLACK)
//                            {
//                                    allBlack=false;
//                                    break;
//                            }
//                        }
//                    }
//                }
//            }
//            if (allBlack)
//            {
//                ret.setPixel(p,BLACK);
//            }
////            else
////            {
////                ret.setPixel(p,WHITE);
////            }
//        }
        
    }
    return ret;
}
