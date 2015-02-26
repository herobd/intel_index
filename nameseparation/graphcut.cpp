#include "graphcut.h"

#include <opencv2/viz/vizcore.hpp>


int maxedge;
int maxweight;
int minweight;
int minedge;

QVector<double> GraphCut::correctScores;
QVector<double> GraphCut::incorrectScores;

inline void setEdge(int x1, int y1, int x2, int y2, GraphType* g, const BPixelCollection &img, const int* invDistMap, double blackToBlackBias, double whiteToBlackBias, double blackToWhiteBias, double whiteToWhiteBias, double reducer, int width)
{
    
    if (img.pixel(x1,y1) && img.pixel(x2,y2))
    {
        g -> add_edge(x1+y1*width, x2+y2*width,
                      (invDistMap[x2+y2*width]+invDistMap[x1+y1*width])*(reducer*blackToBlackBias),
                      (invDistMap[x1+y1*width]+invDistMap[x2+y2*width])*(reducer*blackToBlackBias));
    }
    else if (img.pixel(x1,y1) && !img.pixel(x2,y2))
    {
        g -> add_edge(x1+y1*width, x2+y2*width,
                      (invDistMap[x2+y2*width]+invDistMap[x1+y1*width])*(blackToWhiteBias),
                      (invDistMap[x1+y1*width]+invDistMap[x2+y2*width])*(whiteToBlackBias));
    }
    else if (!img.pixel(x1,y1) && img.pixel(x2,y2))
    {
        g -> add_edge(x1+y1*width, x2+y2*width,
                      (invDistMap[x2+y2*width]+invDistMap[x1+y1*width])*(whiteToBlackBias),
                      (invDistMap[x1+y1*width]+invDistMap[x2+y2*width])*(blackToWhiteBias));
    }
    else
    {
        g -> add_edge(x1+y1*width, x2+y2*width,
                      (invDistMap[x2+y2*width]+invDistMap[x1+y1*width])*whiteToWhiteBias,
                      (invDistMap[x1+y1*width]+invDistMap[x2+y2*width])*whiteToWhiteBias);
    }
}

//Uses Boykov graph cut
int GraphCut::pixelsOfSeparation(int* invDistMap, int width, int height, BPixelCollection &img, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight, int split_method, int vert_divide)
{

    GraphType *g = new GraphType(width*height, 4*(width-1)*(height-1)-(height+width)); 
    
    for (int i=0; i<width*height; i++)
    {
        g->add_node();
    }
#if NORM_SAVE_ANCHOR 
    QImage debug = img.makeImage().getImage();
    QVector<QRgb> ct = debug.colorTable();
    ct.append(qRgb(205,50,50));
    ct.append(qRgb(50,205,50));
    debug.setColorTable(ct);
#endif
    
    if (split_method == SPLIT_HORZ)
    {
        //find source pixels
        int count_source = height*ANCHOR_L;
        for (int i=0; count_source>0 && i<width; i++)
        {
            for (int j=0; count_source>0 && j<height; j++)
            {
                if (img.pixel(i,j))
                {
                    int index = i+width*j;
                    g -> add_tweights(index, anchor_weight,0);//invDistMap[index], 0);
#if NORM_SAVE_ANCHOR
                    debug.setPixel(i,j,2);
#endif
                    count_source--;
                }
            }
        }
        
        
        
        
        //diag method
    //    QImage mark = img.copy(0,0,img.width(),img.height());
    //    QVector<QPoint> workingStack;
    //    for (int o=0; o<width && count_source>0; o++)
    //    {
    //        for (int i=0; i<=o && i<height && count_source>0; i++)
    //        {
    //            if (qGray(img.pixel(o-i,i))==BLACK)
    //            {
    //                int index = (o-i)+width*i;
    //                g -> add_tweights(index, INT_POS_INFINITY,0);
    //                count_source--;
    //                debug.setPixel((o-i),i,150);
                    
                    //fill
    //                QPoint p(o-i,i);
    //                workingStack.push_back(p);
    //                while (!workingStack.isEmpty() && count_source>0)
    //                {   
    //                    QPoint cur = workingStack.back();
    //                    workingStack.pop_back();
    //                    int index = cur.x()+width*cur.y();
    //                    g -> add_tweights(index, INT_POS_INFINITY,0);
    //                    //debug.setPixel(cur,150);
    //                    count_source--;
                        
    //                    mark.setPixel(cur,WHITE);
                        
    //                    if (cur.x()>0 && qGray(mark.pixel(cur.x()-1,cur.y())) == BLACK)
    //                    {
    //                        QPoint pp(cur.x()-1,cur.y());
    //                        workingStack.push_back(pp);
    //                    }
                        
                        
    //                    if (cur.x()<mark.width()-1 && qGray(mark.pixel(cur.x()+1,cur.y())) == BLACK)
    //                    {
    //                        QPoint pp(cur.x()+1,cur.y());
    //                        workingStack.push_back(pp);
                            
    //                    }
    //                    if (cur.y()<mark.height()-1 && qGray(mark.pixel(cur.x(),cur.y()+1)) == BLACK)
    //                    {
    //                        QPoint pp(cur.x(),cur.y()+1);
    //                        workingStack.push_back(pp);
    //                    }
    //                    if (cur.y()>0 && qGray(mark.pixel(cur.x(),cur.y()-1)) == BLACK)
    //                    {
    //                        QPoint pp(cur.x(),cur.y()-1);
    //                        workingStack.push_back(pp);
    //                    }
    //                }
    //            }
    //        }
    //    }
    //    workingStack.clear();
        
        int count_sink=height*ANCHOR_R;
        
        //find sink pixels
    //    for (int i=width-1; count_sink>0 && i>=0; i--)
    //    {
    //        for (int j=height-1; count_sink>0 && j>=0; j--)
    //        {
    //            if (qGray(img.pixel(i,j))==BLACK)
    //            {
    //                int index = i+width*j;
    //                g -> add_tweights(index, 0, INT_POS_INFINITY);//invDistMap[index]);
    ////                debug.setPixel(i,j,150);
    //                count_sink--;
    //            }
    //        }
    //    }
        
        //diag method
        for (int o=0; o<width && count_sink>0; o++)
        {
            for (int i=0; i<=o && i<height && count_sink>0; i++)
            {
                //debug.setPixel((width-1)-(o-i),(height-1)-i,220);
                if (img.pixel((width-1)-(o-i),(height-1)-i))
                {
                    int index = ((width-1)-(o-i))+width*((height-1)-i);
                    g -> add_tweights(index, 0, anchor_weight);
                    count_sink--;
#if NORM_SAVE_ANCHOR
                    debug.setPixel((width-1)-(o-i),(height-1)-i,3);
#endif
                    //fill
    //                QPoint p((width-1)-(o-i),(height-1)-i);
    //                workingStack.push_back(p);
    //                while (!workingStack.isEmpty() && count_sink>0)
    //                {   
    //                    QPoint cur = workingStack.back();
    //                    workingStack.pop_back();
    //                    int index = cur.x()+width*cur.y();
    //                    g -> add_tweights(index, 0, INT_POS_INFINITY);
    //                    //debug.setPixel(cur,150);
    //                    count_sink--;
                        
    //                    mark.setPixel(cur,WHITE);
                        
    //                    if (cur.x()<mark.width()-1 && qGray(mark.pixel(cur.x()+1,cur.y())) == BLACK)
    //                    {
    //                        QPoint pp(cur.x()+1,cur.y());
    //                        workingStack.push_back(pp);
                            
    //                    }
                        
                        
    //                    if (cur.x()>0 && qGray(mark.pixel(cur.x()-1,cur.y())) == BLACK)
    //                    {
    //                        QPoint pp(cur.x()-1,cur.y());
    //                        workingStack.push_back(pp);
    //                    }
    //                    if (cur.y()>0 && qGray(mark.pixel(cur.x(),cur.y()-1)) == BLACK)
    //                    {
    //                        QPoint pp(cur.x(),cur.y()-1);
    //                        workingStack.push_back(pp);
    //                    }
    //                    if (cur.y()<mark.height()-1 && qGray(mark.pixel(cur.x(),cur.y()+1)) == BLACK)
    //                    {
    //                        QPoint pp(cur.x(),cur.y()+1);
    //                        workingStack.push_back(pp);
    //                    }
    //                }
                }
            }
        }
    }
    else if (split_method == SPLIT_VERT)
    {
        //all pixels are either source or sink
        for (int j=0; j<vert_divide; j++)
        {
            double anchor_weight_for_level = anchor_weight * ((2.5*vert_divide-j)/(double)(2.5*vert_divide));
            //printf("%f, ",((vert_divide-j)/(double)vert_divide));
            for (int i=0; i<width; i++)
            {
                if (img.pixel(i,j))
                {
                    int index = i+width*j;
                    g -> add_tweights(index, (int)anchor_weight_for_level,0);
#if NORM_SAVE_ANCHOR
                    debug.setPixel(i,j,2);
#endif
                }
            }
        }
        for (int j=height-1; j>=vert_divide; j--)
        {
            double anchor_weight_for_level = anchor_weight * ((((height-1) -(double)vert_divide) + j-vert_divide)/(2.*((height-1) -(double)vert_divide)));
            //printf("%f- ",((j-vert_divide)/((height-1) -vert_divide)));
            for (int i=0; i<width; i++)
            {
                if (img.pixel(i,j))
                {
                    int index = i+width*j;
                    g -> add_tweights(index, 0, (int)anchor_weight_for_level);
#if NORM_SAVE_ANCHOR
                    debug.setPixel(i,j,3);
#endif
                }
            }
        }
    }
    else if (split_method == CHOP_TOP)
    {
        //all pixels are source and sink
        for (int j=0; j<height; j++)
        {
            double src_anchor_weight_for_level = anchor_weight * (((height-1)-j)/(double)(height-1));
            double sink_anchor_weight_for_level = /*.3 * anchor_weight;*/(anchor_weight/2.0) * (j/(double)(height-1));
            for (int i=0; i<width; i++)
            {
//                if (img.pixel(i,j))
                {
                    int index = i+width*j;
                    g -> add_tweights(index, (int)src_anchor_weight_for_level,(int)sink_anchor_weight_for_level);
    //                debug.setPixel(i,j,150);
                }
            }
        }
    }
    
#if NORM_SAVE_ANCHOR
    debug.save("./anchors.ppm");
#endif
    
    //printf("num source:%d, num sink:%d\n",count_source,count_sink);
    
//    double BLACK_TO_BLACK_V_BIAS = 1;
//    double BLACK_TO_BLACK_H_BIAS = 2;
//    double BLACK_TO_BLACK_D_BIAS = 2.23;
//    double WHITE_TO_BLACK_BIAS = .5;
//    double BLACK_TO_WHITE_BIAS = .5;
//    double WHITE_TO_WHITE_V_BIAS = 1;
//    double WHITE_TO_WHITE_H_BIAS = .333;
//    double WHITE_TO_WHITE_D_BIAS = .5;
    double BLACK_TO_BLACK_V_BIAS = .5;
    double BLACK_TO_BLACK_H_BIAS = .5;
    double BLACK_TO_BLACK_D_BIAS = (BLACK_TO_BLACK_V_BIAS+BLACK_TO_BLACK_H_BIAS)-sqrt(pow(BLACK_TO_BLACK_V_BIAS,2)+pow(BLACK_TO_BLACK_H_BIAS,2));
    double WHITE_TO_BLACK_BIAS = .5;
    double BLACK_TO_WHITE_BIAS = .5;
    double WHITE_TO_WHITE_V_BIAS = .5;
    double WHITE_TO_WHITE_H_BIAS = .5;
    double WHITE_TO_WHITE_D_BIAS = (WHITE_TO_WHITE_V_BIAS+WHITE_TO_WHITE_H_BIAS)-sqrt(pow(WHITE_TO_WHITE_V_BIAS,2)+pow(WHITE_TO_WHITE_H_BIAS,2));
    
    if (split_method==SPLIT_VERT)
    {
        BLACK_TO_BLACK_V_BIAS = 0.75;
        BLACK_TO_BLACK_H_BIAS = 0.75;
        BLACK_TO_BLACK_D_BIAS = (BLACK_TO_BLACK_V_BIAS+BLACK_TO_BLACK_H_BIAS)-sqrt(pow(BLACK_TO_BLACK_V_BIAS,2)+pow(BLACK_TO_BLACK_H_BIAS,2));
        WHITE_TO_BLACK_BIAS = .5;
        BLACK_TO_WHITE_BIAS = .5;
        WHITE_TO_WHITE_V_BIAS = .5;
        WHITE_TO_WHITE_H_BIAS = .5;
        WHITE_TO_WHITE_D_BIAS = (WHITE_TO_WHITE_V_BIAS+WHITE_TO_WHITE_H_BIAS)-sqrt(pow(WHITE_TO_WHITE_V_BIAS,2)+pow(WHITE_TO_WHITE_H_BIAS,2));
    }
    
    double reducer = 1;
    
    //connect all pixels
    for (int i=0; i<width; i++)
    {
        for (int j=0; j<height; j++)
        {   
//            if (split_method==SPLIT_VERT)
//            {
//                if (j<vert_divide)
//                {
//                    reducer = ((2*vert_divide-j)/(double)(2*vert_divide));
//                }
//                else
//                {
//                    reducer = ((((height-1) -(double)vert_divide) + j-vert_divide)/(2.*((height-1) -(double)vert_divide)));
//                }
//            }
//            else if (split_method==CHOP_TOP)
//            {
//                   reducer = ((3.0*height-j)/(double)(3*height));
//            }
            
            
            if (i+1<width)
            {
                setEdge(i,j,i+1,j,g,img,invDistMap,BLACK_TO_BLACK_H_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_H_BIAS,reducer,width);
            }
            
            if (j+1<height)
            {
                setEdge(i,j,i,j+1,g,img,invDistMap,BLACK_TO_BLACK_V_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_V_BIAS,reducer,width);
            }
            
            if (j>0 && i<width-1)
            {
                setEdge(i,j,i+1,j-1,g,img,invDistMap,BLACK_TO_BLACK_D_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_D_BIAS,reducer,width);
            }
            
            if (j<height-1 && i<width-1)
            {
                setEdge(i,j,i+1,j+1,g,img,invDistMap,BLACK_TO_BLACK_D_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_D_BIAS,reducer,width);
            }
        }
    }
    
    int ret = g -> maxflow();
    
//    QImage debug2 = debug.convertToFormat(QImage::Format_RGB16);
//    QRgb lw = qRgb(255, 100, 100);
//    QRgb lb = qRgb(155, 0, 0);
//    QRgb rw = qRgb(100,255, 100);
//    QRgb rb = qRgb(0, 155, 0);
//    QRgb a = qRgb(255, 255, 255);
    
    //add all black pixels which
    for (int index=0; index<width*height; index++)
    {
        if (img.pixelIsMine(index%width,index/width))
        {
            /*if (qGray(debug.pixel(index%width,index/width))!=BLACK && qGray(debug.pixel(index%width,index/width))!=WHITE)
            {
                debug2.setPixel(index%width,index/width,a);
            }
            else*/ if (g->what_segment(index) == GraphType::SOURCE)
            {
                outSource.append(index);
    //            debug2.setPixel(index%width,index/width,lb);
            }
            else
            {
                outSink.append(index);
            }
    //        else if (g->what_segment(index) == GraphType::SOURCE)
    //            debug2.setPixel(index%width,index/width,lw);
    //        else if (qGray(img.pixel(index%width,index/width))==BLACK)
    //            debug2.setPixel(index%width,index/width,rb);
    //        else
    //            debug2.setPixel(index%width,index/width,rw);
        }
    }
    
//    QString debugfile = "./cut_";
//    QString num;
//    num.setNum(width);
//    debugfile.append(num);
//    debugfile.append(".ppm");
//    debug2.save(debugfile);
    
    delete g;
    return ret;
}


int GraphCut::pixelsOfSeparationMicro(int* invDistMap, int width, int height, BPixelCollection &img, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight, int split_method, int vert_divide)
{

    GraphType *g = new GraphType(width*height, 4*(width-1)*(height-1)-(height+width)); 
    
    for (int i=0; i<width*height; i++)
    {
        g->add_node();
    }
    
    QImage debug = img.makeImage().getImage();
    QVector<QRgb> ct = debug.colorTable();
    ct.append(qRgb(205,50,50));
    ct.append(qRgb(50,205,50));
    debug.setColorTable(ct);
    
    if (split_method == SPLIT_HORZ)
    {
        //find source pixels
        int count_source = std::min((int)(height*ANCHOR_MICRO),width);
        
        
        
        
        //diag method
//        QImage mark = img.copy(0,0,img.width(),img.height());
        QVector<QPoint> workingStack;
        for (int o=0; o<width && count_source>0; o++)
        {
            for (int i=0; i<=o && i<height && count_source>0; i++)
            {
                if (img.pixel(o-i,i))
                {
                    int index = (o-i)+width*i;
                    g -> add_tweights(index, INT_POS_INFINITY,0);
                    count_source--;
                    debug.setPixel((o-i),i,2);

                }
            }
        }
        workingStack.clear();
        
        int count_sink=std::min((int)(height*ANCHOR_MICRO),width);
        

        
        //diag method
        for (int o=0; o<width && count_sink>0; o++)
        {
            for (int i=0; i<=o && i<height && count_sink>0; i++)
            {
                //debug.setPixel((width-1)-(o-i),(height-1)-i,220);
                if (img.pixel((width-1)-(o-i),(height-1)-i))
                {
                    int index = ((width-1)-(o-i))+width*((height-1)-i);
                    g -> add_tweights(index, 0, anchor_weight);
                    count_sink--;
                    debug.setPixel((width-1)-(o-i),(height-1)-i,3);
                    

                }
            }
        }
    }
    else 
        assert(false);
    
    debug.save("./anchors.ppm");
    

    double BLACK_TO_BLACK_V_BIAS = .5;
    double BLACK_TO_BLACK_H_BIAS = .5;
    double BLACK_TO_BLACK_D_BIAS = (BLACK_TO_BLACK_V_BIAS+BLACK_TO_BLACK_H_BIAS)-sqrt(pow(BLACK_TO_BLACK_V_BIAS,2)+pow(BLACK_TO_BLACK_H_BIAS,2));
    double WHITE_TO_BLACK_BIAS = .5;
    double BLACK_TO_WHITE_BIAS = .5;
    double WHITE_TO_WHITE_V_BIAS = .5;
    double WHITE_TO_WHITE_H_BIAS = .5;
    double WHITE_TO_WHITE_D_BIAS = (WHITE_TO_WHITE_V_BIAS+WHITE_TO_WHITE_H_BIAS)-sqrt(pow(WHITE_TO_WHITE_V_BIAS,2)+pow(WHITE_TO_WHITE_H_BIAS,2));
    
    if (split_method==SPLIT_VERT)
    {
        BLACK_TO_BLACK_V_BIAS = 0.75;
        BLACK_TO_BLACK_H_BIAS = 0.75;
        BLACK_TO_BLACK_D_BIAS = (BLACK_TO_BLACK_V_BIAS+BLACK_TO_BLACK_H_BIAS)-sqrt(pow(BLACK_TO_BLACK_V_BIAS,2)+pow(BLACK_TO_BLACK_H_BIAS,2));
        WHITE_TO_BLACK_BIAS = .5;
        BLACK_TO_WHITE_BIAS = .5;
        WHITE_TO_WHITE_V_BIAS = .5;
        WHITE_TO_WHITE_H_BIAS = .5;
        WHITE_TO_WHITE_D_BIAS = (WHITE_TO_WHITE_V_BIAS+WHITE_TO_WHITE_H_BIAS)-sqrt(pow(WHITE_TO_WHITE_V_BIAS,2)+pow(WHITE_TO_WHITE_H_BIAS,2));
    }
    
    double reducer = 1;
    
    //connect all pixels
    for (int i=0; i<width; i++)
    {
        for (int j=0; j<height; j++)
        {   
            
            
            if (i+1<width)
            {
                setEdge(i,j,i+1,j,g,img,invDistMap,BLACK_TO_BLACK_H_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_H_BIAS,reducer,width);
            }
            
            if (j+1<height)
            {
                setEdge(i,j,i,j+1,g,img,invDistMap,BLACK_TO_BLACK_V_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_V_BIAS,reducer,width);
            }
            
            if (j>0 && i<width-1)
            {
                setEdge(i,j,i+1,j-1,g,img,invDistMap,BLACK_TO_BLACK_D_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_D_BIAS,reducer,width);
            }
            
            if (j<height-1 && i<width-1)
            {
                setEdge(i,j,i+1,j+1,g,img,invDistMap,BLACK_TO_BLACK_D_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_D_BIAS,reducer,width);
            }
        }
    }
    
    int ret = g -> maxflow();
    
//    QImage debug2 = debug.convertToFormat(QImage::Format_RGB16);
//    QRgb lw = qRgb(255, 100, 100);
//    QRgb lb = qRgb(155, 0, 0);
//    QRgb rw = qRgb(100,255, 100);
//    QRgb rb = qRgb(0, 155, 0);
//    QRgb a = qRgb(255, 255, 255);
    
    //add all black pixels which
    for (int index=0; index<width*height; index++)
    {
        if (img.pixelIsMine(index%width,index/width))
        {
            /*if (qGray(debug.pixel(index%width,index/width))!=BLACK && qGray(debug.pixel(index%width,index/width))!=WHITE)
            {
                debug2.setPixel(index%width,index/width,a);
            }
            else*/ if (g->what_segment(index) == GraphType::SOURCE)
            {
                outSource.append(index);
    //            debug2.setPixel(index%width,index/width,lb);
            }
            else
            {
                outSink.append(index);
            }
    //        else if (g->what_segment(index) == GraphType::SOURCE)
    //            debug2.setPixel(index%width,index/width,lw);
    //        else if (qGray(img.pixel(index%width,index/width))==BLACK)
    //            debug2.setPixel(index%width,index/width,rb);
    //        else
    //            debug2.setPixel(index%width,index/width,rw);
        }
    }
    
//    QString debugfile = "./cut_";
//    QString num;
//    num.setNum(width);
//    debugfile.append(num);
//    debugfile.append(".ppm");
//    debug2.save(debugfile);
    
    delete g;
    return ret;
}

