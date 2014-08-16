#include "graphcut.h"



int maxedge;
int maxweight;
int minweight;
int minedge;

inline void setEdge(int x1, int y1, int x2, int y2, GraphType* g, const BPixelCollection &img, int* invDistMap, double blackToBlackBias, double whiteToBlackBias, double blackToWhiteBias, double whiteToWhiteBias, double reducer, int width)
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
    
//    QImage debug = img.makeImage().getImage();
//    QVector<QRgb> ct = debug.colorTable();
//    ct.append(qRgb(205,50,50));
//    ct.append(qRgb(50,205,50));
//    debug.setColorTable(ct);
    
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
//                    debug.setPixel(i,j,2);
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
//                    debug.setPixel((width-1)-(o-i),(height-1)-i,3);
                    
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
    
//    debug.save("./anchors.ppm");
    
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
    int NEW_ANCHOR = 80;
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
int GraphCut::pixelsOfSeparation(int* invDistMap, int width, int height, const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight, int split_method, int vert_divide)
{
    int NEW_ANCHOR = 80;
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

int GraphCut::pixelsOfSeparation(int* invDistMap, int width, int height, const BPixelCollection &img, const AngleImage &angleImage, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight, int split_method, int vert_divide)
{
    int NEW_ANCHOR = 80;
    
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

int GraphCut::pixelsOfSeparation(const long* invDistMap3D, int width, int height, int depth, const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, const QPoint &crossOverPoint, int anchor_weight, int split_method)
{
    int NEW_ANCHOR = 80;
    
//    QVector<QVector<QVector<double> > > image3d = make3dImage(img,invDistMap,angleImage);
    
    
    
    int numNodes = width*height*depth;
    int numEdges = 4*(width-1)*(height-1)-(height+width);
    numEdges *= depth;
    numEdges += numNodes*(depth-1);//assuming only striaght (no diag) connections for higher dimensions
    
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
    
    
    
    //connect all pixels
    //For simplicity, only doing three dimensions now
//    double FLAT_WEIGHT = .5;
//    double SLOPE_WEIGHT = 4;
    double FLAT_WEIGHT = 1;//1.5//6
    double SLOPE_WEIGHT = .5;
    int slope_size = depth;
    for (int k=0; k<slope_size; k++)
    {
        
        for (int i=0; i<width; i++)
        {
            for (int j=0; j<height; j++)
            {   
                
                //C1
                if (i+1<width)
                {
                    setEdge3DMap(i,j,k,i+1,j,k,g,indexer,invDistMap3D,FLAT_WEIGHT);
                }
                
                if (j+1<height)
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
                if (j>0 && i<width-1)
                {
                    setEdge3DMap(i,j,k,i+1,j-1,k,g,indexer,invDistMap3D,2*FLAT_WEIGHT-sqrt(2*pow(FLAT_WEIGHT,2)));
                }
                
                if (j<height-1 && i<width-1)
                {
                    setEdge3DMap(i,j,k,i+1,j+1,k,g,indexer,invDistMap3D,2*FLAT_WEIGHT-sqrt(2*pow(FLAT_WEIGHT,2)));
                }
                
                
                
                //C3 nope
                
            }//j
        }//i
    }//k
    
    strengthenDescenderComponent(img,crossOverPoint,g,indexer,depth);
    
    int ret = g -> maxflow();

    
    //add all black pixels which
    int slopeDifRange = SLOPE_DIF_TOLERANCE*depth;
    int THRESH=700;
    for (int x=0; x<width; x++)
    {
//        printf("\n");
        for (int y=0; y<height; y++)
        {
            int sourceScore=0;
            int sinkScore=0;
            for (int z=0; z<depth; z++)
            {
                int index = indexer.getIndex(x,y,z);
                if (g->what_segment(index) == GraphType::SOURCE)
                    sourceScore += invDistMap3D[index];
                else
                    sinkScore += invDistMap3D[index];
            }
            
//            printf("(%d,%d) ",sourceScore,sinkScore);
            
            if (sourceScore>=THRESH || sinkScore<THRESH && sourceScore>sinkScore)
                outSource.append(x+width*y);
            if (sinkScore>=THRESH || sourceScore<THRESH && sinkScore>sourceScore)
                outSink.append(x+width*y);
            
            
        }
    }
    
    
    ///test///
//    for (int s=0; s<depth; s++)
//    {
//        BImage test(img.width(), img.height());
//        BPartition tmp1((BPixelCollection*) &test);
//        BPartition tmp2((BPixelCollection*) &test);
//        for (int x=0; x<width; x++)
//        {
//            for (int y=0; y<height; y++)
//            {
////                QMap<int,double> binsForDim = angleImage.getBinsAndStrForPixel(x,y);
////                foreach (int bin, binsForDim.keys())
////                {
////                    if (bin<s+11 && bin>s-11)
////                        test.setPixel(x,y,true);
////                }
//                if (img.pixel(x,y))
//                    test.setPixel(x,y,true);
                
//                int index = indexer.getIndex(x,y,s);
//                if (g->what_segment(index) == GraphType::SOURCE)
//                    tmp1.addPixelFromSrc(x,y);
//                else
//                    tmp2.addPixelFromSrc(x,y);           
//            }
//        }
//        test.claimOwnership(&tmp2,1);
//        test.claimOwnership(&tmp1,1);
        
//        QString debugfile = "./output/layer_";
//        QString num;
//        num.setNum(s);
//        debugfile.append(num);
//        debugfile.append(".ppm");
//        test.saveOwners(debugfile);
        
//    }
    ///test///
    
    
    delete g;
    return ret;
}


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

double GraphCut::computeScore(int sampleSize, double* x, double* y, double meanSlope, double meanCurve, bool print)
{
    double linOut[2];
    double chisqSlope= polynomialfit(sampleSize,2,x,y,linOut);
    double slope=linOut[1];
    
    double quadOut[3];
    double chisqCurve = polynomialfit(sampleSize,3,x,y,quadOut);
    double curvature=quadOut[2];
    
    double score = (copysign(1.0, curvature) == copysign(1.0, meanCurve) && 
                    copysign(1.0, slope) == copysign(1.0, meanSlope)) ? 
                        (.75*pow(1.2,chisqCurve/20))*fabs(curvature-meanCurve) + (.1*pow(1.2,chisqSlope/30))*fabs(slope-meanSlope) : DOUBLE_POS_INFINITY;
    
    
    if (print && score!=DOUBLE_POS_INFINITY)
    {
        if (meanSlope==UPPER_MEAN_SLOPE)
        {
            printf("[U] ");
        }
        else
            printf("[L] ");
        printf("Slope=%f\tchi^2=%f\tCurve=%f\tchi^2=%f\tscore=%f\n",fabs(slope-meanSlope),chisqSlope,fabs(curvature-meanCurve),chisqCurve,score);
    }
    
    return score;
}


void recurStr(int strFactor, const BlobSkeleton &skeleton, int curIndex, int prevIndex, bool* notVisited, GraphType *g, const Indexer3D &indexer,int depth)
{
    int internalIndex=0;
    foreach (int nextIndex, skeleton[curIndex].connectedPoints)
    {
        if (notVisited[nextIndex])
        {
            notVisited[nextIndex]=false;
            int bin = depth*(skeleton[curIndex].angleBetween[internalIndex++]/PI);
            double relativeAngle = getRelAngle(skeleton, prevIndex, curIndex, nextIndex);
            int newStrFactor = std::min(10,(int)(strFactor*std::max(.5,PI/relativeAngle)));
//            g -> add_edge(indexer.getIndex(skeleton[curIndex].x,skeleton[curIndex].y,bin), indexer.getIndex(skeleton[nextIndex].x,skeleton[nextIndex].y,bin),
//                          INV_A*newStrFactor/*skeleton[curIndex].distanceBetween[nextIndex]*/,
//                          INV_A*newStrFactor/*skeleton[curIndex].distanceBetween[nextIndex]*/);
//            recurStr(newStrFactor,skeleton,nextIndex,curIndex,notVisited,g,indexer,depth);
        }
    }
}

void GraphCut::strengthenDescenderComponent(const BPixelCollection &img, const QPoint &crossOverPoint, GraphType *g, const Indexer3D &indexer,int numAngleValues)
{
    assert(img.pixel(crossOverPoint));
    
    BlobSkeleton skeleton(&img);
    int startRegionId = skeleton.regionIdForPoint(crossOverPoint);
//    bool notVisited[skeleton.numberOfVertices()];
//    for (int i=0; i<skeleton.numberOfVertices(); i++)
//        notVisited[i]=true;
//    notVisited[startRegionId]=false;
    QVector<unsigned int> bestLowerPath;
    double bestLowerScore=DOUBLE_POS_INFINITY;
    QVector<unsigned int> bestUpperPath;
    double bestUpperScore=DOUBLE_POS_INFINITY;
    
    PathStackMap upperPaths;
    
    foreach (unsigned int curIndex, skeleton[startRegionId].connectedPoints)
    {
        if (skeleton[curIndex].y>=crossOverPoint.y())
        {
            QVector<unsigned int> newPath;
            newPath.append(startRegionId);
            newPath.append(curIndex);
            lowerDescenderTraverser(skeleton,&bestLowerPath,&bestLowerScore,&bestUpperPath,&bestUpperScore,&newPath,0,&upperPaths);
//            foreach (int nextIndex, skeleton[curIndex].connectedPoints)
//            {
//                if (skeleton[nextIndex].y>=crossOverPoint.y())
//                {
//                    QVector<unsigned int> newPath;
//                    newPath.append(startRegionId);
//                    newPath.append(curIndex);
//                    newPath.append(nextIndex);
//                    lowerDescenderTraverser(skeleton,&bestLowerPath,&bestLowerScore,&bestUpperPath,&bestUpperScore,&newPath,0);
//                }
//            }
        }
    }
    
    
    //TODO: something about it
    printf("[U] Best path (%f) is: ",bestUpperScore);
    foreach (unsigned int i, bestUpperPath)
        printf("(%d,%d), ",skeleton[i].x,skeleton[i].y);
    printf("\n");
    printf("[L] Best path (%f) is: ",bestLowerScore);
    foreach (unsigned int i, bestLowerPath)
        printf("(%d,%d), ",skeleton[i].x,skeleton[i].y);
    printf("\n");
    
    char a;
    printf("Cont? ");
    scanf("%c",&a);
    printf("ok\n");
}

void GraphCut::lowerDescenderTraverser(const BlobSkeleton &skeleton, QVector<unsigned int>* bestLowerPath, double* bestLowerScore, QVector<unsigned int>* bestUpperPath, double* bestUpperScore, const QVector<unsigned int>* currentPath, double clockwiseScore, PathStackMap* upperPaths)
{
    
    int startUpperStack = upperPaths->size();
    
    if (currentPath->size() > 2)
    {
//        printf("[L] Current path is: ");
//        foreach (unsigned int i, *currentPath)
//            printf("(%d,%d), ",skeleton[i].x,skeleton[i].y);
//        printf("\n");
        bool print = (currentPath->size()==4 && 
                skeleton[currentPath->at(3)].x==52 && skeleton[currentPath->at(3)].y==45 &&
                skeleton[currentPath->at(2)].x==54 && skeleton[currentPath->at(2)].y==37)
                ||
        (currentPath->size()==5 && 
         skeleton[currentPath->at(4)].x==47 && skeleton[currentPath->at(4)].y==51 &&       
         skeleton[currentPath->at(3)].x==52 && skeleton[currentPath->at(3)].y==45 &&
                skeleton[currentPath->at(2)].x==54 && skeleton[currentPath->at(2)].y==37);
//                (currentPath->size()==7 && 
//                 skeleton[currentPath->at(6)].x==2 && skeleton[currentPath->at(6)].y==71 &&
//                 skeleton[currentPath->at(5)].x==11 && skeleton[currentPath->at(5)].y==69 &&
//                 skeleton[currentPath->at(4)].x==11 && skeleton[currentPath->at(4)].y==61 &&
//                 skeleton[currentPath->at(3)].x==21 && skeleton[currentPath->at(3)].y==62 &&
//                 skeleton[currentPath->at(2)].x==24 && skeleton[currentPath->at(2)].y==52);
        
        //get score
        QVector<double> x;
        QVector<double> y;
        int sampleSize=extractSampleFromPath(skeleton,currentPath,&x,&y);
        double currentScore = computeScore(sampleSize,x.data(),y.data(),LOWER_MEAN_SLOPE,LOWER_MEAN_CURVE,print);
        //compare score
//        if (currentScore < *bestLowerScore)
//        {
//            bestLowerPath->clear();
//            (*bestLowerPath) += *currentPath;
//            *bestLowerScore=currentScore;
//        }
        double upperScore = (*upperPaths)[currentPath->back()].score;
        if (upperScore!=-1 && upperScore+currentScore<*bestUpperScore+*bestLowerScore)
        {
            bestLowerPath->clear();
            (*bestLowerPath) += *currentPath;
            *bestLowerScore=currentScore;
            bestUpperPath->clear();
            (*bestUpperPath) += (*upperPaths)[currentPath->back()].path;
            *bestUpperScore=upperScore;
        }
        
    }
    
    //check options
    double largestAngle=0;
    int largestAngleIndex=-1;
    QMap<unsigned int, double> toExpand;
    foreach (unsigned int nextIndex, skeleton[currentPath->last()].connectedPoints)
    {
        if (currentPath->contains(nextIndex))
            continue;
        
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
    
    //expand upper branches
    if (skeleton[currentPath->last()].connectedPoints.size()>2 && largestAngleIndex!=-1)
    {
        foreach (unsigned int nextIndex, skeleton[currentPath->last()].connectedPoints)
        {
            if (nextIndex==largestAngleIndex)
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
        lowerDescenderTraverser(skeleton,bestLowerPath,bestLowerScore,bestUpperPath,bestUpperScore,&newPath,toExpand[nextIndex],upperPaths);
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
        bool print = (currentPath->size()==5 && 
                skeleton[currentPath->at(0)].x==55 && skeleton[currentPath->at(0)].y==28 &&
                skeleton[currentPath->at(1)].x==48 && skeleton[currentPath->at(1)].y==32 &&
                skeleton[currentPath->at(3)].x==47 && skeleton[currentPath->at(3)].y==41 &&
                skeleton[currentPath->at(4)].x==52 && skeleton[currentPath->at(4)].y==45)
                ||
                (currentPath->size()==6 && 
                                skeleton[currentPath->at(0)].x==55 && skeleton[currentPath->at(0)].y==28 &&
                                skeleton[currentPath->at(1)].x==48 && skeleton[currentPath->at(1)].y==32 &&
                                skeleton[currentPath->at(3)].x==47 && skeleton[currentPath->at(3)].y==41 &&
                                skeleton[currentPath->at(4)].x==45 && skeleton[currentPath->at(4)].y==44 &&
                                skeleton[currentPath->at(5)].x==47 && skeleton[currentPath->at(5)].y==51);
//                (currentPath->size()==7 && 
//                    skeleton[currentPath->at(0)].x==21 && skeleton[currentPath->at(0)].y==62 &&
//                    skeleton[currentPath->at(1)].x==13 && skeleton[currentPath->at(1)].y==56 &&
//                    skeleton[currentPath->at(2)].x==16 && skeleton[currentPath->at(2)].y==52 &&
//                    skeleton[currentPath->at(3)].x==19 && skeleton[currentPath->at(3)].y==47 &&
//                    skeleton[currentPath->at(4)].x==26 && skeleton[currentPath->at(4)].y==43 &&
//                    skeleton[currentPath->at(6)].x==39 && skeleton[currentPath->at(6)].y==30);
        
        //get score
        
        QVector<double> x;
        QVector<double> y;
        int sampleSize=extractSampleFromPath(skeleton,currentPath,&x,&y);
        double currentScore = computeScore(sampleSize,x.data(),y.data(),UPPER_MEAN_SLOPE,UPPER_MEAN_CURVE,print);
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
    foreach (unsigned int nextIndex, skeleton[currentPath->last()].connectedPoints)
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
    
    QVector<QPoint> samplePoints;
    for (int i=0; i < currentPath->size(); i++)
    {
        samplePoints += skeleton.getRegion(currentPath->at(i));
    }
    foreach (QPoint p, samplePoints)
    {
        yOut->append(p.x());
        xOut->append(p.y());
    }
    return samplePoints.size();
}

/*from < http://rosettacode.org/wiki/Polynomial_regression#C >*/

double GraphCut::polynomialfit(int obs, int degree, 
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
  return chisq; /* we do not "analyse" the result (cov matrix mainly)
		  to know if the fit is "good" */
}
