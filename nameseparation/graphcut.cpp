#include "graphcut.h"

typedef Graph<int,int,int> GraphType;

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
    
    QImage debug = img.makeImage().getImage();
    QVector<QRgb>ct =debug.colorTable();
    ct.append(qRgb(150,150,150));
    debug.setColorTable(ct);
    
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
                    debug.setPixel(i,j,2);
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
                    debug.setPixel((width-1)-(o-i),(height-1)-i,2);
                    
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
    
    debug.save("./anchors.ppm");
    
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
    double BLACK_TO_BLACK_D_BIAS = sqrt(pow(BLACK_TO_BLACK_V_BIAS,2)+pow(BLACK_TO_BLACK_H_BIAS,2));
    double WHITE_TO_BLACK_BIAS = .5;
    double BLACK_TO_WHITE_BIAS = .5;
    double WHITE_TO_WHITE_V_BIAS = .5;
    double WHITE_TO_WHITE_H_BIAS = .5;
    double WHITE_TO_WHITE_D_BIAS = sqrt(pow(WHITE_TO_WHITE_V_BIAS,2)+pow(WHITE_TO_WHITE_H_BIAS,2));
    
    if (split_method==SPLIT_VERT)
    {
        BLACK_TO_BLACK_V_BIAS = .5;
        BLACK_TO_BLACK_H_BIAS = .5;
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

inline int mod(int a, int b)
{
    while (a<0)
        a+=b;
    return a%b;
}

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
                        slope[k]=0;
                    }
                    foreach (int bin, binsForDim)
                    {
//                        int secondBin = bin/dimensions.getBinNums()[0];
//                        bin = bin%dimensions.getBinNums()[0];
                        
                        slope[bin]=invDistMap[x+y*img.width()];
                        for (int kb=1; kb<slopeDifRange; kb++)
                        {
                            slope[mod((bin+kb),dimensions.getBinNums()[0])]+=invDistMap[x+y*img.width()]*((slopeDifRange-kb)/(1.0*slopeDifRange));
                            slope[mod((bin-kb),dimensions.getBinNums()[0])]+=invDistMap[x+y*img.width()]*((slopeDifRange-kb)/(1.0*slopeDifRange));
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
    printf("testMat=%f\n",testMax);
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

int GraphCut::pixelsOfSeparationNDimensions(int* invDistMap, int width, int height, const BPixelCollection &img, const NDimensions &dimensions, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight, int split_method, int vert_divide)
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
    ct.append(qRgb(255,150,150));
    ct.append(qRgb(150,255,150));
    debug.setColorTable(ct);
    
    if (split_method == SPLIT_HORZ)
    {
        //find source pixels
        int count_source = 50;//height*ANCHOR_L;
        for (int j=0; count_source>0 && j<height; j++)
        {
            for (int i=0; count_source>0 && i<width; i++)
            {
                if (img.pixel(i,j))
                {
                    int index = indexer.getIndex(i,j);
                    g -> add_tweights(index, anchor_weight,0);//invDistMap[index], 0);
                    debug.setPixel(i,j,2);
                    count_source--;
                }
            }
        }
        
        
        
        
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
    //    workingStack.clear();
        
        int count_sink=50;//height*ANCHOR_R;
        
        //find sink pixels
        for (int j=height-1; count_sink>0 && j>=0; j--)
        {
            for (int i=width-1; count_sink>0 && i>=0; i--)
            {
                if (img.pixel(i,j))
                {
                    int index = i+width*j;
                    g -> add_tweights(index, 0, INT_POS_INFINITY);//invDistMap[index]);
                    debug.setPixel(i,j,3);
                    count_sink--;
                }
            }
        }
        
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
        debug.save("./anchors.ppm");
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
                    int index = indexer.getIndex(i,j);
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
                    int index = indexer.getIndex(i,j);
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
                    int index = indexer.getIndex(i,j);
                    g -> add_tweights(index, (int)src_anchor_weight_for_level,(int)sink_anchor_weight_for_level);
    //                debug.setPixel(i,j,150);
                }
            }
        }
    }
    
    
    
    //connect all pixels
    //For simplicity, only doing three dimensions now
    double FLAT_WEIGHT = 1;
    double SLOPE_WEIGHT = 1.2;
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
                    setEdge3d(i,j,k,i+1,j-1,k,g,indexer,image3d,sqrt(2*FLAT_WEIGHT));
                }
                
                if (j<height-1 && i<width-1)
                {
                    setEdge3d(i,j,k,i+1,j+1,k,g,indexer,image3d,sqrt(2*FLAT_WEIGHT));
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
                    bool onSource = false;
                    bool onSink = false;
                    foreach (int bin, binsForDim)
                    {
                        int index = indexer.getIndex(x,y,bin);
                        
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
        BImage test(img);
        BPartition tmp1((BPixelCollection*) &test);
        BPartition tmp2((BPixelCollection*) &test);
        for (int x=0; x<width; x++)
        {
            for (int y=0; y<height; y++)
            {
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