/*Each pixel assigned a slope; or multiple slopes; fuzzuness to link to neighbors, or when slope undetrminable (is depenndant on heighbors). Slopes are discretized. Each node is sent to a layer
  D: value(s) for each pixel, highest, lowest, number of bins
  */


inline QVector<QVector<QVector<double> > > make3dImage(const BPixelCollection &img,int* invDistMap,const NDimensions &dimensions)
{
    QVector<QVector<QVector<double> > > ret(img.width());
    
    int slopeDifRange = SLOPE_DIF_TOLERANCE*dimensions.getBinNums()[0];
    printf("difrange:%d\n",slopeDifRange);
    for (int x=0; x<img.width(); x++)
    {
        QVector<QVector<double> > flat(img.height());
        for (int y=0; y<img.height(); y++)
        {
            QVector<double> slope(dimensions.getBinNums()[0]);
            
            if (img.pixel(x,y))
            {
                
                
//                int bin = dimensions.getBinsForPixel(x,y)[0];
//                if (bin>=0)
//                {
//                    for (int k=0; k<dimensions.getBinNums()[0]; k++)
//                    {
//                        slope[k]=0;
//                    }
//                    int secondBin = dimensions.getSecondBinsForPixel(x,y)[0];
//                    if (secondBin >= 0)
//                    {
////                        int secondBin = bin/dimensions.getBinNums()[0];
////                        bin = bin%dimensions.getBinNums()[0];
                        
//                        slope[secondBin]=invDistMap[x+y*img.width()];
//                        for (int kb=1; kb<slopeDifRange; kb++)
//                        {
//                            slope[mod((secondBin+kb),dimensions.getBinNums()[0])]+=invDistMap[x+y*img.width()]*((slopeDifRange-kb)/(1.0*slopeDifRange));
//                            slope[mod((secondBin-kb),dimensions.getBinNums()[0])]+=invDistMap[x+y*img.width()]*((slopeDifRange-kb)/(1.0*slopeDifRange));
//                        }
//                    }
                    
//                    slope[bin]=invDistMap[x+y*img.width()];
//                    for (int kb=1; kb<slopeDifRange; kb++)
//                    {
//                        slope[mod((bin+kb),dimensions.getBinNums()[0])]+=invDistMap[x+y*img.width()]*((slopeDifRange-kb)/(1.0*slopeDifRange));
//                        slope[mod((bin-kb),dimensions.getBinNums()[0])]+=invDistMap[x+y*img.width()]*((slopeDifRange-kb)/(1.0*slopeDifRange));
//                    }
//                }
                QVector<int> binsForDim = dimensions.getBinsForDimensionsOfPixel(x,y)[0];
                if (binsForDim.size()>0)
                {
                    for (int k=0; k<dimensions.getBinNums()[0]; k++)
                    {
                        slope[k]=100;//'a' from inv computation
                    }
                    foreach (int bin, binsForDim)
                    {
//                        int secondBin = bin/dimensions.getBinNums()[0];
//                        bin = bin%dimensions.getBinNums()[0];
                        
                        slope[bin]=invDistMap[x+y*img.width()];
                        for (int kb=1; kb<slopeDifRange; kb++)
                        {
//                            slope[mod((bin+kb),dimensions.getBinNums()[0])] = std::min((int) (invDistMap[x+y*img.width()]*((slopeDifRange-kb)/(1.0*slopeDifRange))+slope[mod((bin+kb),dimensions.getBinNums()[0])]), (int) (invDistMap[x+y*img.width()] * (slopeDifRange-kb)/(1.0*slopeDifRange)));
//                            slope[mod((bin-kb),dimensions.getBinNums()[0])] = std::min((int) (invDistMap[x+y*img.width()]*((slopeDifRange-kb)/(1.0*slopeDifRange))+slope[mod((bin-kb),dimensions.getBinNums()[0])]), (int) (invDistMap[x+y*img.width()] * (slopeDifRange-kb)/(1.0*slopeDifRange)));
                            slope[mod((bin+kb),dimensions.getBinNums()[0])] = std::max((int)(slope[mod((bin+kb),dimensions.getBinNums()[0])]), (int) (invDistMap[x+y*img.width()] * (slopeDifRange-kb)/(1.0*slopeDifRange)));
                            slope[mod((bin-kb),dimensions.getBinNums()[0])] = std::max((int) (slope[mod((bin-kb),dimensions.getBinNums()[0])]), (int) (invDistMap[x+y*img.width()] * (slopeDifRange-kb)/(1.0*slopeDifRange)));
                        }
                    }
                    
                }
                else
                {
                    for (int k=0; k<dimensions.getBinNums()[0]; k++)
                    {
                        slope[k]=invDistMap[x+y*img.width()];
                    }
                }
            }
            else
            {
                for (int k=0; k<dimensions.getBinNums()[0]; k++)
                {
                    slope[k]=invDistMap[x+y*img.width()];
                }
            }
            flat[y]=slope;
        }
        ret[x]=flat;
    }
    
    ///test///
    double testMax=0;
    for (int x=0; x<img.width(); x++)
    {
        for (int y=0; y<img.height(); y++)
        {
            for (int s=0; s<dimensions.getBinNums()[0]; s++)
            {
                if (ret[x][y][s] > testMax)
                    testMax=ret[x][y][s];
            }
        }
    }
//    printf("testMat=%f\n",testMax);
    QVector<QRgb> default_color_table;
    for (int i=0; i<256; i++)
    {
        default_color_table.append(qRgb(i,i,i));
    }
    for (int s=0; s<dimensions.getBinNums()[0]; s++)
    {
        QImage test(img.width(),img.height(),QImage::Format_Indexed8);
        test.setColorTable(default_color_table);
        for (int x=0; x<img.width(); x++)
        {
            for (int y=0; y<img.height(); y++)
            {
                test.setPixel(x,y,ret[x][y][s]*(255.0/testMax));
            }
        }
        QString debugfile = "./dist_3d/layer_";
        QString num;
        num.setNum(s);
        debugfile.append(num);
        debugfile.append(".ppm");
        test.save(debugfile);
    }
    ///test///
    
    return ret;
}

inline void setEdge3d(int x1, int y1, int slope1, int x2, int y2, int slope2, GraphType* g, const Indexer &indexer, const QVector<QVector<QVector<double> > > &image3d, double weight)
{
//    if (img.pixel(x1,y1) && img.pixel(x2,y2))
        g -> add_edge(indexer.getIndex(x1,y1,slope1), indexer.getIndex(x2,y2,slope2),
                      (image3d[x1][y1][slope1]+image3d[x2][y2][slope2])*weight,
                      (image3d[x1][y1][slope1]+image3d[x2][y2][slope2])*weight);
//    else if (img.pixel(x1,y1) && !img.pixel(x2,y2))
//        g -> add_edge(indexer.getIndex(x1,y1,slope1), indexer.getIndex(x2,y2,slope2),
//                      (image3d[x1][y1][slope1]+image3d[x2][y2][slope2]),
//                      (image3d[x1][y1][slope1]+image3d[x2][y2][slope2]));
//    else if (!img.pixel(x1,y1) && img.pixel(x2,y2))
//        g -> add_edge(indexer.getIndex(x1,y1,slope1), indexer.getIndex(x2,y2,slope2),
//                      (image3d[x1][y1][slope1]+image3d[x2][y2][slope2]),
//                      (image3d[x1][y1][slope1]+image3d[x2][y2][slope2]));
//    else
//        g -> add_edge(indexer.getIndex(x1,y1,slope1), indexer.getIndex(x2,y2,slope2),
//                      (image3d[x1][y1][slope1]+image3d[x2][y2][slope2]),
//                      (image3d[x1][y1][slope1]+image3d[x2][y2][slope2]));
}

int GraphCut::pixelsOfSeparationNDimensions(int* invDistMap, int width, int height, const BPixelCollection &img, const NDimensions &dimensions, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight, int split_method, int vert_divide)
{
    assert(dimensions.numOfDim() == 1);//for now, we will only use three
    
    QVector<QVector<QVector<double> > > image3d = make3dImage(img,invDistMap,dimensions);
    
    
    
    int numNodes = width*height;
    int numEdges = 4*(width-1)*(height-1)-(height+width);
    for (int i=0; i<dimensions.numOfDim(); i++)
    {
        const Dimension* dim = dimensions.getDimension(i);
        numNodes *= dim->getNumBins();
        numEdges *= dim->getNumBins();
        numEdges += numNodes*(dim->getNumBins()-1);//assuming only striaght (no diag) connections for higher dimensions
    }
    
    Indexer indexer(width, height, &dimensions);
    
    typedef Graph<int,int,int> GraphType;
    GraphType *g = new GraphType(numNodes, numEdges); 
    
    for (int i=0; i<numNodes; i++)
    {
        g->add_node();
    }
    
    QImage debug = img.makeImage().getImage();
    QVector<QRgb> ct = debug.colorTable();
    ct.append(qRgb(205,50,50));
    ct.append(qRgb(50,205,50));
    debug.setColorTable(ct);
    
    if (split_method == SPLIT_HORZ)
    {
        //find source pixels
        int count_source = NEW_ANCHOR;
//        for (int j=0; count_source>0 && j<height; j++)
//        {
//            for (int i=0; count_source>0 && i<width; i++)
//            {
//                if (img.pixel(i,j))
//                {
//                    int index = indexer.getIndex(i,j);
//                    g -> add_tweights(index, anchor_weight,0);//invDistMap[index], 0);
//                    debug.setPixel(i,j,2);
//                    count_source--;
//                }
//            }
//        }
        
        
        
        
        //diag method
//        QImage mark = img.copy(0,0,img.width(),img.height());
//        QVector<QPoint> workingStack;
//        for (int o=0; o<width && count_source>0; o++)
//        {
//            for (int i=0; i<=o && i<height && count_source>0; i++)
//            {
//                int x = o-i;
//                int y = i;
//                if (img.pixel(x,y))
//                {
//                    int index = indexer.getIndex(x,y);//(o-i)+width*i;
//                    if (index<0)
//                        continue;
//                    g -> add_tweights(index, INT_POS_INFINITY,0);
//                    count_source--;
//                    debug.setPixel(x,y,2);
//                }
//            }
//        }
        //fill
        BImage mark = img.makeImage();
        QVector<QPoint> workingStack;
        foreach (QPoint seed, sourceSeeds)
        {
            workingStack.push_back(seed);
            mark.setPixel(seed,false);
        }
        while (!workingStack.isEmpty() && count_source>0)
        {   
            QPoint cur = workingStack.front();
            workingStack.pop_front();
            for (int k=0; k<dimensions.getBinNums()[0]; k++)
            {
                int index = indexer.getIndex(cur.x(),cur.y(),k);
                g -> add_tweights(index, anchor_weight, 0);
//                g -> add_tweights(index, 0, anchor_weight);
            }
            debug.setPixel(cur,2);
            count_source--;
            
            
            
            if (cur.x()<mark.width()-1 && mark.pixel(cur.x()+1,cur.y()))
            {
                QPoint pp(cur.x()+1,cur.y());
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
                
            }
            if (cur.x()>0 && mark.pixel(cur.x()-1,cur.y()))
            {
                QPoint pp(cur.x()-1,cur.y());
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()>0 && mark.pixel(cur.x(),cur.y()-1))
            {
                QPoint pp(cur.x(),cur.y()-1);
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()<mark.height()-1 && mark.pixel(cur.x(),cur.y()+1))
            {
                QPoint pp(cur.x(),cur.y()+1);
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
        }
        
        int count_sink=NEW_ANCHOR;//height*ANCHOR_R;
        
        //find sink pixels
//        for (int j=height-1; count_sink>0 && j>=0; j--)
//        {
//            for (int i=width-1; count_sink>0 && i>=0; i--)
//            {
//                if (img.pixel(i,j))
//                {
//                    int index = i+width*j;
//                    g -> add_tweights(index, 0, INT_POS_INFINITY);//invDistMap[index]);
//                    debug.setPixel(i,j,3);
//                    count_sink--;
//                }
//            }
//        }
        
        //diag method
//        for (int o=0; o<width && count_sink>0; o++)
//        {
//            for (int i=0; i<=o && i<height && count_sink>0; i++)
//            {
//                //debug.setPixel((width-1)-(o-i),(height-1)-i,220);
//                int x = (width-1)-(o-i);
//                int y = (height-1)-i;
//                if (img.pixel(x,y))
//                {
//                    int index = indexer.getIndex(x,y);
//                    if (index<0)
//                        continue;
//                    g -> add_tweights(index, 0, anchor_weight);
//                    count_sink--;
//                    debug.setPixel(x,y,1);
//                }
//            }
//        }
        //fill
        workingStack.clear();
        foreach (QPoint seed, sinkSeeds)
        {
            workingStack.push_back(seed);
            mark.setPixel(seed,false);
        }
        while (!workingStack.isEmpty() && count_sink>0)
        {   
            QPoint cur = workingStack.front();
            workingStack.pop_front();
            for (int k=0; k<dimensions.getBinNums()[0]; k++)
            {
                int index = indexer.getIndex(cur.x(),cur.y(),k);
                g -> add_tweights(index, 0, anchor_weight);
//                g -> add_tweights(index, anchor_weight,0 );
                
            }
            debug.setPixel(cur,3);
            count_sink--;
            
            
            
            if (cur.x()<mark.width()-1 && mark.pixel(cur.x()+1,cur.y()))
            {
                QPoint pp(cur.x()+1,cur.y());
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
                
            }
            if (cur.x()>0 && mark.pixel(cur.x()-1,cur.y()))
            {
                QPoint pp(cur.x()-1,cur.y());
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()>0 && mark.pixel(cur.x(),cur.y()-1))
            {
                QPoint pp(cur.x(),cur.y()-1);
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()<mark.height()-1 && mark.pixel(cur.x(),cur.y()+1))
            {
                QPoint pp(cur.x(),cur.y()+1);
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
        }
        
        
        debug.save("./anchors.ppm");
    }
    
    
    
    //connect all pixels
    //For simplicity, only doing three dimensions now
//    double FLAT_WEIGHT = .5;
//    double SLOPE_WEIGHT = 4;
    double FLAT_WEIGHT = 6;
    double SLOPE_WEIGHT = .5;
    int slope_size = dimensions.getBinNums()[0];
    for (int k=0; k<slope_size; k++)
    {
        
        for (int i=0; i<width; i++)
        {
            for (int j=0; j<height; j++)
            {   
                
                //C1
                if (i+1<width)
                {
                    setEdge3d(i,j,k,i+1,j,k,g,indexer,image3d,FLAT_WEIGHT);
                }
                
                if (j+1<height)
                {
                    setEdge3d(i,j,k,i,j+1,k,g,indexer,image3d,FLAT_WEIGHT);
                }
                
                if (k+1<slope_size)
                {
                    setEdge3d(i,j,k,i,j,k+1,g,indexer,image3d,SLOPE_WEIGHT);
                }
                else//fold
                {
                    setEdge3d(i,j,k,i,j,0,g,indexer,image3d,SLOPE_WEIGHT);
                }
                
                //C2 dumbed down
                if (j>0 && i<width-1)
                {
                    setEdge3d(i,j,k,i+1,j-1,k,g,indexer,image3d,2*FLAT_WEIGHT-sqrt(2*pow(FLAT_WEIGHT,2)));
                }
                
                if (j<height-1 && i<width-1)
                {
                    setEdge3d(i,j,k,i+1,j+1,k,g,indexer,image3d,2*FLAT_WEIGHT-sqrt(2*pow(FLAT_WEIGHT,2)));
                }
                
                
                
                //C3 nope
                
            }//j
        }//i
    }//k
    
    int ret = g -> maxflow();

    
    //add all black pixels which
    
    for (int x=0; x<width; x++)
    {
        for (int y=0; y<height; y++)
        {
            
            if (img.pixel(x,y))
            {
//                int index = indexer.getIndex(x,y);
//                if (dimensions.getSecondBinsForPixel(x,y)[0] < 0 && index>=0)
//                {
                    
//                    if (g->what_segment(index) == GraphType::SOURCE)
//                        outSource.append(x+width*y);
//                    else
//                        outSink.append(x+width*y);
//                }
//                else if (index >=0)
//                {
//                    int firstBin = dimensions.getBinsForPixel(x,y)[0];
//                    int firstIndex = indexer.getIndex(x,y,firstBin);
//                    int secondBin = dimensions.getSecondBinsForPixel(x,y)[0];
//                    int secondIndex = indexer.getIndex(x,y,secondBin);
                    
//                    if (g->what_segment(firstIndex) == GraphType::SOURCE)
//                        outSource.append(x+width*y);
//                    else
//                        outSink.append(x+width*y);
                    
//                    if (g->what_segment(secondIndex) == GraphType::SOURCE)
//                        outSource.append(x+width*y);
//                    else
//                        outSink.append(x+width*y);
                        
//                }
                QVector<int> binsForDim = dimensions.getBinsForDimensionsOfPixel(x,y)[0];
               
                if (binsForDim.size() >0)
                {
                    int onSource = 0;
                    int onSink = 0;
                    foreach (int bin, binsForDim)
                    {
                        for (int around=-5; around<=5; around++)
                        {
                            int index = indexer.getIndex(x,y,mod(bin+around,dimensions.getBinNums()[0]));
                            
                            if (g->what_segment(index) == GraphType::SOURCE)
                                onSource++;
                            else
                                onSink++;
                        }
                    }
                    if (onSource/(1.0*onSource+onSink)>.3)
                        outSource.append(x+width*y);
                    if (onSink/(1.0*onSource+onSink)>.3)
                        outSink.append(x+width*y);
                    
//                    if (onSource && onSink)
//                        printf("overlap occurred\n");
                }
                else
                {
                    bool onSource = false;
                    bool onSink = false;
                    for (int s=0; (!onSource || !onSink) && s<dimensions.getBinNums()[0]; s++)
                    {
                        int index = indexer.getIndex(x,y,s);
                        if (g->what_segment(index) == GraphType::SOURCE)
                            onSource=true;
                        else
                            onSink=true;
                    }
                    if (onSource)
                        outSource.append(x+width*y);
                    if (onSink)
                        outSink.append(x+width*y);
                }
            }
            else
            {
                int source_count = 0;
                int sink_count = 0;
                for (int s=0; s<dimensions.getBinNums()[0]; s++)
                {
                    int index = indexer.getIndex(x,y,s);
                    if (g->what_segment(index) == GraphType::SOURCE)
                        source_count++;
                    else
                        sink_count++;
                }
                if (source_count >= sink_count)
                    outSource.append(x+width*y);
                else
                    outSink.append(x+width*y);
            }
            
        }
    }
    
    
    ///test///
    for (int s=0; s<dimensions.getBinNums()[0]; s++)
    {
        BImage test(img.width(), img.height());
        BPartition tmp1((BPixelCollection*) &test);
        BPartition tmp2((BPixelCollection*) &test);
        for (int x=0; x<width; x++)
        {
            for (int y=0; y<height; y++)
            {
                QVector<int> binsForDim = dimensions.getBinsForDimensionsOfPixel(x,y)[0];
                foreach (int bin, binsForDim)
                {
                    if (bin<s+11 && bin>s-11)
                        test.setPixel(x,y,true);
                }
                    int index = indexer.getIndex(x,y,s);
                    if (g->what_segment(index) == GraphType::SOURCE)
                        tmp1.addPixelFromSrc(x,y);
                    else
                        tmp2.addPixelFromSrc(x,y);           
            }
        }
        test.claimOwnership(&tmp2,1);
        test.claimOwnership(&tmp1,1);
        
        QString debugfile = "./output/layer_";
        QString num;
        num.setNum(s);
        debugfile.append(num);
        debugfile.append(".ppm");
        test.saveOwners(debugfile);
        
    }
    ///test///
    
    
    delete g;
    return ret;
}

inline double slopeMultiplier(int x1, int y1, int x2, int y2, const QVector<QVector<double> > &slopes)
{
    double CONST = 10;
    if (slopes[x1][y1]<0 || slopes[x2][y2]<0)
        return 1/(1+CONST*HALF_PI);
    double slope_dif = abs(slopes[x1][y1]-slopes[x2][y2]);
    if (slope_dif > HALF_PI)
        slope_dif -= HALF_PI;
    
//    printf("(%d,%d) (%d,%d) slope dif=%f, slope mult=%f\n",x1,y1,x2,y2,slope_dif,1/(1+CONST*slope_dif));
    return 1/(1+CONST*slope_dif);
}

/*
    Precondition: I'm assuming that slopes[] is actually the angle in radians [0, pi)
    This doesn't work all that well
*/
int GraphCut::pixelsOfSeparationWithSlope(int* invDistMap, int width, int height, BPixelCollection &img, const QVector<QVector<double> > &slopes, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight, int split_method, int vert_divide)
{
    typedef Graph<int,int,int> GraphType;
    GraphType *g = new GraphType(width*height, 4*(width-1)*(height-1)-(height+width)); 
    
    for (int i=0; i<width*height; i++)
    {
        g->add_node();
    }
    
//    QImage debug = img.copy(0,0,img.width(),img.height());
    
    if (split_method == SPLIT_HORZ)
    {
        
        int count_source=15;
        
        
        //diag method

        for (int o=0; o<width && count_source>0; o++)
        {
            for (int i=0; i<=o && i<height && count_source>0; i++)
            {
                if (img.pixel(o-i,i))
                {
                    int index = (o-i)+width*i;
                    g -> add_tweights(index, INT_POS_INFINITY,0);
                    count_source--;

                }
            }
        }
        
        int count_sink=15;
        

        
        //diag method
        for (int o=0; o<width && count_sink>0; o++)
        {
            for (int i=0; i<=o && i<height && count_sink>0; i++)
            {
                //debug.setPixel((width-1)-(o-i),(height-1)-i,220);
                if (img.pixel((width-1)-(o-i),i))
                {
                    int index = ((width-1)-(o-i))+width*(i);
//                    g -> add_tweights(index, 0, anchor_weight);
                    g -> add_tweights(index, 0, anchor_weight);
                    count_sink--;

                }
            }
        }
    }
    
    //printf("num source:%d, num sink:%d\n",count_source,count_sink);
    
    double BLACK_TO_BLACK_V_BIAS = 2;
    double BLACK_TO_BLACK_H_BIAS = 2;
    double BLACK_TO_BLACK_D_BIAS = sqrt(pow(BLACK_TO_BLACK_V_BIAS,2)+pow(BLACK_TO_BLACK_H_BIAS,2));
    double WHITE_TO_BLACK_BIAS = .5;
    double BLACK_TO_WHITE_BIAS = .5;
    double WHITE_TO_WHITE_V_BIAS = 1;
    double WHITE_TO_WHITE_H_BIAS = .333;
    double WHITE_TO_WHITE_D_BIAS = .5;
    
    if (split_method==SPLIT_VERT)
    {
        BLACK_TO_BLACK_V_BIAS = 1.25;
        BLACK_TO_BLACK_H_BIAS = 1.25;
        BLACK_TO_BLACK_D_BIAS = sqrt(pow(BLACK_TO_BLACK_V_BIAS,2)+pow(BLACK_TO_BLACK_H_BIAS,2));
        WHITE_TO_BLACK_BIAS = .5;
        BLACK_TO_WHITE_BIAS = .5;
        WHITE_TO_WHITE_V_BIAS = .5;
        WHITE_TO_WHITE_H_BIAS = .5;
        WHITE_TO_WHITE_D_BIAS = sqrt(pow(WHITE_TO_WHITE_V_BIAS,2)+pow(WHITE_TO_WHITE_H_BIAS,2));
    }
    else if (split_method==CHOP_TOP)
    {
        BLACK_TO_BLACK_V_BIAS = 1.4;//1.85;
        BLACK_TO_BLACK_H_BIAS = 2.1;//1.75;
        BLACK_TO_BLACK_D_BIAS = sqrt(pow(BLACK_TO_BLACK_V_BIAS,2)+pow(BLACK_TO_BLACK_H_BIAS,2));
        WHITE_TO_BLACK_BIAS = .5;
        BLACK_TO_WHITE_BIAS = .5;
        WHITE_TO_WHITE_V_BIAS = .5;
        WHITE_TO_WHITE_H_BIAS = .5;
        WHITE_TO_WHITE_D_BIAS = sqrt(pow(WHITE_TO_WHITE_V_BIAS,2)+pow(WHITE_TO_WHITE_H_BIAS,2));
    }
    
    double reducer = 1;
    
    //connect all pixels
    for (int i=0; i<width; i++)
    {
        for (int j=0; j<height; j++)
        {   
            if (split_method==SPLIT_VERT)
            {
                if (j<vert_divide)
                {
                    reducer = ((vert_divide + vert_divide-j)/(double)(2*vert_divide));
                }
                else
                {
                    reducer = ((((height-1) -(double)vert_divide) + j-vert_divide)/(2.*((height-1) -(double)vert_divide)));
                }
            }
            else if (split_method==CHOP_TOP)
            {
                   reducer = ((3.0*height-j)/(double)(3*height));
            }
            
            
            if (i+1<width)
            {
                double slope_mult = slopeMultiplier(i,j,i+1,j,slopes);
                
                if (img.pixel(i,j) && img.pixel(i+1,j))
                    g -> add_edge(i+j*width, (i+1)+j*width,
                                  (invDistMap[(i+1)+j*width]+invDistMap[i+j*width])*(slope_mult*reducer*BLACK_TO_BLACK_H_BIAS),
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+j*width])*(slope_mult*reducer*BLACK_TO_BLACK_H_BIAS));
                else if (img.pixel(i,j) && !img.pixel(i+1,j))
                    g -> add_edge(i+j*width, (i+1)+j*width,
                                  (invDistMap[(i+1)+j*width]+invDistMap[i+j*width])*(BLACK_TO_WHITE_BIAS),
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+j*width])*(WHITE_TO_BLACK_BIAS));
                else if (!img.pixel(i,j) && img.pixel(i+1,j))
                    g -> add_edge(i+j*width, (i+1)+j*width,
                                  (invDistMap[(i+1)+j*width]+invDistMap[i+j*width])*(WHITE_TO_BLACK_BIAS),
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+j*width])*(BLACK_TO_WHITE_BIAS));
                else
                    g -> add_edge(i+j*width, (i+1)+j*width,
                                  (invDistMap[(i+1)+j*width]+invDistMap[i+j*width])*WHITE_TO_WHITE_V_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+j*width])*WHITE_TO_WHITE_V_BIAS);
            }
            
            if (j+1<height)
            {
                double slope_mult = slopeMultiplier(i,j,i,j+1,slopes);
                
                if (img.pixel(i,j) && img.pixel(i,j+1))
                    g -> add_edge(i+j*width, i+(j+1)*width,
                                  (invDistMap[i+(j+1)*width]+invDistMap[i+j*width])*slope_mult*reducer*BLACK_TO_BLACK_V_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[i+(j+1)*width])*slope_mult*reducer*BLACK_TO_BLACK_V_BIAS);
                else if (img.pixel(i,j) && !img.pixel(i,j+1))
                    g -> add_edge(i+j*width, i+(j+1)*width,
                                  (invDistMap[i+(j+1)*width]+invDistMap[i+j*width])*BLACK_TO_WHITE_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[i+(j+1)*width])*WHITE_TO_BLACK_BIAS);
                else if (!img.pixel(i,j) && img.pixel(i,j+1))
                    g -> add_edge(i+j*width, i+(j+1)*width,
                                  (invDistMap[i+(j+1)*width]+invDistMap[i+j*width])*WHITE_TO_BLACK_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[i+(j+1)*width])*BLACK_TO_WHITE_BIAS);
                else
                    g -> add_edge(i+j*width, i+(j+1)*width,
                                  (invDistMap[i+(j+1)*width]+invDistMap[i+j*width])*WHITE_TO_WHITE_H_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[i+(j+1)*width])*WHITE_TO_WHITE_H_BIAS);
            }
            
            if (j>0 && i<width-1)
            {
                double slope_mult = slopeMultiplier(i,j,i+1,j-1,slopes);
                
                if (img.pixel(i,j) && img.pixel(i+1,j-1))
                    g -> add_edge(i+j*width, (i+1)+(j-1)*width,
                                  (invDistMap[(i+1)+(j-1)*width]+invDistMap[i+j*width])*slope_mult*reducer*BLACK_TO_BLACK_D_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j-1)*width])*slope_mult*reducer*BLACK_TO_BLACK_D_BIAS);
                else if (img.pixel(i,j) && !img.pixel(i+1,j-1))
                    g -> add_edge(i+j*width, (i+1)+(j-1)*width,
                                  (invDistMap[(i+1)+(j-1)*width]+invDistMap[i+j*width])*BLACK_TO_WHITE_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j-1)*width])*WHITE_TO_BLACK_BIAS);
                else if (!img.pixel(i,j) && img.pixel(i+1,j-1))
                    g -> add_edge(i+j*width, (i+1)+(j-1)*width,
                                  (invDistMap[(i+1)+(j-1)*width]+invDistMap[i+j*width])*WHITE_TO_BLACK_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j-1)*width])*BLACK_TO_WHITE_BIAS);
                else
                    g -> add_edge(i+j*width, (i+1)+(j-1)*width,
                                  (invDistMap[(i+1)+(j-1)*width]+invDistMap[i+j*width])*WHITE_TO_WHITE_D_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j-1)*width])*WHITE_TO_WHITE_D_BIAS);
            }
            
            if (j<height-1 && i<width-1)
            {
                double slope_mult = slopeMultiplier(i,j,i+1,j+1,slopes);
                
                if (img.pixel(i,j) && img.pixel(i+1,j+1))
                    g -> add_edge(i+j*width, (i+1)+(j+1)*width,
                                  (invDistMap[(i+1)+(j+1)*width]+invDistMap[i+j*width])*slope_mult*reducer*BLACK_TO_BLACK_D_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j+1)*width])*slope_mult*reducer*BLACK_TO_BLACK_D_BIAS);
                else if (img.pixel(i,j) && !img.pixel(i+1,j+1))
                    g -> add_edge(i+j*width, (i+1)+(j+1)*width,
                                  (invDistMap[(i+1)+(j+1)*width]+invDistMap[i+j*width])*BLACK_TO_WHITE_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j+1)*width])*WHITE_TO_BLACK_BIAS);
                else if (!img.pixel(i,j) && img.pixel(i+1,j+1))
                    g -> add_edge(i+j*width, (i+1)+(j+1)*width,
                                  (invDistMap[(i+1)+(j+1)*width]+invDistMap[i+j*width])*WHITE_TO_BLACK_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j+1)*width])*BLACK_TO_WHITE_BIAS);
                else
                    g -> add_edge(i+j*width, (i+1)+(j+1)*width,
                                  (invDistMap[(i+1)+(j+1)*width]+invDistMap[i+j*width])*WHITE_TO_WHITE_D_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j+1)*width])*WHITE_TO_WHITE_D_BIAS);
            }
        }
    }
    
    int ret = g -> maxflow();
    
    //add all black pixels which
    for (int index=0; index<width*height; index++)
    {
        if (img.pixelIsMine(index%width,index/width))
        {
if (g->what_segment(index) == GraphType::SOURCE)
            {
                outSource.append(index);
            }
            else
            {
                outSink.append(index);
            }
//        debug2.setPixel(index%width,index/width,rw);
        }
    }
    

    
    delete g;
    return ret;
}
//////////////////////


int GraphCut::pixelsOfSeparationNoDistMap(int* invDistMap, int width, int height, BPixelCollection &img, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight, int split_method, int vert_divide)
{
    anchor_weight=1000;
    assert(invDistMap==NULL);
    invDistMap = new int[width*height];
    GraphType *g = new GraphType(width*height, 4*(width-1)*(height-1)-(height+width)); 
    
    for (int i=0; i<width*height; i++)
    {
        g->add_node();
        invDistMap[i] = 1;
    }
    
//    QImage debug = img.copy(0,0,img.width(),img.height());
    
    if (split_method == SPLIT_HORZ)
    {
        //find source pixels
        int count_source = height*ANCHOR_L;
        for (int i=0; count_source>0 && i<width; i++)
        {
            for (int j=0; count_source>0 && j<height; j++)
            {
                if (img.pixel(i,j))
                {
                    int index = i+width*j;
                    g -> add_tweights(index, anchor_weight,0);//invDistMap[index], 0);
    //                debug.setPixel(i,j,150);
                    count_source--;
                }
            }
        }

        
        int count_sink=height*ANCHOR_R;

        
        //diag method
        for (int o=0; o<width && count_sink>0; o++)
        {
            for (int i=0; i<=o && i<height && count_sink>0; i++)
            {
                //debug.setPixel((width-1)-(o-i),(height-1)-i,220);
                if (img.pixel((width-1)-(o-i),(height-1)-i))
                {
                    int index = ((width-1)-(o-i))+width*((height-1)-i);
                    g -> add_tweights(index, 0, anchor_weight);
                    count_sink--;

                }
            }
        }
    }
    else if (split_method == SPLIT_VERT)
    {
        //all pixels are either source or sink
        for (int j=0; j<vert_divide; j++)
        {
            double anchor_weight_for_level = anchor_weight * ((2.5*vert_divide-j)/(double)(2.5*vert_divide));
            //printf("%f, ",((vert_divide-j)/(double)vert_divide));
            for (int i=0; i<width; i++)
            {
                if (img.pixel(i,j))
                {
                    int index = i+width*j;
                    g -> add_tweights(index, (int)anchor_weight_for_level,0);
    //                debug.setPixel(i,j,150);
                }
            }
        }
        for (int j=height-1; j>=vert_divide; j--)
        {
            double anchor_weight_for_level = anchor_weight * ((((height-1) -(double)vert_divide) + j-vert_divide)/(2.*((height-1) -(double)vert_divide)));
            //printf("%f- ",((j-vert_divide)/((height-1) -vert_divide)));
            for (int i=0; i<width; i++)
            {
                if (img.pixel(i,j))
                {
                    int index = i+width*j;
                    g -> add_tweights(index, 0, (int)anchor_weight_for_level);
    //                debug.setPixel(i,j,150);
                }
            }
        }
    }
    else if (split_method == CHOP_TOP)
    {
        //all pixels are source and sink
        for (int j=0; j<height; j++)
        {
            double src_anchor_weight_for_level = anchor_weight * (((height-1)-j)/(double)(height-1));
            double sink_anchor_weight_for_level = /*.3 * anchor_weight;*/(anchor_weight/2.0) * (j/(double)(height-1));
            for (int i=0; i<width; i++)
            {
//                if (img.pixel(i,j))
                {
                    int index = i+width*j;
                    g -> add_tweights(index, (int)src_anchor_weight_for_level,(int)sink_anchor_weight_for_level);
    //                debug.setPixel(i,j,150);
                }
            }
        }
    }
    
    //printf("num source:%d, num sink:%d\n",count_source,count_sink);
    
    double BLACK_TO_BLACK_V_BIAS = 100;
    double BLACK_TO_BLACK_H_BIAS = 200;
    double BLACK_TO_BLACK_D_BIAS = sqrt(pow(BLACK_TO_BLACK_V_BIAS,2)+pow(BLACK_TO_BLACK_H_BIAS,2));
    double WHITE_TO_BLACK_BIAS = .5;
    double BLACK_TO_WHITE_BIAS = .5;
    double WHITE_TO_WHITE_V_BIAS = 1;
    double WHITE_TO_WHITE_H_BIAS = .333;
    double WHITE_TO_WHITE_D_BIAS = sqrt(pow(WHITE_TO_WHITE_V_BIAS,2)+pow(WHITE_TO_WHITE_H_BIAS,2));
    
    if (split_method==SPLIT_VERT)
    {
        BLACK_TO_BLACK_V_BIAS = 1.25;
        BLACK_TO_BLACK_H_BIAS = 1.25;
        BLACK_TO_BLACK_D_BIAS = sqrt(pow(BLACK_TO_BLACK_V_BIAS,2)+pow(BLACK_TO_BLACK_H_BIAS,2));
        WHITE_TO_BLACK_BIAS = .5;
        BLACK_TO_WHITE_BIAS = .5;
        WHITE_TO_WHITE_V_BIAS = .5;
        WHITE_TO_WHITE_H_BIAS = .5;
        WHITE_TO_WHITE_D_BIAS = sqrt(pow(WHITE_TO_WHITE_V_BIAS,2)+pow(WHITE_TO_WHITE_H_BIAS,2));
    }
    else if (split_method==CHOP_TOP)
    {
        BLACK_TO_BLACK_V_BIAS = 1.4;//1.85;
        BLACK_TO_BLACK_H_BIAS = 2.1;//1.75;
        BLACK_TO_BLACK_D_BIAS = sqrt(pow(BLACK_TO_BLACK_V_BIAS,2)+pow(BLACK_TO_BLACK_H_BIAS,2));
        WHITE_TO_BLACK_BIAS = .5;
        BLACK_TO_WHITE_BIAS = .5;
        WHITE_TO_WHITE_V_BIAS = .5;
        WHITE_TO_WHITE_H_BIAS = .5;
        WHITE_TO_WHITE_D_BIAS = sqrt(pow(WHITE_TO_WHITE_V_BIAS,2)+pow(WHITE_TO_WHITE_H_BIAS,2));
    }
    
    double reducer = 1;
    
    //connect all pixels
    for (int i=0; i<width; i++)
    {
        for (int j=0; j<height; j++)
        {   
            if (split_method==SPLIT_VERT)
            {
                if (j<vert_divide)
                {
                    reducer = ((vert_divide + vert_divide-j)/(double)(2*vert_divide));
                }
                else
                {
                    reducer = ((((height-1) -(double)vert_divide) + j-vert_divide)/(2.*((height-1) -(double)vert_divide)));
                }
            }
            else if (split_method==CHOP_TOP)
            {
                   reducer = ((3.0*height-j)/(double)(3*height));
            }
            
            
            if (i+1<width)
            {
                setEdge(i,j,i+1,j,g,img,invDistMap,BLACK_TO_BLACK_H_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_H_BIAS,reducer,width);
            }
            
            if (j+1<height)
            {
                setEdge(i,j,i,j+1,g,img,invDistMap,BLACK_TO_BLACK_V_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_V_BIAS,reducer,width);
            }
            
            if (j>0 && i<width-1)
            {
                setEdge(i,j,i+1,j-1,g,img,invDistMap,BLACK_TO_BLACK_D_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_D_BIAS,reducer,width);
            }
            
            if (j<height-1 && i<width-1)
            {
                setEdge(i,j,i+1,j+1,g,img,invDistMap,BLACK_TO_BLACK_D_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_D_BIAS,reducer,width);
            }
        }
    }
    
    int ret = g -> maxflow();
    
//    QImage debug2 = debug.convertToFormat(QImage::Format_RGB16);
//    QRgb lw = qRgb(255, 100, 100);
//    QRgb lb = qRgb(155, 0, 0);
//    QRgb rw = qRgb(100,255, 100);
//    QRgb rb = qRgb(0, 155, 0);
//    QRgb a = qRgb(255, 255, 255);
    
    //add all black pixels which
    for (int index=0; index<width*height; index++)
    {
        if (img.pixelIsMine(index%width,index/width))
        {
            /*if (qGray(debug.pixel(index%width,index/width))!=BLACK && qGray(debug.pixel(index%width,index/width))!=WHITE)
            {
                debug2.setPixel(index%width,index/width,a);
            }
            else*/ if (g->what_segment(index) == GraphType::SOURCE)
            {
                outSource.append(index);
    //            debug2.setPixel(index%width,index/width,lb);
            }
            else
            {
                outSink.append(index);
            }
    //        else if (g->what_segment(index) == GraphType::SOURCE)
    //            debug2.setPixel(index%width,index/width,lw);
    //        else if (qGray(img.pixel(index%width,index/width))==BLACK)
    //            debug2.setPixel(index%width,index/width,rb);
    //        else
    //            debug2.setPixel(index%width,index/width,rw);
        }
    }
    
//    QString debugfile = "./cut_";
//    QString num;
//    num.setNum(width);
//    debugfile.append(num);
//    debugfile.append(".ppm");
//    debug2.save(debugfile);
    
    delete g;
    return ret;
}


///exp
int GraphCut::pixelsOfSeparationExperimental(int* invDistMap, int width, int height, const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight, int split_method, int vert_divide)
{
    typedef Graph<int,int,int> GraphType;
    GraphType *g = new GraphType(width*height, 4*(width-1)*(height-1)-(height+width)); 
    
    for (int i=0; i<width*height; i++)
    {
        g->add_node();
    }
    
    QImage debug = img.makeImage().getImage();
    QVector<QRgb> ct = debug.colorTable();
    ct.append(qRgb(205,50,50));
    ct.append(qRgb(50,205,50));
    debug.setColorTable(ct);
    
    //find source pixels
    int count_source = NEW_ANCHOR;
    //fill
    BImage mark = img.makeImage();
    
    QVector<QPoint> workingStack;
    foreach (QPoint seed, sourceSeeds)
    {
        workingStack.push_back(seed);
        mark.setPixel(seed,false);
    }
    while (!workingStack.isEmpty() && count_source>0)
    {   
        QPoint cur = workingStack.front();
        workingStack.pop_front();
        int index = cur.x()+width*cur.y();
        g -> add_tweights(index, anchor_weight, 0);
//                g -> add_tweights(index, 0, anchor_weight);
        debug.setPixel(cur,2);
        count_source--;
        
        
        
        if (cur.x()<mark.width()-1 && mark.pixel(cur.x()+1,cur.y()))
        {
            QPoint pp(cur.x()+1,cur.y());
            workingStack.push_back(pp);
            mark.setPixel(pp,false);
            
        }
        if (cur.x()>0 && mark.pixel(cur.x()-1,cur.y()))
        {
            QPoint pp(cur.x()-1,cur.y());
            workingStack.push_back(pp);
            mark.setPixel(pp,false);
        }
        if (cur.y()>0 && mark.pixel(cur.x(),cur.y()-1))
        {
            QPoint pp(cur.x(),cur.y()-1);
            workingStack.push_back(pp);
            mark.setPixel(pp,false);
        }
        if (cur.y()<mark.height()-1 && mark.pixel(cur.x(),cur.y()+1))
        {
            QPoint pp(cur.x(),cur.y()+1);
            workingStack.push_back(pp);
            mark.setPixel(pp,false);
        }
    }
    
    int count_sink=NEW_ANCHOR;//height*ANCHOR_R;
    
    //find sink pixels
    //fill
    workingStack.clear();
    foreach (QPoint seed, sinkSeeds)
    {
        workingStack.push_back(seed);
        mark.setPixel(seed,false);
    }
    while (!workingStack.isEmpty() && count_sink>0)
    {   
        QPoint cur = workingStack.front();
        workingStack.pop_front();
        int index = cur.x()+width*cur.y();
        g -> add_tweights(index, 0, anchor_weight);
//                g -> add_tweights(index, anchor_weight,0 );
        debug.setPixel(cur,3);
        count_sink--;
        
        
        
        if (cur.x()<mark.width()-1 && mark.pixel(cur.x()+1,cur.y()))
        {
            QPoint pp(cur.x()+1,cur.y());
            workingStack.push_back(pp);
            mark.setPixel(pp,false);
            
        }
        if (cur.x()>0 && mark.pixel(cur.x()-1,cur.y()))
        {
            QPoint pp(cur.x()-1,cur.y());
            workingStack.push_back(pp);
            mark.setPixel(pp,false);
        }
        if (cur.y()>0 && mark.pixel(cur.x(),cur.y()-1))
        {
            QPoint pp(cur.x(),cur.y()-1);
            workingStack.push_back(pp);
            mark.setPixel(pp,false);
        }
        if (cur.y()<mark.height()-1 && mark.pixel(cur.x(),cur.y()+1))
        {
            QPoint pp(cur.x(),cur.y()+1);
            workingStack.push_back(pp);
            mark.setPixel(pp,false);
        }
    }
    
    
    debug.save("./anchors.ppm");
    
    
    
    //connect all pixels
    //For simplicity, only doing three dimensions now
    double WEIGHT = .5;
    double D_WEIGHT = (2*WEIGHT)-sqrt(2*pow(WEIGHT,2));
        
    for (int i=0; i<width; i++)
    {
        for (int j=0; j<height; j++)
        {   
            
            //C1
            if (i+1<width)
            {
                g -> add_edge(i+width*j, i+1+width*j,
                              (invDistMap[i+width*j]+invDistMap[i+1+width*j])*WEIGHT,
                              (invDistMap[i+width*j]+invDistMap[i+1+width*j])*WEIGHT);
            }
            
            if (j+1<height)
            {
                g -> add_edge(i+width*j, i+width*(1+j),
                              (invDistMap[i+width*j]+invDistMap[i+width*(1+j)])*WEIGHT,
                              (invDistMap[i+width*j]+invDistMap[i+width*(1+j)])*WEIGHT);
            }
            
            
            //C2 
            if (j>0 && i<width-1)
            {
                g -> add_edge(i+width*j, i+1+width*(j-1),
                              (invDistMap[i+width*j]+invDistMap[i+1+width*(j-1)])*D_WEIGHT,
                              (invDistMap[i+width*j]+invDistMap[i+1+width*(j-1)])*D_WEIGHT);
                
            }
            
            if (j<height-1 && i<width-1)
            {
                g -> add_edge(i+width*j, 1+i+width*(1+j),
                              (invDistMap[i+width*j]+invDistMap[1+i+width*(1+j)])*D_WEIGHT,
                              (invDistMap[i+width*j]+invDistMap[1+i+width*(1+j)])*D_WEIGHT);
            }
            
            
            
        }//j
    }//i
    
    int ret = g -> maxflow();

    
    //add all black pixels which
    
    for (int x=0; x<width; x++)
    {
        for (int y=0; y<height; y++)
        {
            
//            if (img.pixel(x,y))
            {
                int index = x+width*y;
                
                if (g->what_segment(index) == GraphType::SOURCE)
                    outSource.append(index);
                else
                    outSink.append(index);
            }
            
        }
    }
    
    
    
    delete g;
    return ret;
}

/////////////////////////////////////
inline QVector<QVector<QVector<double> > > make3dImage(const BPixelCollection &img,int* invDistMap,const AngleImage &angleImage)
{
    QVector<QVector<QVector<double> > > ret(img.width());
    
    int slopeDifRange = SLOPE_DIF_TOLERANCE*angleImage.getNumOfBins();
//    printf("difrange:%d\n",slopeDifRange);
    for (int x=0; x<img.width(); x++)
    {
        QVector<QVector<double> > flat(img.height());
        for (int y=0; y<img.height(); y++)
        {
            QVector<double> slope(angleImage.getNumOfBins());
            
            if (img.pixel(x,y))
            {
                QMap<int,double> binsAndStrForDim = angleImage.getBinsAndStrForPixel(x,y);
                if (binsAndStrForDim.size()>0)
                {
                    for (int k=0; k<angleImage.getNumOfBins(); k++)
                    {
                        slope[k]=INV_A;//'a' from inv dist map computation
                    }
                    foreach (int bin, binsAndStrForDim.keys())
                    {
                        double strength = binsAndStrForDim[bin];
                        double initVal = invDistMap[x+y*img.width()] * strength;
                        
                        slope[bin]=initVal;
                        for (int kb=1; kb<slopeDifRange; kb++)
                        {
                            slope[mod((bin+kb),angleImage.getNumOfBins())] = std::max((int)(slope[mod((bin+kb),angleImage.getNumOfBins())]), (int) (initVal * (slopeDifRange-kb)/(1.0*slopeDifRange)));
                            slope[mod((bin-kb),angleImage.getNumOfBins())] = std::max((int) (slope[mod((bin-kb),angleImage.getNumOfBins())]), (int) (initVal * (slopeDifRange-kb)/(1.0*slopeDifRange)));
                        }
                    }
                    
                }
                else
                {
                    for (int k=0; k<angleImage.getNumOfBins(); k++)
                    {
                        slope[k]=invDistMap[x+y*img.width()];
                    }
                }
            }
            else
            {
                for (int k=0; k<angleImage.getNumOfBins(); k++)
                {
                    slope[k]=invDistMap[x+y*img.width()];
                }
            }
            flat[y]=slope;
        }
        ret[x]=flat;
    }
    
    ///test///
    double testMax=0;
    for (int x=0; x<img.width(); x++)
    {
        for (int y=0; y<img.height(); y++)
        {
            for (int s=0; s<angleImage.getNumOfBins(); s++)
            {
                if (ret[x][y][s] > testMax)
                    testMax=ret[x][y][s];
            }
        }
    }
    printf("testMax=%f\n",testMax);
    QVector<QRgb> default_color_table;
    for (int i=0; i<256; i++)
    {
        default_color_table.append(qRgb(i,i,i));
    }
    for (int s=0; s<angleImage.getNumOfBins(); s++)
    {
        QImage test(img.width(),img.height(),QImage::Format_Indexed8);
        test.setColorTable(default_color_table);
        for (int x=0; x<img.width(); x++)
        {
            for (int y=0; y<img.height(); y++)
            {
                test.setPixel(x,y,ret[x][y][s]*(255.0/testMax));
            }
        }
        QString debugfile = "./dist_3d/layer_";
        QString num;
        num.setNum(s);
        debugfile.append(num);
        debugfile.append(".ppm");
        test.save(debugfile);
    }
    ///test///
    
    return ret;
}

inline void setEdge3d(int x1, int y1, int slope1, int x2, int y2, int slope2, GraphType* g, const Indexer3D &indexer, const QVector<QVector<QVector<double> > > &image3d, double weight)
{
        g -> add_edge(indexer.getIndex(x1,y1,slope1), indexer.getIndex(x2,y2,slope2),
                      (image3d[x1][y1][slope1]+image3d[x2][y2][slope2])*weight,
                      (image3d[x1][y1][slope1]+image3d[x2][y2][slope2])*weight);
}

int GraphCut::pixelsOfSeparationOld3D(int* invDistMap, int width, int height, const BPixelCollection &img, const AngleImage &angleImage, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight, int split_method, int vert_divide)
{
    
    QVector<QVector<QVector<double> > > image3d = make3dImage(img,invDistMap,angleImage);
    
    
    
    int numNodes = width*height*angleImage.getNumOfBins();
    int numEdges = 4*(width-1)*(height-1)-(height+width);
    numEdges *= angleImage.getNumOfBins();
    numEdges += numNodes*(angleImage.getNumOfBins()-1);//assuming only striaght (no diag) connections for higher dimensions
    
    Indexer3D indexer(width, height);
    
    typedef Graph<int,int,int> GraphType;
    GraphType *g = new GraphType(numNodes, numEdges); 
    
    for (int i=0; i<numNodes; i++)
    {
        g->add_node();
    }
    
    QImage debug = img.makeImage().getImage();
    QVector<QRgb> ct = debug.colorTable();
    ct.append(qRgb(205,50,50));
    ct.append(qRgb(50,205,50));
    debug.setColorTable(ct);
    
    if (split_method == SPLIT_HORZ)
    {
        //find source pixels
        int count_source = NEW_ANCHOR;
        //fill
        BImage mark = img.makeImage();
        QVector<QPoint> workingStack;
        foreach (QPoint seed, sourceSeeds)
        {
            workingStack.push_back(seed);
            mark.setPixel(seed,false);
        }
        while (!workingStack.isEmpty() && count_source>0)
        {   
            QPoint cur = workingStack.front();
            workingStack.pop_front();
            for (int k=0; k<angleImage.getNumOfBins(); k++)
            {
                int index = indexer.getIndex(cur.x(),cur.y(),k);
                g -> add_tweights(index, anchor_weight, 0);
//                g -> add_tweights(index, 0, anchor_weight);
            }
            debug.setPixel(cur,2);
            count_source--;
            
            
            
            if (cur.x()<mark.width()-1 && mark.pixel(cur.x()+1,cur.y()))
            {
                QPoint pp(cur.x()+1,cur.y());
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
                
            }
            if (cur.x()>0 && mark.pixel(cur.x()-1,cur.y()))
            {
                QPoint pp(cur.x()-1,cur.y());
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()>0 && mark.pixel(cur.x(),cur.y()-1))
            {
                QPoint pp(cur.x(),cur.y()-1);
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()<mark.height()-1 && mark.pixel(cur.x(),cur.y()+1))
            {
                QPoint pp(cur.x(),cur.y()+1);
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
        }
        
        int count_sink=NEW_ANCHOR;//height*ANCHOR_R;
        
        //find sink pixels

        //fill
        workingStack.clear();
        foreach (QPoint seed, sinkSeeds)
        {
            workingStack.push_back(seed);
            mark.setPixel(seed,false);
        }
        while (!workingStack.isEmpty() && count_sink>0)
        {   
            QPoint cur = workingStack.front();
            workingStack.pop_front();
            for (int k=0; k<angleImage.getNumOfBins(); k++)
            {
                int index = indexer.getIndex(cur.x(),cur.y(),k);
                g -> add_tweights(index, 0, anchor_weight);
//                g -> add_tweights(index, anchor_weight,0 );
                
            }
            debug.setPixel(cur,3);
            count_sink--;
            
            
            
            if (cur.x()<mark.width()-1 && mark.pixel(cur.x()+1,cur.y()))
            {
                QPoint pp(cur.x()+1,cur.y());
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
                
            }
            if (cur.x()>0 && mark.pixel(cur.x()-1,cur.y()))
            {
                QPoint pp(cur.x()-1,cur.y());
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()>0 && mark.pixel(cur.x(),cur.y()-1))
            {
                QPoint pp(cur.x(),cur.y()-1);
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()<mark.height()-1 && mark.pixel(cur.x(),cur.y()+1))
            {
                QPoint pp(cur.x(),cur.y()+1);
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
        }
        
        
        debug.save("./anchors.ppm");
    }
    
    
    
    //connect all pixels
    //For simplicity, only doing three dimensions now
//    double FLAT_WEIGHT = .5;
//    double SLOPE_WEIGHT = 4;
    double FLAT_WEIGHT = 6;//1
    double SLOPE_WEIGHT = .5;
    int slope_size = angleImage.getNumOfBins();
    for (int k=0; k<slope_size; k++)
    {
        
        for (int i=0; i<width; i++)
        {
            for (int j=0; j<height; j++)
            {   
                
                //C1
                if (i+1<width)
                {
                    setEdge3d(i,j,k,i+1,j,k,g,indexer,image3d,FLAT_WEIGHT);
                }
                
                if (j+1<height)
                {
                    setEdge3d(i,j,k,i,j+1,k,g,indexer,image3d,FLAT_WEIGHT);
                }
                
                if (k+1<slope_size)
                {
                    setEdge3d(i,j,k,i,j,k+1,g,indexer,image3d,SLOPE_WEIGHT);
                }
                else//fold
                {
                    setEdge3d(i,j,k,i,j,0,g,indexer,image3d,SLOPE_WEIGHT);
                }
                
                //C2 dumbed down
                if (j>0 && i<width-1)
                {
                    setEdge3d(i,j,k,i+1,j-1,k,g,indexer,image3d,2*FLAT_WEIGHT-sqrt(2*pow(FLAT_WEIGHT,2)));
                }
                
                if (j<height-1 && i<width-1)
                {
                    setEdge3d(i,j,k,i+1,j+1,k,g,indexer,image3d,2*FLAT_WEIGHT-sqrt(2*pow(FLAT_WEIGHT,2)));
                }
                
                
                
                //C3 nope
                
            }//j
        }//i
    }//k
    
    int ret = g -> maxflow();

    
    //add all black pixels which
    int slopeDifRange = SLOPE_DIF_TOLERANCE*angleImage.getNumOfBins();
    for (int x=0; x<width; x++)
    {
        for (int y=0; y<height; y++)
        {
            
            if (img.pixel(x,y))
            {
                QMap<int,double> binsForDim = angleImage.getBinsAndStrForPixel(x,y);
               
                if (binsForDim.size() >0)
                {
//                    int onSource = 0;
//                    int onSink = 0;
//                    foreach (int bin, binsForDim.keys())
//                    {
//                        for (int around=-5; around<=5; around++)
//                        {
//                            int index = indexer.getIndex(x,y,mod(bin+around,angleImage.getNumOfBins()));
                            
//                            if (g->what_segment(index) == GraphType::SOURCE)
//                                onSource++;
//                            else
//                                onSink++;
//                        }
//                    }
//                    if (onSource/(1.0*onSource+onSink)>.1)
//                        outSource.append(x+width*y);
//                    if (onSink/(1.0*onSource+onSink)>.1)
//                        outSink.append(x+width*y);
                    double sourceScore=0;
                    double sinkScore=0;
                    foreach (int bin, binsForDim.keys())
                    {
                        double strength = binsForDim[bin];
                        int index = indexer.getIndex(x,y,mod(bin,angleImage.getNumOfBins()));
                        if (g->what_segment(index) == GraphType::SOURCE)
                            sourceScore+=strength;
                        else
                            sinkScore+=strength;
                        
                        for (int delta=1; delta<slopeDifRange; delta++)
                        {
                            index = indexer.getIndex(x,y,mod(bin+delta,angleImage.getNumOfBins()));
                            
                            if (g->what_segment(index) == GraphType::SOURCE)
                                sourceScore+=strength*(slopeDifRange-delta*1.0)/slopeDifRange;
                            else
                                sinkScore+=strength*(slopeDifRange-delta*1.0)/slopeDifRange;
                            
                            index = indexer.getIndex(x,y,mod(bin-delta,angleImage.getNumOfBins()));
                            
                            if (g->what_segment(index) == GraphType::SOURCE)
                                sourceScore+=strength*(slopeDifRange-delta*1.0)/slopeDifRange;
                            else
                                sinkScore+=strength*(slopeDifRange-delta*1.0)/slopeDifRange;
                        }
                    }
                    if (sourceScore>=.277*slopeDifRange)
                        outSource.append(x+width*y);
                    if (sinkScore>=.277*slopeDifRange)
                        outSink.append(x+width*y);
                }
                else
                {
                    bool onSource = false;
                    bool onSink = false;
                    for (int s=0; (!onSource || !onSink) && s<angleImage.getNumOfBins(); s++)
                    {
                        int index = indexer.getIndex(x,y,s);
                        if (g->what_segment(index) == GraphType::SOURCE)
                            onSource=true;
                        else
                            onSink=true;
                    }
                    if (onSource)
                        outSource.append(x+width*y);
                    if (onSink)
                        outSink.append(x+width*y);
                }
            }
            else
            {
                int source_count = 0;
                int sink_count = 0;
                for (int s=0; s<angleImage.getNumOfBins(); s++)
                {
                    int index = indexer.getIndex(x,y,s);
                    if (g->what_segment(index) == GraphType::SOURCE)
                        source_count++;
                    else
                        sink_count++;
                }
                if (source_count >= sink_count)
                    outSource.append(x+width*y);
                else
                    outSink.append(x+width*y);
            }
            
        }
    }
    
    
    ///test///
    for (int s=0; s<angleImage.getNumOfBins(); s++)
    {
        BImage test(img.width(), img.height());
        BPartition tmp1((BPixelCollection*) &test);
        BPartition tmp2((BPixelCollection*) &test);
        for (int x=0; x<width; x++)
        {
            for (int y=0; y<height; y++)
            {
                QMap<int,double> binsForDim = angleImage.getBinsAndStrForPixel(x,y);
                foreach (int bin, binsForDim.keys())
                {
                    if (bin<s+11 && bin>s-11)
                        test.setPixel(x,y,true);
                }
                    int index = indexer.getIndex(x,y,s);
                    if (g->what_segment(index) == GraphType::SOURCE)
                        tmp1.addPixelFromSrc(x,y);
                    else
                        tmp2.addPixelFromSrc(x,y);           
            }
        }
        test.claimOwnership(&tmp2,1);
        test.claimOwnership(&tmp1,1);
        
        QString debugfile = "./output/layer_";
        QString num;
        num.setNum(s);
        debugfile.append(num);
        debugfile.append(".ppm");
        test.saveOwners(debugfile);
        
    }
    ///test///
    
    
    delete g;
    return ret;
}



inline void setEdge3DMap(int x1, int y1, int slope1, int x2, int y2, int slope2, GraphType* g, const Indexer3D &indexer, const long* invDistMap3D, double weight)
{
        g -> add_edge(indexer.getIndex(x1,y1,slope1), indexer.getIndex(x2,y2,slope2),
                      (invDistMap3D[indexer.getIndex(x1,y1,slope1)]+invDistMap3D[indexer.getIndex(x2,y2,slope2)])*weight,
                      (invDistMap3D[indexer.getIndex(x1,y1,slope1)]+invDistMap3D[indexer.getIndex(x2,y2,slope2)])*weight);
}

int GraphCut::pixelsOfSeparationRecut3D(const long* invDistMap3D, int width, int height, int depth, const AngleImage &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, int crossOverY, const QVector<QPoint> &crossOverPoints, int anchor_weight, int split_method)
{
    
//    QVector<QVector<QVector<double> > > image3d = make3dImage(img,invDistMap,angleImage);
    
    
    
    int numNodes = width*height*depth;
    int numEdges = 4*(width-1)*(height-1)-(height+width);
    numEdges *= depth;
    numEdges += numNodes*(depth-1);//assuming only striaght (no diag) connections for higher dimensions
    
    Indexer3D indexer(width, height);
    
    GraphType *g = new GraphType(numNodes, numEdges); 
    
    for (int i=0; i<numNodes; i++)
    {
        g->add_node();
    }
    
    
    
    //connect all pixels
    //For simplicity, only doing three dimensions now
//    double FLAT_WEIGHT = .5;
//    double SLOPE_WEIGHT = 4;
    double FLAT_WEIGHT = 1;//1.5//6
    double SLOPE_WEIGHT = .75;//.5;
    int slope_size = depth;
    
//    int numPixAbove=width*crossOverPoint.y()*depth;
//    int numPixBelow=width*(height-crossOverPoint.y())*depth;
    
    for (int k=0; k<slope_size; k++)
    {
        
        for (int j=0; j<height; j++)
        {
#if VERT_BIAS_3D_CUT
            double anchor_weight_for_level_top = CENTER_BIAS_ANCHOR_WIEGHT * ((2.5*crossOverY-j)/(double)(2.5*crossOverY));
            double anchor_weight_for_level_bottom = CENTER_BIAS_ANCHOR_WIEGHT * ((((height-1) -(double)crossOverY) + j-crossOverY)/(2.*((height-1) -(double)crossOverY)));
#endif
            for (int i=0; i<width; i++)
            {   
                if (img.pixel(i,j))
                {
                    //C1
                    if (i+1<width && img.pixel(i+1,j))
                    {
                        setEdge3DMap(i,j,k,i+1,j,k,g,indexer,invDistMap3D,FLAT_WEIGHT);
                    }
                    
                    if (j+1<height && img.pixel(i,j+1))
                    {
                        setEdge3DMap(i,j,k,i,j+1,k,g,indexer,invDistMap3D,FLAT_WEIGHT);
                    }
                    
                    if (k+1<slope_size)
                    {
                        setEdge3DMap(i,j,k,i,j,k+1,g,indexer,invDistMap3D,SLOPE_WEIGHT);
                    }
                    else//fold
                    {
                        setEdge3DMap(i,j,k,i,j,0,g,indexer,invDistMap3D,SLOPE_WEIGHT);
                    }
                    
                    //C2 dumbed down
                    if (j>0 && i<width-1 && img.pixel(i+1,j-1))
                    {
                        setEdge3DMap(i,j,k,i+1,j-1,k,g,indexer,invDistMap3D,2*FLAT_WEIGHT-sqrt(2*pow(FLAT_WEIGHT,2)));
                    }
                    
                    if (j<height-1 && i<width-1 && img.pixel(i+1,j+1))
                    {
                        setEdge3DMap(i,j,k,i+1,j+1,k,g,indexer,invDistMap3D,2*FLAT_WEIGHT-sqrt(2*pow(FLAT_WEIGHT,2)));
                    }
                    
                    
                    
                    //C3 nope
                    
                    //extra bias
#if VERT_BIAS_3D_CUT
                    //                 QMap<int,double> bins = img.getBinsAndStrForPixel(i,j);
                    if (img.pixel(i,j)/* && bins.keys().contains(k)*/)
                    {
                        if (j<=crossOverY)
                            g -> add_tweights(indexer.getIndex(i,j,k), anchor_weight_for_level_top, 0);
                        else
                            g -> add_tweights(indexer.getIndex(i,j,k), 0, anchor_weight_for_level_bottom);
                    }
#endif
                    
                }
            }//j
        }//i
    }//k
    
    //anchoring
    QImage debug = img.makeImage().getImage();
    QVector<QRgb> ct = debug.colorTable();
    ct.append(qRgb(205,50,50));
    ct.append(qRgb(50,205,50));
    debug.setColorTable(ct);
    
    if (split_method == SPLIT_HORZ)
    {
        //find source pixels
        int count_source = NEW_ANCHOR;
        //fill
        BImage mark = img.makeImage();
        QVector<QPoint> workingStack;
        foreach (QPoint seed, sourceSeeds)
        {
            workingStack.push_back(seed);
            mark.setPixel(seed,false);
        }
        while (!workingStack.isEmpty() && count_source>0)
        {   
            QPoint cur = workingStack.front();
            workingStack.pop_front();
            for (int k=0; k<depth; k++)
            {
                int index = indexer.getIndex(cur.x(),cur.y(),k);
                g -> add_tweights(index, anchor_weight, 0);
//                g -> add_tweights(index, 0, anchor_weight);
            }
            debug.setPixel(cur,2);
            count_source--;
            
            
            
            if (cur.x()<mark.width()-1 && mark.pixel(cur.x()+1,cur.y()))
            {
                QPoint pp(cur.x()+1,cur.y());
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
                
            }
            if (cur.x()>0 && mark.pixel(cur.x()-1,cur.y()))
            {
                QPoint pp(cur.x()-1,cur.y());
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()>0 && mark.pixel(cur.x(),cur.y()-1))
            {
                QPoint pp(cur.x(),cur.y()-1);
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()<mark.height()-1 && mark.pixel(cur.x(),cur.y()+1))
            {
                QPoint pp(cur.x(),cur.y()+1);
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
        }
        
        int count_sink=NEW_ANCHOR;//height*ANCHOR_R;
        
        //find sink pixels

        //fill
        workingStack.clear();
        foreach (QPoint seed, sinkSeeds)
        {
            workingStack.push_back(seed);
            mark.setPixel(seed,false);
        }
        while (!workingStack.isEmpty() && count_sink>0)
        {   
            QPoint cur = workingStack.front();
            workingStack.pop_front();
            for (int k=0; k<depth; k++)
            {
                int index = indexer.getIndex(cur.x(),cur.y(),k);
                g -> add_tweights(index, 0, anchor_weight);
//                g -> add_tweights(index, anchor_weight,0 );
                
            }
            debug.setPixel(cur,3);
            count_sink--;
            
            
            
            if (cur.x()<mark.width()-1 && mark.pixel(cur.x()+1,cur.y()))
            {
                QPoint pp(cur.x()+1,cur.y());
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
                
            }
            if (cur.x()>0 && mark.pixel(cur.x()-1,cur.y()))
            {
                QPoint pp(cur.x()-1,cur.y());
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()>0 && mark.pixel(cur.x(),cur.y()-1))
            {
                QPoint pp(cur.x(),cur.y()-1);
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()<mark.height()-1 && mark.pixel(cur.x(),cur.y()+1))
            {
                QPoint pp(cur.x(),cur.y()+1);
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
        }
        
        
        debug.save("./anchors.ppm");
    }
    
//    int ret=0;
    
#if USE_DESC_LOOP_FINDER
    //Descender extra work
    foreach (QPoint crossOverPoint, crossOverPoints)
        strengthenDescenderComponentAccum(img,crossOverPoint,g,indexer);
#endif
    int ret = g -> maxflow();
    
    //debug
//    printf("Maxflow of cut is %d\n",ret);
#if SHOW_VIZ_CUT
    //visilization
//    cv::Mat cloud(1,width*height*depth, CV_32FC3);
//    cv::Point3f* anglePoints = cloud.ptr<cv::Point3f>();
//    cv::Mat color_cloud(1,width*height*depth, CV_8UC3);
    cv::viz::Viz3d window("segmentation");
#endif
    
    //add all black pixels which
//    int slopeDifRange = SLOPE_DIF_TOLERANCE*depth;
    int THRESH=320;//700;
    for (int x=0; x<width; x++)
    {
//        printf("\n");
        for (int y=0; y<height; y++)
        {
            
            if (img.pixel(x,y))
            {
                int sourceScore=0;
                int sinkScore=0;
                for (int z=0; z<depth; z++)
                {
                    int index = indexer.getIndex(x,y,z);
                    if (invDistMap3D[index] > 75)
                    {
                        if (g->what_segment(index) == GraphType::SOURCE)
                            sourceScore += invDistMap3D[index];
                        else
                            sinkScore += invDistMap3D[index];
                        
                        //testing
//                        if ((x==37 && y==43) || (x==37 && y==49)/*(x==33 && y==49)*/)
//                        {
//                            if (g->what_segment(index) == GraphType::SOURCE)
//                                printf("source + %f = %d\n",invDistMap3D[index]*img.getBinsAndStrForPixel(x,y)[z],sourceScore);
//                            else
//                                printf("sink + %f = %d\n",invDistMap3D[index]*img.getBinsAndStrForPixel(x,y)[z],sinkScore);
//                        }
                        
#if SHOW_VIZ_CUT
                        //visulization
                        if (img.getBinsAndStrForPixel(x,y)[z] > .15)
                        {
//                            anglePoints[x+width*y+width*height*z].x=(float)x;
//                            anglePoints[x+width*y+width*height*z].y=(float)y;
//                            anglePoints[x+width*y+width*height*z].z=(float)z;
                            
                            
                            if (g->what_segment(index) == GraphType::SOURCE)
                            {
                                cv::Point3f start(x,y,z);
                                cv::Point3f end(x+1,y+1,z+1);
                                cv::viz::WCube voxel(start,end,false,cv::viz::Color::red());
                                QString xs;
                                xs.setNum(x);
                                QString ys;
                                ys.setNum(y);
                                QString zs;
                                zs.setNum(z);
                                QString name = xs+","+ys+","+zs;
                                window.showWidget(name.toStdString(),voxel);
//                                color_cloud.at<cv::Vec3b>(0,x+width*y+width*height*z)[0]=0;
//                                color_cloud.at<cv::Vec3b>(0,x+width*y+width*height*z)[1]=(int)(img.getBinsAndStrForPixel(x,y)[z]*254);
//                                color_cloud.at<cv::Vec3b>(0,x+width*y+width*height*z)[2]=0;
                            }
                            else
                            {
                                cv::Point3f start(x,y,z);
                                cv::Point3f end(x+1,y+1,z+1);
                                cv::viz::WCube voxel(start,end,false,cv::viz::Color::green());
                                QString xs;
                                xs.setNum(x);
                                QString ys;
                                ys.setNum(y);
                                QString zs;
                                zs.setNum(z);
                                QString name = xs+","+ys+","+zs;
                                window.showWidget(name.toStdString(),voxel);
//                                color_cloud.at<cv::Vec3b>(0,x+width*y+width*height*z)[0]=0;
//                                color_cloud.at<cv::Vec3b>(0,x+width*y+width*height*z)[1]=0;
//                                color_cloud.at<cv::Vec3b>(0,x+width*y+width*height*z)[2]=(int)(img.getBinsAndStrForPixel(x,y)[z]*254);
                            }
                        }
#endif
                    }
                    
                    //            printf("(%d,%d) ",sourceScore,sinkScore);
                    
                    if (sourceScore>=THRESH || (sinkScore<THRESH && sourceScore>sinkScore))
                        outSource.append(x+width*y);
                    if (sinkScore>=THRESH || (sourceScore<THRESH && sinkScore>sourceScore))
                        outSink.append(x+width*y);
                }
            }
            
            
        }
    }
    
#if SHOW_VIZ_CUT
    //visulization
    
    cv::viz::WCoordinateSystem axis(20.0);
    window.showWidget("axis",axis);
//    cv::viz::WCloud cloudImage(cloud,color_cloud);
//    window.showWidget("segmentation",cloudImage);
    window.spin();
#endif
    
//    ///test///
////    for (int s=0; s<depth; s++)
////    {
////        BImage test(img.width(), img.height());
////        BPartition tmp1((BPixelCollection*) &test);
////        BPartition tmp2((BPixelCollection*) &test);
////        for (int x=0; x<width; x++)
////        {
////            for (int y=0; y<height; y++)
////            {
//////                QMap<int,double> binsForDim = angleImage.getBinsAndStrForPixel(x,y);
//////                foreach (int bin, binsForDim.keys())
//////                {
//////                    if (bin<s+11 && bin>s-11)
//////                        test.setPixel(x,y,true);
//////                }
////                if (img.pixel(x,y))
////                    test.setPixel(x,y,true);
                
////                int index = indexer.getIndex(x,y,s);
////                if (g->what_segment(index) == GraphType::SOURCE)
////                    tmp1.addPixelFromSrc(x,y);
////                else
////                    tmp2.addPixelFromSrc(x,y);           
////            }
////        }
////        test.claimOwnership(&tmp2,1);
////        test.claimOwnership(&tmp1,1);
        
////        QString debugfile = "./output/layer_";
////        QString num;
////        num.setNum(s);
////        debugfile.append(num);
////        debugfile.append(".ppm");
////        test.saveOwners(debugfile);
        
////    }
//    //////////////////////////////
//    int newmax=0;
//    int topindex=0;
//    for (int i=0; i<width*height*depth; i++)
//    {
//        if (invDistMap3D[i]>newmax)
//        {
//            newmax=invDistMap3D[i];
//            topindex=i;
//        }
//    }
////    printf("invdistmap max was %d at %d:(%d,%d,%d)\t [%d,%d,%d]\n",newmax,topindex,topindex%width,(topindex/width)%height,topindex/(width*height),width,height,depth);
////    QVector<QRgb> default_color_table;
////    for (int i=0; i<255; i++)
////    {
////        default_color_table.append(qRgb(i*.8+255*.2,0,0));
////    }
////    for (int i=0; i<255; i++)
////    {
////        default_color_table.append(qRgb(0,0,i*.8+255*.2));
////    }
//    for (int z=0; z<depth; z++)
//    {
//        QImage debug(width,height,QImage::Format_RGB16);
        
////        debug.setColorTable(default_color_table);
//        for (int x=0; x<width; x++)
//        {
//            for (int y=0; y<debug.height(); y++)
//            {
//                int inten = (int)((invDistMap3D[indexer.getIndex(x,y,z)]/((double)newmax))*254);
//                QRgb color;
//                if (g->what_segment(indexer.getIndex(x,y,z)) == GraphType::SOURCE)
//                    color=qRgb(inten*.8+255*.2,0,0);
//                else
//                    color=qRgb(0,inten*.8+255*.2,0);  
//                debug.setPixel(x,y,color);
//            }
            
//        }
//        QString debugfile = "./output/layer_";
//        QString num;
//        num.setNum(z);
//        debugfile.append(num);
//        debugfile.append(".ppm");
//        debug.save(debugfile);
//    }
//    ///test///
    
    
    delete g;
    return ret;
}


///////////
int GraphCut::pixelsOfSeparationRecut2D(const BPixelCollection &img, const int* invDistMap, int width, int height, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, const QPoint &crossOverPoint, int anchor_weight, int split_method)
{

    GraphType *g = new GraphType(width*height, 4*(width-1)*(height-1)-(height+width)); 
    
    for (int i=0; i<width*height; i++)
    {
        g->add_node();
    }
    
//    QImage debug = img.makeImage().getImage();
//    QVector<QRgb> ct = debug.colorTable();
//    ct.append(qRgb(205,50,50));
//    ct.append(qRgb(50,205,50));
//    debug.setColorTable(ct);
    
    //anchoring
    QImage debug = img.makeImage().getImage();
    QVector<QRgb> ct = debug.colorTable();
    ct.append(qRgb(205,50,50));
    ct.append(qRgb(50,205,50));
    debug.setColorTable(ct);
    
    {
        double anchor_weight_bias = 100;
        int vert_divide = crossOverPoint.y();
        
        //all pixels are either source or sink
        for (int j=0; j<vert_divide; j++)
        {
            double anchor_weight_for_level = anchor_weight_bias * ((2.5*vert_divide-j)/(double)(2.5*vert_divide));
            //printf("%f, ",((vert_divide-j)/(double)vert_divide));
            for (int i=0; i<width; i++)
            {
                if (img.pixel(i,j))
                {
                    int index = i+width*j;
                    g -> add_tweights(index, (int)anchor_weight_for_level,0);
    //                debug.setPixel(i,j,150);
                }
            }
        }
        for (int j=height-1; j>=vert_divide; j--)
        {
            double anchor_weight_for_level = anchor_weight_bias * ((((height-1) -(double)vert_divide) + j-vert_divide)/(2.*((height-1) -(double)vert_divide)));
            //printf("%f- ",((j-vert_divide)/((height-1) -vert_divide)));
            for (int i=0; i<width; i++)
            {
                if (img.pixel(i,j))
                {
                    int index = i+width*j;
                    g -> add_tweights(index, 0, (int)anchor_weight_for_level);
    //                debug.setPixel(i,j,150);
                }
            }
        }

    /////////////begin solid anchors
        //find source pixels
        int count_source = NEW_ANCHOR;
        //fill
        BImage mark = img.makeImage();
        QVector<QPoint> workingStack;
        foreach (QPoint seed, sourceSeeds)
        {
            workingStack.push_back(seed);
            mark.setPixel(seed,false);
        }
        while (!workingStack.isEmpty() && count_source>0)
        {   
            QPoint cur = workingStack.front();
            workingStack.pop_front();
            int index = cur.x()+cur.y()*width;
            g -> add_tweights(index, anchor_weight*2, 0);
            debug.setPixel(cur,2);
            count_source--;
            
            
            
            if (cur.x()<mark.width()-1 && mark.pixel(cur.x()+1,cur.y()))
            {
                QPoint pp(cur.x()+1,cur.y());
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
                
            }
            if (cur.x()>0 && mark.pixel(cur.x()-1,cur.y()))
            {
                QPoint pp(cur.x()-1,cur.y());
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()>0 && mark.pixel(cur.x(),cur.y()-1))
            {
                QPoint pp(cur.x(),cur.y()-1);
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()<mark.height()-1 && mark.pixel(cur.x(),cur.y()+1))
            {
                QPoint pp(cur.x(),cur.y()+1);
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
        }
        
        int count_sink=NEW_ANCHOR;
        
        //find sink pixels

        //fill
        workingStack.clear();
        foreach (QPoint seed, sinkSeeds)
        {
            workingStack.push_back(seed);
            mark.setPixel(seed,false);
        }
        while (!workingStack.isEmpty() && count_sink>0)
        {   
            QPoint cur = workingStack.front();
            workingStack.pop_front();
            int index = cur.x()+cur.y()*width;
            g -> add_tweights(index, 0, anchor_weight*2);
            debug.setPixel(cur,3);
            count_sink--;
            
            
            
            if (cur.x()<mark.width()-1 && mark.pixel(cur.x()+1,cur.y()))
            {
                QPoint pp(cur.x()+1,cur.y());
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
                
            }
            if (cur.x()>0 && mark.pixel(cur.x()-1,cur.y()))
            {
                QPoint pp(cur.x()-1,cur.y());
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()>0 && mark.pixel(cur.x(),cur.y()-1))
            {
                QPoint pp(cur.x(),cur.y()-1);
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()<mark.height()-1 && mark.pixel(cur.x(),cur.y()+1))
            {
                QPoint pp(cur.x(),cur.y()+1);
                workingStack.push_back(pp);
                mark.setPixel(pp,false);
            }
        }
        
        
        debug.save("./anchors.ppm");
    }
    
    
    //printf("num source:%d, num sink:%d\n",count_source,count_sink);
    
    double BLACK_TO_BLACK_V_BIAS = .5;
    double BLACK_TO_BLACK_H_BIAS = .5;
    double BLACK_TO_BLACK_D_BIAS = (BLACK_TO_BLACK_V_BIAS+BLACK_TO_BLACK_H_BIAS)-sqrt(pow(BLACK_TO_BLACK_V_BIAS,2)+pow(BLACK_TO_BLACK_H_BIAS,2));
    double WHITE_TO_BLACK_BIAS = .5;
    double BLACK_TO_WHITE_BIAS = .5;
    double WHITE_TO_WHITE_V_BIAS = .5;
    double WHITE_TO_WHITE_H_BIAS = .5;
    double WHITE_TO_WHITE_D_BIAS = (WHITE_TO_WHITE_V_BIAS+WHITE_TO_WHITE_H_BIAS)-sqrt(pow(WHITE_TO_WHITE_V_BIAS,2)+pow(WHITE_TO_WHITE_H_BIAS,2));
    
//    if (split_method==SPLIT_VERT)
//    {
//        BLACK_TO_BLACK_V_BIAS = 0.75;
//        BLACK_TO_BLACK_H_BIAS = 0.75;
//        BLACK_TO_BLACK_D_BIAS = (BLACK_TO_BLACK_V_BIAS+BLACK_TO_BLACK_H_BIAS)-sqrt(pow(BLACK_TO_BLACK_V_BIAS,2)+pow(BLACK_TO_BLACK_H_BIAS,2));
//        WHITE_TO_BLACK_BIAS = .5;
//        BLACK_TO_WHITE_BIAS = .5;
//        WHITE_TO_WHITE_V_BIAS = .5;
//        WHITE_TO_WHITE_H_BIAS = .5;
//        WHITE_TO_WHITE_D_BIAS = (WHITE_TO_WHITE_V_BIAS+WHITE_TO_WHITE_H_BIAS)-sqrt(pow(WHITE_TO_WHITE_V_BIAS,2)+pow(WHITE_TO_WHITE_H_BIAS,2));
//    }

    double reducer = 1;
    
    //connect all pixels
    for (int i=0; i<width; i++)
    {
        for (int j=0; j<height; j++)
        {   
//            if (split_method==SPLIT_VERT)
//            {
//                if (j<vert_divide)
//                {
//                    reducer = ((2*vert_divide-j)/(double)(2*vert_divide));
//                }
//                else
//                {
//                    reducer = ((((height-1) -(double)vert_divide) + j-vert_divide)/(2.*((height-1) -(double)vert_divide)));
//                }
//            }
//            else if (split_method==CHOP_TOP)
//            {
//                   reducer = ((3.0*height-j)/(double)(3*height));
//            }
            
            
            if (i+1<width)
            {
                setEdge(i,j,i+1,j,g,img,invDistMap,BLACK_TO_BLACK_H_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_H_BIAS,reducer,width);
            }
            
            if (j+1<height)
            {
                setEdge(i,j,i,j+1,g,img,invDistMap,BLACK_TO_BLACK_V_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_V_BIAS,reducer,width);
            }
            
            if (j>0 && i<width-1)
            {
                setEdge(i,j,i+1,j-1,g,img,invDistMap,BLACK_TO_BLACK_D_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_D_BIAS,reducer,width);
            }
            
            if (j<height-1 && i<width-1)
            {
                setEdge(i,j,i+1,j+1,g,img,invDistMap,BLACK_TO_BLACK_D_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_D_BIAS,reducer,width);
            }
        }
    }
    
    BlobSkeleton skeleton(&img);
    findDescenderAccumulatively(skeleton,crossOverPoint);
//    strengthenDescenderComponent2D(img,crossOverPoint,g);
    int ret = g -> maxflow();
    
//    QImage debug2 = debug.convertToFormat(QImage::Format_RGB16);
//    QRgb lw = qRgb(255, 100, 100);
//    QRgb lb = qRgb(155, 0, 0);
//    QRgb rw = qRgb(100,255, 100);
//    QRgb rb = qRgb(0, 155, 0);
//    QRgb a = qRgb(255, 255, 255);
    
    //add all black pixels which
    for (int index=0; index<width*height; index++)
    {
        if (img.pixelIsMine(index%width,index/width))
        {
            /*if (qGray(debug.pixel(index%width,index/width))!=BLACK && qGray(debug.pixel(index%width,index/width))!=WHITE)
            {
                debug2.setPixel(index%width,index/width,a);
            }
            else*/ if (g->what_segment(index) == GraphType::SOURCE)
            {
                outSource.append(index);
    //            debug2.setPixel(index%width,index/width,lb);
            }
            else
            {
                outSink.append(index);
            }
    //        else if (g->what_segment(index) == GraphType::SOURCE)
    //            debug2.setPixel(index%width,index/width,lw);
    //        else if (qGray(img.pixel(index%width,index/width))==BLACK)
    //            debug2.setPixel(index%width,index/width,rb);
    //        else
    //            debug2.setPixel(index%width,index/width,rw);
        }
    }
    
//    QString debugfile = "./cut_";
//    QString num;
//    num.setNum(width);
//    debugfile.append(num);
//    debugfile.append(".ppm");
//    debug2.save(debugfile);
    
    delete g;
    return ret;
}
///////////





double getRelAngle(const BlobSkeleton &skeleton, int indexA, int indexB, int indexC)
{
    double angle1 = (atan2((skeleton[indexA].y-skeleton[indexB].y),(skeleton[indexA].x-skeleton[indexB].x)));
    double angle2 = (atan2((skeleton[indexB].y-skeleton[indexC].y),(skeleton[indexB].x-skeleton[indexC].x)));
    double ret = PI-angle2+angle1;
    if (ret<0) ret+=2*PI;
//    printf("getRelAngle test,\tone (%d,%d)(%d,%d):%f,\ttwo (%d,%d)(%d,%d):%f,\tres=%f\n",skeleton[indexA].x,skeleton[indexA].y,skeleton[indexB].x,skeleton[indexB].y,angle1,skeleton[indexB].x,skeleton[indexB].y,skeleton[indexC].x,skeleton[indexC].y,angle2,ret/PI);
    return ret;
}

inline double getDist(const BlobSkeleton &skeleton, int indexA, int indexB)
{
    return pow( (pow(skeleton[indexA].x-skeleton[indexB].x,2) + pow(skeleton[indexA].y-skeleton[indexB].y,2)) ,.5);
}

double GraphCut::computeScore(int sampleSize, double* x, double* y, double meanSlope, double stdDevSlope, double meanCurve, double stdDevCurve, bool print)
{
    double tss = gsl_stats_tss(y,1,sampleSize);
    double cov[9];
    double linOut[2];
//    double chisqSlope= polynomialfit(sampleSize,2,x,y,linOut,cov);
    double slope=linOut[1];
//    double rsqSlope=1-chisqSlope/tss;
    
    double quadOut[3];
    double chisqCurve = polynomialfit(sampleSize,3,x,y,quadOut,cov);
    double curvature=quadOut[2];
    double rsqCurve=1-chisqCurve/tss;
    
    double yOfVertex = quadOut[1]/(2*quadOut[2]);
    
//    double score = (copysign(1.0, curvature) == copysign(1.0, meanCurve) && 
//                    copysign(1.0, slope) == copysign(1.0, meanSlope)) ? 
//                        (.75*pow(1.2,chisqCurve/20))*fabs(curvature-meanCurve) + (.1*pow(1.2,chisqSlope/30))*fabs(slope-meanSlope) : DOUBLE_POS_INFINITY;
//    double score = (copysign(1.0, curvature) == copysign(1.0, meanCurve) && 
//                    copysign(1.0, slope) == copysign(1.0, meanSlope)) ? 
//                        .75*(1-rsqCurve)*fabs(curvature-meanCurve) + .1*(1-rsqSlope)*fabs(slope-meanSlope) : DOUBLE_POS_INFINITY;
//    double score = (copysign(1.0, curvature) == copysign(1.0, meanCurve) && 
//                    copysign(1.0, slope) == copysign(1.0, meanSlope)) ? 
//                        .75*(1/rsqCurve)*std::max(fabs(curvature-meanCurve),2*stdDevCurve) + .1*(1/rsqSlope)*fabs(slope-meanSlope) : DOUBLE_POS_INFINITY;
    double score = (copysign(1.0, curvature) == copysign(1.0, meanCurve) ||
                    fabs(curvature)<0.001) ? 
                10*(1/std::max(rsqCurve,0.1))*std::max(fabs(curvature-meanCurve),2*stdDevCurve)/(2*stdDevCurve) + .1*std::max(fabs(slope-meanSlope),2*stdDevSlope)/(2*stdDevSlope) : DOUBLE_POS_INFINITY;
    
    
    if (print /*&& score!=DOUBLE_POS_INFINITY*/)
    {
        if (meanSlope==UPPER_MEAN_SLOPE)
        {
            printf("[U] ");
        }
        else
            printf("[L] ");
        printf("Curve=%f\tdif=%f\tR^2=%f\tscore=%f\tV_y=%d\n",curvature,curvature-meanCurve,rsqCurve,score==DOUBLE_POS_INFINITY?-1:score,(int)yOfVertex);
        int maxY=0;
        int minY=INT_POS_INFINITY;
        for (int i=0; i<sampleSize; i++)
        {
            if (x[i]>maxY)
                maxY=x[i];
            if (x[i]<minY)
                minY=x[i];
        }
        if (yOfVertex<minY && yOfVertex<maxY)
            printf("Vertex is above.\n");
        else if (yOfVertex>minY && yOfVertex>maxY)
            printf("Vertex is below.\n");
        else
            printf("Vertex is between.\n");
        
//        printf("chi^2=%f\tTSS=%f\n",chisqSlope,tss);
    }
    
    return score;
}



class ComparePointer
{
public:
    bool operator() (const DescenderPath* a, const DescenderPath* b) {return a->score() < b->score();}
};

//int testIterPass;
//int testIterPassTotal;

DescenderPath* GraphCut::findDescenderAccumulatively(const BlobSkeleton &skeleton, const QPoint &startPoint)
{
    std::priority_queue<DescenderPath*,QVector<DescenderPath*>,ComparePointer>* queue = new std::priority_queue<DescenderPath*,QVector<DescenderPath*>,ComparePointer>();
    QMap<double,DescenderPath*> finishedPaths;
    int startRegionId = skeleton.closestRegionIdForPoint(startPoint);
//    QVector<unsigned int> startPath;
    DescenderPath* startPath = new DescenderPath(&skeleton);
    startPath->append(startRegionId);
    queue->push(startPath);
    
    int testCount=0;
    int testIter=0;
    QMap<DescenderPath*,int> finishedIters;
    
//    printf("New Descender\n");
    
    while (!queue->empty())
    {
        
        
        DescenderPath* path = queue->top();
        queue->pop();
        
        ///test///
//        printf("Current path[%f]: ",path->score());
//        for (int i=0; i<path->size(); i++)
//            printf("(%d,%d), ",path->pointAt(i).x(),path->pointAt(i).y());
//        printf("\t%d neighbors\n",skeleton[path->last()].connectedPoints().size());
//        path->printScore();
        ///test///
        
        
        if (skeleton[path->last()].connectedPoints().size()>0 && (path->size()==1 || path->count(path->last())<2))
        {
            bool betterFound=false;
            foreach (unsigned int nextIndex, skeleton[path->last()].connectedPoints())
            {
                if (skeleton[nextIndex].y<skeleton[startRegionId].y || (path->size()>1 && nextIndex==path->at(path->size()-2)))
                    continue;
                
                DescenderPath* newPath = new DescenderPath(*path);
                testCount++;
                newPath->append(nextIndex);
                if (newPath->score() < NEW_SCORE_THRESH)
                {
                    queue->push(newPath);
                    
                    betterFound |= (newPath->score() < path->score());
                }
                else
                {
                    ///test///
//                    printf("Tossed path[%f]: ",newPath->score());
//                    for (int i=0; i<newPath->size(); i++)
//                        printf("(%d,%d), ",newPath->pointAt(i).x(),newPath->pointAt(i).y());
//                    printf("\n");
//                    newPath->printScore();
                    ///test///
                }
            }
            
            if (!betterFound && path->hasTop())
            {
                finishedPaths.insert(path->score(),path);
                finishedIters.insert(path,testIter);
            }
            else
                delete path;
        }
        else if (path->hasTop())
        {
            finishedPaths.insert(path->score(),path);
            finishedIters.insert(path,testIter);
        }
        else
        {
            delete path;
            testCount--;
        }
        testIter++;
    }
    
    ///test
//    printf("Top 7 paths:\n");
//    for (int i=0; i<finishedPaths.size() && i<7; i++)
//    {
//        finishedPaths.values()[i]->printScore();
//        printf("Path %d [%f]: ",i,finishedPaths.values()[i]->score());
//        for (int j=0; j<finishedPaths.values()[i]->size(); j++)
//        {
//            printf("(%d,%d), ",skeleton[finishedPaths.values()[i]->at(j)].x,skeleton[finishedPaths.values()[i]->at(j)].y);
//        }
//        printf("\n");
//    }
    ///test///
    //delete
    delete queue;
    for (int i=1; i<finishedPaths.size(); i++)
    {
        delete finishedPaths.values()[i];
    }
    
    if (finishedPaths.size()>0)
    {
//        printf("Desc found : %f\t",finishedPaths.values()[0]->score());
        
//        printf("Found path in %d iterations \tof %d\n",finishedIters[finishedPaths.values()[0]],testIter);
//        finishedPaths.values()[0]->printScore();
//        testIterPass=finishedIters[finishedPaths.values()[0]];
//        testIterPassTotal=testIter;
        return finishedPaths.values()[0];
    }
    else
        return NULL;
}

void GraphCut::strengthenDescenderComponentAccum(const AngleImage &img, const QPoint &crossOverPoint, GraphType *g, const Indexer3D &indexer)
{
    DescenderPath* descPath = findDescenderAccumulatively(img.getSkeleton(), crossOverPoint);
    
    if (descPath!=NULL && descPath->score() < DESCENDER_LIKELIHOOD_THRESH)
    {
        
        
        QImage test(img.makeImage().getImage());
        QVector<QRgb> colors=test.colorTable();
        colors.append(qRgb(255,0,0));
        test.setColorTable(colors);
        
        int firstX = descPath->pointAt(0).x();
        int firstY = descPath->pointAt(0).y();
//        int index = img.getSkeleton()[descPath->at(0)].connectedPoints.indexOf(descPath->at(1));
//        double angle = img.getSkeleton()[descPath->at(0)].angleBetween[index];
        double angle = img.getSkeleton()[descPath->at(0)].angleBetween(descPath->at(1));
        int firstZ = img.getBinForAngle(angle);
        
//        int prevUpperX=firstUpperX;
//        int prevUpperY=firstUpperY;
//        int prevUpperZ=firstUpperZ;
        int curX=firstX;
        int curY=firstY;
        int z=firstZ;
        
        int nextX = descPath->pointAt(1).x();
        int nextY = descPath->pointAt(1).y();
        //connect to next
        strengthenConnection3D(curX,curY,z,nextX,nextY,z,g,img,img.getNumOfBins(),&test);
//        int indexA = indexer.getIndex(curX,curY,z);
//        int indexB = indexer.getIndex(nextX,nextY,z);
//        g->add_edge(indexA,indexB,DESC_BIAS_LEN_2D,DESC_BIAS_LEN_2D);
        
        test.setPixel(firstX,firstY,colors.size()-1);
        test.setPixel(nextX,nextY,colors.size()-1);
        for (unsigned int j=1; j<descPath->size()-1; j++)////////
        {
            
            
//            int prevZ=z;
            
//            index = img.getSkeleton()[descPath->at(j)].connectedPoints.indexOf(descPath->at(j+1));
//            angle = img.getSkeleton()[descPath->at(j)].angleBetween[index];
            angle = img.getSkeleton()[descPath->at(j)].angleBetween(descPath->at(j+1));
            z= img.getBinForAngle(angle);
            
            curX = nextX;
            curY = nextY;
            nextX = descPath->pointAt(j+1).x();
            nextY = descPath->pointAt(j+1).y();
            test.setPixel(nextX,nextY,colors.size()-1);
            
            //connect Z
//            if (prevZ!=z)
//            {
//                strengthenConnection3D(curX,curY,prevZ,curX,curY,z,g,img,img.getNumOfBins(),&test);
//            }
            
            //connect to next
            strengthenConnection3D(curX,curY,z,nextX,nextY,z,g,img,img.getNumOfBins(),&test);
        }
        
//        if (nextX==firstX && nextY == firstY && z!=firstZ)//close loops
//        {
//            strengthenConnection3D(firstX,firstY,firstZ,nextX,nextY,z,g,img,img.getNumOfBins(),&test);
//        }
        
        test.save("./test_descender_id.ppm");
//        if (testIterPass>50)
//        {
        if (WRITE_SCORES)
        {
            char read;
            char dump;
            while (true)
            {
                printf("Correct(m/n)?:");
                scanf("%c%c",&read,&dump);
                if (read=='m')
                {
                    correctScores.append(descPath->score());
                    break;
                }
                else if (read=='n')
                {
                    incorrectScores.append(descPath->score());
                    break;
                }
            }
        }
//        }
    }
}

void GraphCut::strengthenDescenderComponent(const AngleImage &img, const QPoint &crossOverPoint, GraphType *g, const Indexer3D &indexer)
{
    assert(img.pixel(crossOverPoint));
    
    int startRegionId = img.getSkeleton().closestRegionIdForPoint(crossOverPoint);
    
    
//    printf("Starting point:(%d,%d)\n",img.getSkeleton()[startRegionId].x,img.getSkeleton()[startRegionId].y);
    QVector<QVector<unsigned int> > bestLowerPaths;
    QVector<double> bestLowerScores;
    QVector<QVector<unsigned int> > bestUpperPaths;
    QVector<double> bestUpperScores;
    
    PathStackMap upperPaths;
    
//    foreach (unsigned int curIndex, img.getSkeleton()[startRegionId].connectedPoints)
//    {
//        if (img.getSkeleton()[curIndex].y>=crossOverPoint.y())
//        {
//            QVector<unsigned int> newPath;
//            newPath.append(startRegionId);
//            newPath.append(curIndex);
//            lowerDescenderTraverser(img.getSkeleton(),&bestLowerPath,&bestLowerScore,&bestUpperPath,&bestUpperScore,&newPath,0,&upperPaths);
//        }
//    }
    QVector<unsigned int> newPath;
    newPath.append(startRegionId);
    lowerDescenderTraverser(img.getSkeleton(),&bestLowerPaths,&bestLowerScores,&bestUpperPaths,&bestUpperScores,&newPath,0,&upperPaths);
    
    
    QImage test(img.makeImage().getImage());
    QVector<QRgb> colors=test.colorTable();
    colors.append(qRgb(255,0,0));
    test.setColorTable(colors);
//    printf("Best paths:\n");
    QMap<double,unsigned int> byCombinedScore;
    for (int i=0; i<bestLowerPaths.size(); i++)
    {
        byCombinedScore[bestUpperScores[i]+bestLowerScores[i]] = i;
        
//        printf("Combine (%f):\n[U] path (%f) is: ",bestUpperScores[i]+bestLowerScores[i],bestUpperScores[i]);
//        foreach (unsigned int j, bestUpperPaths[i])
//            printf("(%d,%d), ",img.getSkeleton()[j].x,img.getSkeleton()[j].y);
//        printf("\n");
//        printf("[L] path (%f) is: ",bestLowerScores[i]);
//        foreach (unsigned int j, bestLowerPaths[i])
//            printf("(%d,%d), ",img.getSkeleton()[j].x,img.getSkeleton()[j].y);
//        printf("\n");
    }
    
    int count=0;
    foreach (unsigned int i, byCombinedScore.values())
    {
        if (count++>=5)
            break;
        
//                printf("Combine (%f):\n[U] path (%f) is: ",bestUpperScores[i]+bestLowerScores[i],bestUpperScores[i]);
//                foreach (unsigned int j, bestUpperPaths[i])
//                    printf("(%d,%d), ",img.getSkeleton()[j].x,img.getSkeleton()[j].y);
//                printf("\n");
//                printf("[L] path (%f) is: ",bestLowerScores[i]);
//                foreach (unsigned int j, bestLowerPaths[i])
//                    printf("(%d,%d), ",img.getSkeleton()[j].x,img.getSkeleton()[j].y);
//                printf("\n");
        
        int firstUpperX = img.getSkeleton()[bestUpperPaths[i][0]].x;
        int firstUpperY = img.getSkeleton()[bestUpperPaths[i][0]].y;
//        int index = img.getSkeleton()[bestUpperPaths[i][0]].connectedPoints.indexOf(bestUpperPaths[i][1]);
//        double angle = img.getSkeleton()[bestUpperPaths[i][0]].angleBetween[index];
        double angle = img.getSkeleton()[bestUpperPaths[i][0]].angleBetween(bestUpperPaths[i][1]);
        int firstUpperZ = img.getBinForAngle(angle);
        
//        int prevUpperX=firstUpperX;
//        int prevUpperY=firstUpperY;
//        int prevUpperZ=firstUpperZ;
        int curX=firstUpperX;
        int curY=firstUpperY;
        int z=firstUpperZ;
        
        int nextX = img.getSkeleton()[bestUpperPaths[i][1]].x;
        int nextY = img.getSkeleton()[bestUpperPaths[i][1]].y;
        //connect to next
        strengthenConnection3D(curX,curY,z,nextX,nextY,z,g,img,img.getNumOfBins(),&test);
//        int indexA = indexer.getIndex(curX,curY,z);
//        int indexB = indexer.getIndex(nextX,nextY,z);
//        g->add_edge(indexA,indexB,DESC_BIAS_LEN_2D,DESC_BIAS_LEN_2D);
        
        test.setPixel(img.getSkeleton()[bestUpperPaths[i][0]].x,img.getSkeleton()[bestUpperPaths[i][0]].y,colors.size()-1);
        test.setPixel(img.getSkeleton()[bestUpperPaths[i][1]].x,img.getSkeleton()[bestUpperPaths[i][1]].y,colors.size()-1);
        for (unsigned int j=1; j<bestUpperPaths[i].size()-1; j++)////////
        {
            test.setPixel(img.getSkeleton()[bestUpperPaths[i][j+1]].x,img.getSkeleton()[bestUpperPaths[i][j+1]].y,colors.size()-1);
            
            int prevZ=z;
//            index = img.getSkeleton()[bestUpperPaths[i][j]].connectedPoints.indexOf(bestUpperPaths[i][j+1]);
//            angle = img.getSkeleton()[bestUpperPaths[i][j]].angleBetween[index];
            angle = img.getSkeleton()[bestUpperPaths[i][j]].angleBetween(bestUpperPaths[i][j+1]);
            z= img.getBinForAngle(angle);
            
            curX = nextX;
            curY = nextY;
            nextX = img.getSkeleton()[bestUpperPaths[i][j+1]].x;
            nextY = img.getSkeleton()[bestUpperPaths[i][j+1]].y;
            
            //connect Z
            if (prevZ!=z)
            {
                strengthenConnection3D(curX,curY,prevZ,curX,curY,z,g,img,img.getNumOfBins(),&test);
//                indexA = indexer.getIndex(curX,curY,prevZ);
//                indexB = indexer.getIndex(curX,curX,z);
//                g->add_edge(indexA,indexB,DESC_BIAS_Z,DESC_BIAS_Z);
            }
            
            //connect to next
            strengthenConnection3D(curX,curY,z,nextX,nextY,z,g,img,img.getNumOfBins(),&test);
        }
        
        int lastUpperZ=z;
        
        curX = img.getSkeleton()[bestLowerPaths[i][0]].x;                              
        curY = img.getSkeleton()[bestLowerPaths[i][0]].y;    
//        index = img.getSkeleton()[bestLowerPaths[i][0]].connectedPoints.indexOf(bestLowerPaths[i][1]);
//        angle = img.getSkeleton()[bestLowerPaths[i][0]].angleBetween[index];
        angle = img.getSkeleton()[bestUpperPaths[i][0]].angleBetween(bestUpperPaths[i][1]);
        z = img.getBinForAngle(angle);
        
        nextX = img.getSkeleton()[bestLowerPaths[i][1]].x;
        nextY = img.getSkeleton()[bestLowerPaths[i][1]].y;
        //connect to next
        strengthenConnection3D(curX,curY,z,nextX,nextY,z,g,img,img.getNumOfBins(),&test);
        
        if (curX==firstUpperX && curY==firstUpperY && firstUpperZ!=z)
        {
            strengthenConnection3D(curX,curY,firstUpperZ,curX,curY,z,g,img,img.getNumOfBins(),&test);
//            indexA = indexer.getIndex(curX,curY,firstUpperZ);
//            indexB = indexer.getIndex(curX,curX,z);
//            g->add_edge(indexA,indexB,DESC_BIAS_Z,DESC_BIAS_Z);
        }
        
        test.setPixel(img.getSkeleton()[bestLowerPaths[i][0]].x,img.getSkeleton()[bestLowerPaths[i][0]].y,colors.size()-1);
        test.setPixel(img.getSkeleton()[bestLowerPaths[i][1]].x,img.getSkeleton()[bestLowerPaths[i][1]].y,colors.size()-1);
        for (unsigned int j=1; j<bestLowerPaths[i].size()-1; j++)////////
        {
            test.setPixel(img.getSkeleton()[bestLowerPaths[i][j+1]].x,img.getSkeleton()[bestLowerPaths[i][j+1]].y,colors.size()-1);
            
            int prevZ=z;
//            index = img.getSkeleton()[bestLowerPaths[i][j]].connectedPoints.indexOf(bestLowerPaths[i][j+1]);
//            angle = img.getSkeleton()[bestLowerPaths[i][j]].angleBetween[index];
            angle = img.getSkeleton()[bestUpperPaths[i][j]].angleBetween(bestUpperPaths[i][j+1]);
            z= img.getBinForAngle(angle);
            
            
            curX = nextX;
            curY = nextY;
            nextX = img.getSkeleton()[bestLowerPaths[i][j+1]].x;
            nextY = img.getSkeleton()[bestLowerPaths[i][j+1]].y;
            
            //connect Z
            if (prevZ!=z)
            {
                strengthenConnection3D(curX,curY,prevZ,curX,curY,z,g,img,img.getNumOfBins(),&test);
            }
            
            //connect to next
            strengthenConnection3D(curX,curY,z,nextX,nextY,z,g,img,img.getNumOfBins(),&test);
            
            //check upper start
            if (curX==firstUpperX && curY==firstUpperY)
            {
                if (prevZ!=firstUpperZ)
                {
                    strengthenConnection3D(curX,curY,prevZ,curX,curY,firstUpperZ,g,img,img.getNumOfBins(),&test);
                }
                
                if (firstUpperZ!=z)
                {
                    strengthenConnection3D(curX,curY,firstUpperZ,curX,curY,z,g,img,img.getNumOfBins(),&test);
                }
            }
        }
        
        if (lastUpperZ!=z)
        {
            strengthenConnection3D(nextX,nextY,lastUpperZ,nextX,nextY,z,g,img,img.getNumOfBins(),&test);
        }
        
    }
    
    test.save("./test_descender_id.ppm");
    
//    char a;
//    printf("Cont? ");
//    scanf("%c",&a);
//    printf("ok\n");
}

void GraphCut::strengthenConnection3D(int curX, int curY, int curZ, int nextX, int nextY, int nextZ, GraphType *g, const BPixelCollection &img, unsigned int depth, QImage *test)
{
//    printf("Connecting (%d,%d) to (%d,%d)\n",curX,curY,nextX,nextY);
    
    QVector<QVector3D> line;
    QVector3D start(curX,curY,curZ);
    line.append(start);
    if (curX==nextX && curY==nextY)
    {
        int inc = copysign(1.0, nextZ-curZ);
        for (int z=curZ+inc; inc*z<inc*nextZ; z+=inc)
        {
            QVector3D toAdd(curX,curY,z);
            line.append(toAdd);
        }
    }
    else if (curX==nextX || fabs((curY-nextY)/((double)curX-nextX)) > 1)
    {
        double slopeX = ((double)curX-nextX)/(curY-nextY);
        double intersectX = curX-curY*slopeX;
        double slopeZ = ((double)curZ-nextZ)/(curY-nextY);
        double intersectZ = curZ-curY*slopeZ;
        int inc = copysign(1.0, nextY-curY);
        for (int y=curY+inc; inc*y<inc*nextY; y+=inc)
        {
            QVector3D toAdd(y*slopeX+intersectX,y,y*slopeZ+intersectZ);
//            if (img.pixel(toAdd.toPoint()))
                line.append(toAdd);
//            else
//            {
//                return;
//            }
            test->setPixel(toAdd.toPoint(),test->colorTable().size()-1);
        }
    }
    else
    {
        double slopeY = (curY-nextY)/((double)curX-nextX);
        double intersectY = curY-curX*slopeY;
        double slopeZ = (curZ-nextZ)/((double)curX-nextX);
        double intersectZ = curZ-curX*slopeZ;
        int inc = copysign(1.0, nextX-curX);
        for (int x=curX+inc; inc*x<inc*nextX; x+=inc)
        {
            QVector3D toAdd(x,slopeY*x+intersectY,slopeZ*x+intersectZ);
//            if (img.pixel(toAdd.toPoint()))
                line.append(toAdd);
//            else
//            {
//                return;
//            }
            test->setPixel(toAdd.toPoint(),test->colorTable().size()-1);
        }
    }
    QVector3D end(nextX,nextY,nextZ);
    line.append(end);
    
    g->add_tweights(line[0].x() + line[0].y()*img.width(),DESC_BIAS_T_3D,0);
    for (int i=1; i<line.size(); i++)
    {
        for (int deltaZ=-2; deltaZ<=2; deltaZ++)
        {
            int indexA=line[i-1].x() + line[i-1].y()*img.width() + mod(line[i-1].z()+deltaZ,depth)*img.width()*img.height();
            int indexB=line[i].x() + line[i].y()*img.width() + mod(line[i].z()+deltaZ,depth)*img.width()*img.height();
            
            g->add_edge(indexA,indexB,DESC_BIAS_LEN_3D,DESC_BIAS_LEN_3D);
            g->add_tweights(indexB,DESC_BIAS_T_3D,0);
        }
    }
}


void GraphCut::strengthenDescenderComponent2D(const BPixelCollection &img, const QPoint &crossOverPoint, GraphType *g)
{
    assert(img.pixel(crossOverPoint));
    BlobSkeleton skeleton(&img);
    int startRegionId = skeleton.regionIdForPoint(crossOverPoint);
    if (startRegionId==-2)
    {
        BImage mark = img.makeImage();
        QVector<QPoint> stack;
        stack.push_back(crossOverPoint);
        mark.setPixel(crossOverPoint,false);
        while (startRegionId==-2 && !stack.empty())
        {
            QPoint p = stack.front();
            stack.pop_front();
            int tableIndex=8;
            for (int cc=0; cc<9 && startRegionId==-2; cc++)
            {
                tableIndex=(tableIndex+2)%9;
                if (tableIndex==4)
                    continue;
                
                int xDelta=(tableIndex%3)-1;
                int yDelta=(tableIndex/3)-1;
                int x = p.x()+xDelta;
                int y = p.y()+yDelta;
                if (mark.pixel(x,y))
                {
                    mark.setPixel(x,y,false);
                    startRegionId = skeleton.regionIdForPoint(x,y);
                    QPoint next(x,y);
                    stack.push_back(next);
                }
            }
        }
    }
//    printf("Starting point:(%d,%d)\n",skeleton[startRegionId].x,skeleton[startRegionId].y);
    QVector<QVector<unsigned int> > bestLowerPaths;
    QVector<double> bestLowerScores;
    QVector<QVector<unsigned int> > bestUpperPaths;
    QVector<double> bestUpperScores;
    
    PathStackMap upperPaths;
    
//    foreach (unsigned int curIndex, skeleton[startRegionId].connectedPoints)
//    {
//        if (skeleton[curIndex].y>=crossOverPoint.y())
//        {
//            QVector<unsigned int> newPath;
//            newPath.append(startRegionId);
//            newPath.append(curIndex);
//            lowerDescenderTraverser(skeleton,&bestLowerPath,&bestLowerScore,&bestUpperPath,&bestUpperScore,&newPath,0,&upperPaths);
//        }
//    }
    QVector<unsigned int> newPath;
    newPath.append(startRegionId);
    lowerDescenderTraverser(skeleton,&bestLowerPaths,&bestLowerScores,&bestUpperPaths,&bestUpperScores,&newPath,0,&upperPaths);
    
    
    QImage test(img.makeImage().getImage());
    QVector<QRgb> colors=test.colorTable();
    colors.append(qRgb(255,0,0));
    test.setColorTable(colors);
//    printf("Best paths:\n");
    QMap<double,unsigned int> byCombinedScore;
    for (int i=0; i<bestLowerPaths.size(); i++)
    {
        byCombinedScore[bestUpperScores[i]+bestLowerScores[i]] = i;
        
//        printf("Combine (%f):\n[U] path (%f) is: ",bestUpperScores[i]+bestLowerScores[i],bestUpperScores[i]);
//        foreach (unsigned int j, bestUpperPaths[i])
//            printf("(%d,%d), ",skeleton[j].x,skeleton[j].y);
//        printf("\n");
//        printf("[L] path (%f) is: ",bestLowerScores[i]);
//        foreach (unsigned int j, bestLowerPaths[i])
//            printf("(%d,%d), ",skeleton[j].x,skeleton[j].y);
//        printf("\n");
    }
    
    int count=0;
    foreach (unsigned int i, byCombinedScore.values())
    {
        if (count++>=5)
            break;
        
//                printf("Combine (%f):\n[U] path (%f) is: ",bestUpperScores[i]+bestLowerScores[i],bestUpperScores[i]);
//                foreach (unsigned int j, bestUpperPaths[i])
//                    printf("(%d,%d), ",skeleton[j].x,skeleton[j].y);
//                printf("\n");
//                printf("[L] path (%f) is: ",bestLowerScores[i]);
//                foreach (unsigned int j, bestLowerPaths[i])
//                    printf("(%d,%d), ",skeleton[j].x,skeleton[j].y);
//                printf("\n");
        
        int firstUpperX = skeleton[bestUpperPaths[i][0]].x;
        int firstUpperY = skeleton[bestUpperPaths[i][0]].y;
//        int index = skeleton[bestUpperPaths[i][0]].connectedPoints.indexOf(bestUpperPaths[i][1]);
//        double angle = skeleton[bestUpperPaths[i][0]].angleBetween[index];
//        int firstUpperZ = img.getBinForAngle(angle);
        
        int curX=firstUpperX;
        int curY=firstUpperY;
//        int z=firstUpperZ;
        
        int nextX = skeleton[bestUpperPaths[i][1]].x;
        int nextY = skeleton[bestUpperPaths[i][1]].y;
        //connect to next
        strengthenConnection2D(curX,curY,nextX,nextY,g,img,&test);
//        int indexA = curX+curY*width;
//        int indexB = nextX+nextY*width;
//        g->add_edge(indexA,indexB,DESC_BIAS_LEN_2D,DESC_BIAS_LEN_2D);
        
        test.setPixel(skeleton[bestUpperPaths[i][0]].x,skeleton[bestUpperPaths[i][0]].y,colors.size()-1);
        test.setPixel(skeleton[bestUpperPaths[i][1]].x,skeleton[bestUpperPaths[i][1]].y,colors.size()-1);
        for (unsigned int j=1; j<bestUpperPaths[i].size()-1; j++)////////
        {
            test.setPixel(skeleton[bestUpperPaths[i][j+1]].x,skeleton[bestUpperPaths[i][j+1]].y,colors.size()-1);
            
//            index = skeleton[bestUpperPaths[i][j]].connectedPoints.indexOf(bestUpperPaths[i][j+1]);
//            angle = skeleton[bestUpperPaths[i][j]].angleBetween[index];
//            z= img.getBinForAngle(angle);
            
            curX = nextX;
            curY = nextY;
            nextX = skeleton[bestUpperPaths[i][j+1]].x;
            nextY = skeleton[bestUpperPaths[i][j+1]].y;
            
            
            //connect to next
            strengthenConnection2D(curX,curY,nextX,nextY,g,img,&test);
    //        int indexA = curX+curY*width;
    //        int indexB = nextX+nextY*width;
//            g->add_edge(indexA,indexB,DESC_BIAS_LEN_2D,DESC_BIAS_LEN_2D);
        }
        
        
        curX = skeleton[bestLowerPaths[i][0]].x;                              
        curY = skeleton[bestLowerPaths[i][0]].y;    
//        index = skeleton[bestLowerPaths[i][0]].connectedPoints.indexOf(bestLowerPaths[i][1]);
//        angle = skeleton[bestLowerPaths[i][0]].angleBetween[index];
//        z = img.getBinForAngle(angle);
//        g->add_tweights(curX+curY*width,DESC_BIAS_LEN_2D,0);
        
        nextX = skeleton[bestLowerPaths[i][1]].x;
        nextY = skeleton[bestLowerPaths[i][1]].y;
        //connect to next
        strengthenConnection2D(curX,curY,nextX,nextY,g,img,&test);
//        int indexA = curX+curY*width;
//        int indexB = nextX+nextY*width;
//        g->add_edge(indexA,indexB,DESC_BIAS_LEN_2D,DESC_BIAS_LEN_2D);
        
        test.setPixel(skeleton[bestLowerPaths[i][0]].x,skeleton[bestLowerPaths[i][0]].y,colors.size()-1);
        test.setPixel(skeleton[bestLowerPaths[i][1]].x,skeleton[bestLowerPaths[i][1]].y,colors.size()-1);
        for (unsigned int j=1; j<bestLowerPaths[i].size()-1; j++)////////
        {
            test.setPixel(skeleton[bestLowerPaths[i][j+1]].x,skeleton[bestLowerPaths[i][j+1]].y,colors.size()-1);
            
//            index = skeleton[bestLowerPaths[i][j]].connectedPoints.indexOf(bestLowerPaths[i][j+1]);
//            angle = skeleton[bestLowerPaths[i][j]].angleBetween[index];
//            z= img.getBinForAngle(angle);
            
            
            curX = nextX;
            curY = nextY;
            nextX = skeleton[bestLowerPaths[i][j+1]].x;
            nextY = skeleton[bestLowerPaths[i][j+1]].y;
            
            
            //connect to next
            strengthenConnection2D(curX,curY,nextX,nextY,g,img,&test);
    //        int indexA = curX+curY*width;
    //        int indexB = nextX+nextY*width;
//            g->add_edge(indexA,indexB,DESC_BIAS_LEN_2D,DESC_BIAS_LEN_2D);
            
            
        }
        

        
    }
    
    test.save("./test_descender_id.ppm");
    
//    char a;
//    printf("Cont? ");
//    scanf("%c",&a);
//    printf("ok\n");
}

void GraphCut::strengthenConnection2D(int curX, int curY, int nextX, int nextY, GraphType *g, const BPixelCollection &img, QImage *test)
{
//    printf("Connecting (%d,%d) to (%d,%d)\n",curX,curY,nextX,nextY);
    
    QVector<QPoint> line;
    QPoint start(curX,curY);
    line.append(start);
    if (curX==nextX || fabs((curY-nextY)/((double)curX-nextX)) > 1)
    {
        double slope = ((double)curX-nextX)/(curY-nextY);
        double intersect = curX-curY*slope;
        int inc = copysign(1.0, nextY-curY);
        for (int y=curY+inc; inc*y<inc*nextY; y+=inc)
        {
            QPoint toAdd(y*slope+intersect,y);
            if (img.pixel(toAdd))
                line.append(toAdd);
            else
            {
//                return;
            }
            test->setPixel(toAdd,test->colorTable().size()-1);
        }
    }
    else
    {
        double slope = (curY-nextY)/((double)curX-nextX);
        double intersect = curY-curX*slope;
        int inc = copysign(1.0, nextX-curX);
        for (int x=curX+inc; inc*x<inc*nextX; x+=inc)
        {
            QPoint toAdd(x,slope*x+intersect);
            if (img.pixel(toAdd))
                line.append(toAdd);
            else
            {
//                return;
            }
            test->setPixel(toAdd,test->colorTable().size()-1);
        }
    }
    QPoint end(nextX,nextY);
    line.append(end);
    
    g->add_tweights(line[0].x() + line[0].y()*img.width(),DESC_BIAS_T_2D,0);
    for (int i=1; i<line.size(); i++)
    {
        int indexA=line[i-1].x() + line[i-1].y()*img.width();
        int indexB=line[i].x() + line[i].y()*img.width();
        g->add_edge(indexA,indexB,DESC_BIAS_LEN_2D,DESC_BIAS_LEN_2D);
        g->add_tweights(indexB,DESC_BIAS_T_2D,0);
    }
}

void GraphCut::lowerDescenderTraverser(const BlobSkeleton &skeleton, QVector<QVector<unsigned int> >* bestLowerPaths, QVector<double>* bestLowerScores, QVector<QVector<unsigned int> >* bestUpperPaths, QVector<double>* bestUpperScores, const QVector<unsigned int>* currentPath, double clockwiseScore, PathStackMap* upperPaths)
{
    
    int startUpperStack = upperPaths->size();
    
    if (currentPath->size() > 2)
    {
//        printf("[L] Current path is: ");
//        foreach (unsigned int i, *currentPath)
//            printf("(%d,%d), ",skeleton[i].x,skeleton[i].y);
//        printf("\n");
        bool print = /*(currentPath->size()==5 && 
                skeleton[currentPath->at(3)].x==46 && skeleton[currentPath->at(3)].y==54 &&
                skeleton[currentPath->at(4)].x==42 && skeleton[currentPath->at(4)].y==54);*/
//                ||
//        (currentPath->size()==5 && 
//         skeleton[currentPath->at(4)].x==47 && skeleton[currentPath->at(4)].y==51 &&       
//         skeleton[currentPath->at(3)].x==52 && skeleton[currentPath->at(3)].y==45 &&
//                skeleton[currentPath->at(2)].x==54 && skeleton[currentPath->at(2)].y==37);
//                (currentPath->size()==5 &&
//                skeleton[currentPath->at(4)].x==11 && skeleton[currentPath->at(4)].y==69 &&
//                 skeleton[currentPath->at(3)].x==21 && skeleton[currentPath->at(3)].y==62 &&
//                 skeleton[currentPath->at(2)].x==24 && skeleton[currentPath->at(2)].y==52)
//                ((/*currentPath->size()==4 || */(currentPath->size()==5 && skeleton[currentPath->at(4)].x==47 && skeleton[currentPath->at(4)].y==49)) &&  
//                 skeleton[currentPath->at(3)].x==52 && skeleton[currentPath->at(3)].y==51 &&
//                    skeleton[currentPath->at(1)].x==34 && skeleton[currentPath->at(1)].y==42 &&
//                    skeleton[currentPath->at(2)].x==60 && skeleton[currentPath->at(2)].y==47);
//                ||
//                (currentPath->size()>=3 &&  
//                 skeleton[currentPath->at(4)].x==13 && skeleton[currentPath->at(4)].y==56 &&
//                    skeleton[currentPath->at(1)].x==55 && skeleton[currentPath->at(1)].y==28 &&
//                    skeleton[currentPath->at(2)].x==60 && skeleton[currentPath->at(2)].y==47);
                (currentPath->size()==11 && 
                 skeleton[currentPath->at(5)].x==28 && skeleton[currentPath->at(5)].y==52 &&
                 skeleton[currentPath->at(6)].x==24 && skeleton[currentPath->at(6)].y==55 &&
                 skeleton[currentPath->at(8)].x==23 && skeleton[currentPath->at(8)].y==64 &&
                 skeleton[currentPath->at(9)].x==25 && skeleton[currentPath->at(9)].y==69 &&
                 skeleton[currentPath->at(10)].x==20 && skeleton[currentPath->at(10)].y==74);
        
        //get score
        QVector<double> x;
        QVector<double> y;
        int sampleSize=extractSampleFromPath(skeleton,currentPath,&x,&y);
        double currentScore = computeScore(sampleSize,x.data(),y.data(),LOWER_MEAN_SLOPE,LOWER_STD_DEV_SLOPE,LOWER_MEAN_CURVE,LOWER_STD_DEV_CURVE,print);
        
        for (int i=0; i< upperPaths->size(currentPath->back()); i++)
        {
        
            double upperScore = upperPaths->at(currentPath->back(),i).score;
            if (upperScore<SCORE_THRESH && currentScore<SCORE_THRESH && upperScore+currentScore<COMBINE_SCORE_THRESH)
            {
                bool goodMatch = true;
                for (int j=0; j<currentPath->size()-1; j++)
                {
                    if (upperPaths->at(currentPath->back(),i).path.contains(currentPath->at(j)) &&
                            upperPaths->at(currentPath->back(),i).path[0] != currentPath->at(j))
                    {
                        goodMatch = false;
                    }
                }
                if (goodMatch)
                {
                    bestLowerPaths->append(*currentPath);
                    bestLowerScores->append(currentScore);
                    bestUpperPaths->append(upperPaths->at(currentPath->back(),i).path);
                    bestUpperScores->append(upperScore);
//                    break;
                }
            }
            else
            {
                break;
            }
        }
        
    }
    
    //check options
    double largestAngle=0;
    int largestAngleIndex=-1;
    QMap<unsigned int, double> toExpand;
    foreach (unsigned int nextIndex, skeleton[currentPath->last()].connectedPoints())
    {
        if (currentPath->contains(nextIndex))
            continue;
        
        if (currentPath->size()>1)
        {
            double relativeAngle = getRelAngle(skeleton, currentPath->at(currentPath->size()-2), currentPath->last(), nextIndex);
            double distance = getDist(skeleton, currentPath->last(), nextIndex);
            double newClockwiseScore = (clockwiseScore/1.5)+std::min(PI-relativeAngle,PI/3)*distance;
            
            if (newClockwiseScore > CLOCKWISE_THRESH)
            {
                toExpand[nextIndex]=newClockwiseScore;
            }
            
            if (relativeAngle > largestAngle)
            {
                largestAngle = relativeAngle;
                largestAngleIndex = nextIndex;
            }
            
        }
        else if (skeleton[nextIndex].y >= skeleton[currentPath->last()].y)
        {
            toExpand[nextIndex]=0;
        }
    }
    
    //expand upper branches
//    if (skeleton[currentPath->last()].connectedPoints.size()>2 && largestAngleIndex!=-1)
    {
        foreach (unsigned int nextIndex, skeleton[currentPath->last()].connectedPoints())
        {
            if (nextIndex==largestAngleIndex || currentPath->contains(nextIndex))
                continue;
            
            
            QVector<unsigned int> newPath;// = *currentPath;
            newPath.append(currentPath->last());
            newPath.append(nextIndex);
            upperDescenderTraverser(skeleton,&newPath,0,upperPaths);
        }
    }
    
    //expand lower branches
    foreach (unsigned int nextIndex, toExpand.keys())
    {
        QVector<unsigned int> newPath = *currentPath;
        newPath.append(nextIndex);
        lowerDescenderTraverser(skeleton,bestLowerPaths,bestLowerScores,bestUpperPaths,bestUpperScores,&newPath,toExpand[nextIndex],upperPaths);
    }
    
    
    //remove upper branchs we spawned
    while (upperPaths->size()>startUpperStack)
    {
        upperPaths->pop_back();
    }
    
}

void GraphCut::upperDescenderTraverser(const BlobSkeleton &skeleton, const QVector<unsigned int>* currentPath, double counterClockwiseScore, PathStackMap* upperPaths)
{
    
    
    if (currentPath->size() > 2)
    {
//        printf("[U] Current path is: ");
//        foreach (int i, *currentPath)
//            printf("(%d,%d), ",skeleton[i].x,skeleton[i].y);
//        printf("\n");
        bool print = /*(currentPath->size()==3 && 
                skeleton[currentPath->at(0)].x==42 && skeleton[currentPath->at(0)].y==43 &&
                skeleton[currentPath->at(1)].x==40 && skeleton[currentPath->at(1)].y==50 &&
                skeleton[currentPath->at(2)].x==42 && skeleton[currentPath->at(2)].y==54 &&
                skeleton[currentPath->at(3)].x==46 && skeleton[currentPath->at(3)].y==54);*/
//                ||
                (currentPath->size()==11 && 
                                skeleton[currentPath->at(0)].x==41 && skeleton[currentPath->at(0)].y==25 &&
                                skeleton[currentPath->at(1)].x==29 && skeleton[currentPath->at(1)].y==28 &&
                                skeleton[currentPath->at(10)].x==20 && skeleton[currentPath->at(10)].y==74 /*&&
                                skeleton[currentPath->at(5)].x==25 && skeleton[currentPath->at(5)].y==69 &&
                                skeleton[currentPath->at(5)].x==47 && skeleton[currentPath->at(5)].y==51*/);
//                (currentPath->size()==4 && 
//                    skeleton[currentPath->at(0)].x==30 && skeleton[currentPath->at(0)].y==33 &&
//                    skeleton[currentPath->at(1)].x==25 && skeleton[currentPath->at(1)].y==39 &&
//                    skeleton[currentPath->at(3)].x==27 && skeleton[currentPath->at(3)].y==52 )
//                ||
//                (currentPath->size()==5 && 
//                 skeleton[currentPath->at(0)].x==30 && skeleton[currentPath->at(0)].y==33 &&
//                 skeleton[currentPath->at(1)].x==25 && skeleton[currentPath->at(1)].y==39 &&
//                 skeleton[currentPath->at(3)].x==27 && skeleton[currentPath->at(3)].y==52 &&
//                 skeleton[currentPath->at(4)].x==38 && skeleton[currentPath->at(4)].y==55);
        
        //get score
        
        QVector<double> x;
        QVector<double> y;
        int sampleSize=extractSampleFromPath(skeleton,currentPath,&x,&y);
        double currentScore = computeScore(sampleSize,x.data(),y.data(),UPPER_MEAN_SLOPE,UPPER_STD_DEV_SLOPE,UPPER_MEAN_CURVE,UPPER_STD_DEV_CURVE,print);
        //compare score
//        if (currentScore < *bestUpperScore)
//        {
//            bestUpperPath->clear();
//            (*bestUpperPath) += *currentPath;
//            *bestUpperScore = currentScore;
//        }
        pathAndScore p;
        p.path=*currentPath;
        p.score=currentScore;
        upperPaths->push_back(p);
        
        
//        printf("[U] Current cclockwiseScore is: %f\tCurrent score is: %f,\tchi^2 is:%f\n",counterClockwiseScore,currentScore,chisq);
    }
    
    //check options, expand
    foreach (unsigned int nextIndex, skeleton[currentPath->last()].connectedPoints())
    {
        if (currentPath->contains(nextIndex))
            continue;
        
        double relativeAngle = getRelAngle(skeleton, currentPath->at(currentPath->size()-2), currentPath->last(), nextIndex);
        double distance = getDist(skeleton, currentPath->last(), nextIndex);
        double newCounterClockwiseScore = (counterClockwiseScore/1.5)+std::min(relativeAngle - PI,PI/3)*distance;
        
        if (newCounterClockwiseScore > COUNTER_CLOCKWISE_THRESH)
        {
            QVector<unsigned int> newPath = *currentPath;
            newPath.append(nextIndex);
            upperDescenderTraverser(skeleton,&newPath,newCounterClockwiseScore,upperPaths);
        }
        
//        if (skeleton[currentPath->last()].x==27 && skeleton[currentPath->last()].y==52)
//        {
//            printf("score to [%d]:(%d,%d) is %f\n",nextIndex,skeleton[nextIndex].x,skeleton[nextIndex].y,newCounterClockwiseScore);
//                    printf("[U] Current path is: ");
//                    foreach (int i, *currentPath)
//                        printf("(%d,%d), ",skeleton[i].x,skeleton[i].y);
//                    printf("\n");
//        }
        
    }
}

int GraphCut::extractSampleFromPath(const BlobSkeleton &skeleton, const QVector<unsigned int>* currentPath, QVector<double>* xOut, QVector<double>* yOut)
{
    int sampleSize=currentPath->size();
    for (int i=0; i < currentPath->size(); i++)
    {
        yOut->append( skeleton[currentPath->at(i)].x );
        xOut->append( skeleton[currentPath->at(i)].y );
    }
    return sampleSize;
    
    //or
    
//    QVector<QPoint> samplePoints;
//    for (int i=0; i < currentPath->size(); i++)
//    {
//        samplePoints += skeleton.getRegion(currentPath->at(i));
//    }
//    foreach (QPoint p, samplePoints)
//    {
//        yOut->append(p.x());
//        xOut->append(p.y());
//    }
//    return samplePoints.size();
}

/*from < http://rosettacode.org/wiki/Polynomial_regression#C >*/

double GraphCut::polynomialfit(int obs, int degree, 
		   double *dx, double *dy, double *store, double *covarience) /* n, p */
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
