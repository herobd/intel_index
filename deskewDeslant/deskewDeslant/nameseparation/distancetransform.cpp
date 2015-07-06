#include "distancetransform.h"

#include <opencv2/viz/vizcore.hpp>

using namespace std;

//Meijster distance <http://fab.cba.mit.edu/classes/S62.12/docs/Meijster_distance.pdf>
//This can be parallelized. Should probably flip from column to row first
void DistanceTransform::computeInverseDistanceMap(const BPixelCollection &src, int* out)
{
    int maxDist=0;
    int g[src.width()*src.height()];
    for (int x=0; x<src.width(); x++)
    {
        if (src.pixel(x,0))
        {
            g[x+0*src.width()]=0;
        }
        else
        {
            g[x+0*src.width()]=INT_POS_INFINITY;//src.width()*src.height();
        }
        
        for (int y=1; y<src.height(); y++)
        {
            if (src.pixel(x,y))
            {
                g[x+y*src.width()]=0;
            }
            else
            {
                if (g[x+(y-1)*src.width()] != INT_POS_INFINITY)
                    g[x+y*src.width()]=1+g[x+(y-1)*src.width()];
                else
                    g[x+y*src.width()] = INT_POS_INFINITY;
            }
        }
        
        for (int y=src.height()-2; y>=0; y--)
        {
            if (g[x+(y+1)*src.width()]<g[x+y*src.width()])
            {
                if (g[x+(y+1)*src.width()] != INT_POS_INFINITY)
                    g[x+y*src.width()]=1+g[x+(y+1)*src.width()];
                else
                    g[x+y*src.width()] = INT_POS_INFINITY;
            }
        }
        
        /*if(x==20)
        {
            for (int y=0; y<src.height(); y++)
            {
                printf("%d .. %d\n",qGray(src.pixel(x,y)),g[x+y*src.width()]);
            }
        }*/
    }
    
    int q;
    int s[src.width()];
    int t[src.width()];
    int w;
    for (int y=0; y<src.height(); y++)
    {
        q=0;
        s[0]=0;
        t[0]=0;
        for (int u=1; u<src.width();u++)
        {
            while (q>=0 && f(t[q],s[q],y,src.width(),g) > f(t[q],u,y,src.width(),g))
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
                w = SepPlusOne(s[q],u,y,src.width(),g);
                if (w<src.width())
                {
                    q++;
                    s[q]=u;
                    t[q]=w;
                }
            }
        }
        
        for (int u=src.width()-1; u>=0; u--)
        {
            out[u+y*src.width()]= f(u,s[q],y,src.width(),g);
            if (out[u+y*src.width()] > maxDist)
                maxDist = out[u+y*src.width()];
            if (u==t[q])
                q--;
        }
    }
    
    
    //    QImage debug(src.width(),src.height(),src.format());
    //    debug.setColorTable(src.colorTable());
    //    for (int i=0; i<debug.width(); i++)
    //    {
    //        for (int j=0; j<debug.height(); j++)
    //            debug.setPixel(i,j,(int)((pow(out[i+j*debug.width()],.2)/((double)pow(maxDist,.2)))*255));
            
    //    }
    //    debug.save("./reg_dist_map.pgm");
    //    printf("image format:%d\n",debug.format());
    
    //invert
//    printf("maxDist=%d\n",maxDist);
    maxDist++;
//    double normalizer = (25.0/maxDist);
    double e = 10;
    double b = 25;
    double m = 2000;
    double a = INV_A;
    int max_cc_size=500;
    
//    double normalizer = (b/m);
    BImage mark = src.makeImage();
    QVector<QPoint> workingStack;
    QVector<QPoint> growingComponent;
    
    
    int newmax = 0;
    int newmax2 = 0;
    int newmin = INT_MAX;
    for (int q = 0; q < src.width()*src.height(); q++)
    {   
        //out[q] = pow(6,24-out[q]*normalizer)/pow(6,20);
        if (src.pixel(q%src.width(),q/src.width()) && mark.pixel(q%src.width(),q/src.width()))
        {
            //fill bias
            QPoint p(q%src.width(),q/src.width());
            workingStack.push_back(p);
            mark.setPixel(p,false);
            while (!workingStack.isEmpty())
            {   
                QPoint cur = workingStack.back();
                workingStack.pop_back();
                growingComponent.append(cur);
                
                
                
                
                if (cur.x()>0 && mark.pixel(cur.x()-1,cur.y()))
                {
                    QPoint pp(cur.x()-1,cur.y());
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                
                
                if (cur.x()<mark.width()-1 && mark.pixel(cur.x()+1,cur.y()))
                {
                    QPoint pp(cur.x()+1,cur.y());
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                    
                }
                if (cur.y()<mark.height()-1 && mark.pixel(cur.x(),cur.y()+1))
                {
                    QPoint pp(cur.x(),cur.y()+1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                if (cur.y()>0 && mark.pixel(cur.x(),cur.y()-1))
                {
                    QPoint pp(cur.x(),cur.y()-1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                //diagonals
                if (cur.x()>0 && cur.y()>0 && mark.pixel(cur.x()-1,cur.y()-1))
                {
                    QPoint pp(cur.x()-1,cur.y()-1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                
                
                if (cur.x()<mark.width()-1 && cur.y()>0 && mark.pixel(cur.x()+1,cur.y()-1))
                {
                    QPoint pp(cur.x()+1,cur.y()-1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                    
                }
                if (cur.x()>0 && cur.y()<mark.height()-1 && mark.pixel(cur.x()-1,cur.y()+1))
                {
                    QPoint pp(cur.x()-1,cur.y()+1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                if (cur.x()<mark.width()-1 && cur.y()>0 && mark.pixel(cur.x()+1,cur.y()-1))
                {
                    QPoint pp(cur.x()+1,cur.y()-1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
            }
            int cc_size = growingComponent.size();
            while (!growingComponent.isEmpty())
            {
                QPoint cur = growingComponent.back();
                growingComponent.pop_back();
                int index = cur.x()+src.width()*cur.y();
                out[index] = pow(b-std::min(out[index]*(b/m),b),e)*a/(pow(b,e)) + std::min(cc_size,max_cc_size) + 1;
                
                if (out[index]>newmax)
                    newmax=out[index];
                
                if (out[index]>newmax2 && out[index]<newmax)
                    newmax2=out[index];
                
                if (out[index]<newmin)
                    newmin=out[index];
            }
        }
        else if (!src.pixel(q%src.width(),q/src.width()))
        {
            out[q] = pow(b-std::min(out[q]*(b/m),b),e)*a/(pow(b,e)) + 1;
        }

        if (out[q]>newmax)
            newmax=out[q];
        if (out[q]>newmax2 && out[q]<newmax)
            newmax2=out[q];
        if (out[q]<newmin)
            newmin=out[q];
    }
    
//    printf("newMax:%d, newMin:%d\n",newmax,newmin);
#if HORZ_SAVE_INV_DIST_MAP
//    QImage debug(src.width(),src.height(),QImage::Format_Indexed8);
//    QVector<QRgb> default_color_table;
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(i,i,i));
//    }
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(255,255,255));
//    }
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(255,255,255));
//    }
//    debug.setColorTable(default_color_table);
    QImage debug(src.width(),src.height(),QImage::Format_RGB32);
    
    for (int i=0; i<debug.width(); i++)
    {
        for (int j=0; j<debug.height(); j++)
        {
//            if (out[i+j*debug.width()]!=newmax)
//                debug.setPixel(i,j,(int)((out[i+j*debug.width()]/((double)newmax2))*254));
//            else
//                debug.setPixel(i,j,0);
            int val =(int)((out[i+j*debug.width()]/((double)newmax2))*(255));
            debug.setPixel(i,j,qRgb(val,val,val));
            if (out[i+j*debug.width()]!=newmax)
            {
                int val =(int)((out[i+j*debug.width()]/((double)newmax2))*(255));
                debug.setPixel(i,j,qRgb(val,val,val));
            }
            else
                debug.setPixel(i,j,qRgb(200,220,255));
        }
        
    }
    debug.save("./inv_dist_map.ppm");
#endif
}

void DistanceTransform::compute3DInverseDistanceMap(const bool* src, int* out, int width, int height, int depth)
{
    Indexer3D ind(width,height);
    int maxDist=0;
    int g[width*height*depth];
    
    
    
    //First pass
    for (int z=0; z<depth; z++)
    {
        for (int x=0; x<width; x++)
        {
            if (src[ind.getIndex(x,0,z)])
            {
                g[ind.getIndex(x,0,z)]=0;
            }
            else
            {
                g[ind.getIndex(x,0,z)]=INT_POS_INFINITY;//width*src.height();
            }
            
            for (int y=1; y<height; y++)
            {
                if (src[ind.getIndex(x,y,z)])
                {
                    g[ind.getIndex(x,y,z)]=0;
                }
                else
                {
                    if (g[ind.getIndex(x,y-1,z)] != INT_POS_INFINITY)
                        g[ind.getIndex(x,y,z)]=1+g[ind.getIndex(x,y-1,z)];
                    else
                        g[ind.getIndex(x,y,z)] = INT_POS_INFINITY;
                }
            }
            
            for (int y=height-2; y>=0; y--)
            {
                if (g[ind.getIndex(x,y+1,z)]<g[ind.getIndex(x,y,z)])
                {
                    if (g[ind.getIndex(x,y+1,z)] != INT_POS_INFINITY)
                        g[ind.getIndex(x,y,z)]=1+g[ind.getIndex(x,y+1,z)];
                    else
                        g[ind.getIndex(x,y,z)] = INT_POS_INFINITY;
                }
            }
        }
    }
    
    //second pass, compute dist map for each depth
    int g2[width*height*depth];
    
    for (int z=0; z<depth; z++)
    {
        int q;
        int s[width];
        int t[width];
        int w;
        for (int y=0; y<height; y++)
        {
            q=0;
            s[0]=0;
            t[0]=0;
            for (int u=1; u<width;u++)
            {
                while (q>=0 && f2D(t[q],s[q],y,z,ind,g) > f2D(t[q],u,y,z,ind,g))
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
                    w = SepPlusOne2D(s[q],u,y,z,ind,g);
                    if (w<width)
                    {
                        q++;
                        s[q]=u;
                        t[q]=w;
                    }
                }
            }
            
            for (int u=width-1; u>=0; u--)
            {
               g2[ind.getIndex(u,y,z)]= f2D(u,s[q],y,z,ind,g);
//                if (out[ind.getIndex(u,y,z)] > maxDist)
//                    maxDist = out[ind.getIndex(u,y,z)];
                if (u==t[q])
                    q--;
            }
        }
    }
    
    //third pass
    for (int x=0; x<width; x++)
    {
        int q;
        int s[depth];
        int t[depth];
        int w;
        for (int y=0; y<height; y++)
        {
            q=0;
            s[0]=0;
            t[0]=0;
            for (int u=1; u<depth;u++)
            {
                while (q>=0 && f3D(x,y,t[q],s[q],ind,g2) > f3D(x,y,t[q],u,ind,g2))
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
                    w = SepPlusOne3D(x,y,s[q],u,ind,g2);
                    if (w<depth)
                    {
                        q++;
                        s[q]=u;
                        t[q]=w;
                    }
                }
            }
            
            for (int u=depth-1; u>=0; u--)
            {
                out[ind.getIndex(x,y,u)]= f3D(x,y,u,s[q],ind,g2);
                if (out[ind.getIndex(x,y,u)] > maxDist)
                    maxDist = out[ind.getIndex(x,y,u)];
                if (u==t[q])
                    q--;
            }
        }
    }
    printf("maxdist:%d\n",maxDist);
    
    int newmax=0;
    double e = 15;
    double b = 30;
    double m = 444000;
    double a = INV_A;
    for (int i=0; i<width*height*depth; i++)
    {
        out[i] = pow(b-std::min(out[i]*(b/m),b),e)*a/(pow(b,e));
        if (out[i]>newmax)
            newmax=out[i];
    }
    
    QVector<QRgb> default_color_table;
    for (int i=0; i<255; i++)
    {
        default_color_table.append(qRgb(i,i,i));
    }
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(255,255,255));
//    }
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(255,255,255));
//    }
    for (int z=0; z<depth; z++)
    {
        QImage debug(width,height,QImage::Format_Indexed8);
        QImage debug2(width,height,QImage::Format_Indexed8);
        
        debug.setColorTable(default_color_table);
        debug2.setColorTable(default_color_table);
        for (int x=0; x<width; x++)
        {
            for (int y=0; y<debug.height(); y++)
            {
                debug.setPixel(x,y,(int)((out[ind.getIndex(x,y,z)]/((double)newmax))*254));
                if (src[ind.getIndex(x,y,z)])
                    debug2.setPixel(x,y,0);
                else
                    debug2.setPixel(x,y,254);
            }
            
        }
        QString debugfile = "./dist_3d/layer_";
        QString debugfile2 = "./output/layer_";
        QString num;
        num.setNum(z);
        debugfile.append(num);
        debugfile.append(".ppm");
        debugfile2.append(num);
        debugfile2.append(".ppm");
        debug.save(debugfile);
        debug2.save(debugfile2);
    }
}

int DistanceTransform::f(int x, int i, int y, int m, int* g)
{
    if (g[i+y*m]==INT_POS_INFINITY || x==INT_POS_INFINITY)
        return INT_POS_INFINITY;
    return (x-i)*(x-i) + g[i+y*m]*g[i+y*m];
}

int DistanceTransform::SepPlusOne(int i, int u, int y, int m, int* g)
{
    if (g[u+y*m] == INT_POS_INFINITY)// && g[i+y*m] != INT_POS_INFINITY)
    {
        return INT_POS_INFINITY;
    }
    return 1 + ((u*u)-(i*i)+g[u+y*m]*g[u+y*m]-(g[i+y*m]*g[i+y*m])) / (2*(u-i));
}

int DistanceTransform::f2D(int x, int i, int y, int z, Indexer3D &ind, int* g)
{
    if (g[ind.getIndex(i,y,z)]==INT_POS_INFINITY || x==INT_POS_INFINITY)
        return INT_POS_INFINITY;
    return pow((x-i),2) + pow(g[ind.getIndex(i,y,z)],2);
}

int DistanceTransform::SepPlusOne2D(int i, int u, int y, int z, Indexer3D &ind, int* g)
{
    if (g[ind.getIndex(u,y,z)] == INT_POS_INFINITY)// && g[i+y*m] != INT_POS_INFINITY)
    {
        return INT_POS_INFINITY;
    }
    return 1 + ((u*u)-(i*i)+pow(g[ind.getIndex(u,y,z)],2)-pow(g[ind.getIndex(i,y,z)],2)) / (2*(u-i));
}

int DistanceTransform::f3D(int x, int y, int z, int i, Indexer3D &ind, int* g)
{
    if (g[ind.getIndex(x,y,i)]==INT_POS_INFINITY || y==INT_POS_INFINITY)
        return INT_POS_INFINITY;
    return pow((z-i),2) + pow(g[ind.getIndex(x,y,i)],2);
}

int DistanceTransform::SepPlusOne3D(int x, int y, int i, int u, Indexer3D &ind, int* g)
{
    if (g[ind.getIndex(x,y,u)] == INT_POS_INFINITY)// && g[i+y*m] != INT_POS_INFINITY)
    {
        return INT_POS_INFINITY;
    }
    return 1 + ((u*u)-(i*i)+pow(g[ind.getIndex(x,y,u)],2)-pow(g[ind.getIndex(x,y,i)],2)) / (2*(u-i));
}

///my own///////////
void DistanceTransform::compute3DInverseDistanceMapNaive(const bool* src, int* out, int width, int height, int depth)
{
    Indexer3D ind(width,height);
    
    bool mark[width*height*depth];
    
    
    for (int x=0; x<width; x++)
    {
        for (int y=0; y<height; y++)
        {
            for (int z=0; z<depth; z++)
            {
                if (src[ind.getIndex(x,y,z)])
                {
                    out[ind.getIndex(x,y,z)]=0;
                }
                else
                {
                    for (int i=0; i<width*height*depth; i++)
                        mark[i]=true;
                    int bestDist=INT_POS_INFINITY;
                    QVector<point3D> stack;
                    point3D start(x,y,z);
                    stack.push_back(start);
                    mark[ind.getIndex(start)]=false;
                    while (!stack.empty())
                    {
                        point3D cur = stack.front();
                        stack.pop_front();
                        int curDist = pow(x-cur.x,2) + pow(y-cur.y,2) + pow(z-cur.z,2);
                        if (src[ind.getIndex(cur)] && curDist<bestDist)
                            bestDist=curDist;
                        
                        if (curDist<bestDist)
                        {
                            if (mark[ind.getIndex(cur.x-1,cur.y,cur.z)] && cur.x>0)
                            {
                                point3D p(cur.x-1,cur.y,cur.z);
                                stack.push_back(p);
                                mark[ind.getIndex(p)]=false;
                            }
                            
                            if (mark[ind.getIndex(cur.x+1,cur.y,cur.z)] && cur.x+1<width)
                            {
                                point3D p(cur.x+1,cur.y,cur.z);
                                stack.push_back(p);
                                mark[ind.getIndex(p)]=false;
                            }
                            
                            if (mark[ind.getIndex(cur.x,cur.y-1,cur.z)] && cur.y>0)
                            {
                                point3D p(cur.x,cur.y-1,cur.z);
                                stack.push_back(p);
                                mark[ind.getIndex(p)]=false;
                            }
                            
                            if (mark[ind.getIndex(cur.x,cur.y+1,cur.z)] && cur.y+1<height)
                            {
                                point3D p(cur.x,cur.y+1,cur.z);
                                stack.push_back(p);
                                mark[ind.getIndex(p)]=false;
                            }
                            
                            if (mark[ind.getIndex(cur.x,cur.y,cur.z-1)] && cur.z>0)
                            {
                                point3D p(cur.x,cur.y,cur.z-1);
                                stack.push_back(p);
                                mark[ind.getIndex(p)]=false;
                            }
                            
                            if (mark[ind.getIndex(cur.x,cur.y,cur.z+1)] && cur.z+1<depth)
                            {
                                point3D p(cur.x,cur.y,cur.z+1);
                                stack.push_back(p);
                                mark[ind.getIndex(p)]=false;
                            }
                        }
                    }
                    
                    out[ind.getIndex(x,y,z)]=bestDist;
                }
            }
        }
    }
    
    int newmax=0;
    double e = 15;
    double b = 30;
    double m = 444000;
    double a = INV_A;
    for (int i=0; i<width*height*depth; i++)
    {
        out[i] = pow(b-std::min(out[i]*(b/m),b),e)*a/(pow(b,e));
        if (out[i]>newmax)
            newmax=out[i];
    }
    
    QVector<QRgb> default_color_table;
    for (int i=0; i<255; i++)
    {
        default_color_table.append(qRgb(i,i,i));
    }
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(255,255,255));
//    }
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(255,255,255));
//    }
    for (int z=0; z<depth; z++)
    {
        QImage debug(width,height,QImage::Format_Indexed8);
        QImage debug2(width,height,QImage::Format_Indexed8);
        
        debug.setColorTable(default_color_table);
        debug2.setColorTable(default_color_table);
        for (int x=0; x<width; x++)
        {
            for (int y=0; y<debug.height(); y++)
            {
                debug.setPixel(x,y,(int)((out[ind.getIndex(x,y,z)]/((double)newmax))*254));
                if (src[ind.getIndex(x,y,z)])
                    debug2.setPixel(x,y,0);
                else
                    debug2.setPixel(x,y,254);
            }
            
        }
        QString debugfile = "./dist_3d/layer_";
        QString debugfile2 = "./output/layer_";
        QString num;
        num.setNum(z);
        debugfile.append(num);
        debugfile.append(".ppm");
        debugfile2.append(num);
        debugfile2.append(".ppm");
        debug.save(debugfile);
        debug2.save(debugfile2);
    }
}

///my own//////////
///test///////////////

void DistanceTransform::compute3DInverseDistanceMapTest(const bool* src, int* out, int width, int height, int depth)
{
    Indexer3D ind(width,height);
    int maxDist=0;
    int g[width*height*depth];
    
    
    
    //First pass
    for (int y=0; y<height; y++)
    {
        for (int x=0; x<width; x++)
        {
            if (src[ind.getIndex(x,y,0)])
            {
                g[ind.getIndex(x,y,0)]=0;
            }
            else
            {
                g[ind.getIndex(x,y,0)]=INT_POS_INFINITY;//width*src.height();
            }
            
            for (int z=1; z<depth; z++)
            {
                if (src[ind.getIndex(x,y,z)])
                {
                    g[ind.getIndex(x,y,z)]=0;
                }
                else
                {
                    if (g[ind.getIndex(x,y,z-1)] != INT_POS_INFINITY)
                        g[ind.getIndex(x,y,z)]=1+g[ind.getIndex(x,y,z-1)];
                    else
                        g[ind.getIndex(x,y,z-1)] = INT_POS_INFINITY;
                }
            }
            
            for (int z=depth-2; z>=0; z--)
            {
                if (g[ind.getIndex(x,y,z+1)]<g[ind.getIndex(x,y,z)])
                {
                    if (g[ind.getIndex(x,y+1,z)] != INT_POS_INFINITY)
                        g[ind.getIndex(x,y,z)]=1+g[ind.getIndex(x,y,z+1)];
                    else
                        g[ind.getIndex(x,y,z)] = INT_POS_INFINITY;
                }
            }
        }
    }
    
    //second pass, compute dist map for each depth
    int g2[width*height*depth];
    
    for (int y=0; y<height; y++)
    {
        int q;
        int s[width];
        int t[width];
        int w;
        for (int z=0; z<depth; z++)
        {
            q=0;
            s[0]=0;
            t[0]=0;
            for (int u=1; u<width;u++)
            {
                while (q>=0 && f2D(t[q],s[q],y,z,ind,g) > f2D(t[q],u,y,z,ind,g))
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
                    w = SepPlusOne2D(s[q],u,y,z,ind,g);
                    if (w<width)
                    {
                        q++;
                        s[q]=u;
                        t[q]=w;
                    }
                }
            }
            
            for (int u=width-1; u>=0; u--)
            {
               g2[ind.getIndex(u,y,z)]= f2D(u,s[q],y,z,ind,g);
//                if (out[ind.getIndex(u,y,z)] > maxDist)
//                    maxDist = out[ind.getIndex(u,y,z)];
                if (u==t[q])
                    q--;
            }
        }
    }
    
    //third pass
    for (int x=0; x<width; x++)
    {
        int q;
        int s[height];
        int t[height];
        int w;
        for (int z=0; z<depth; z++)
        {
            q=0;
            s[0]=0;
            t[0]=0;
            for (int u=1; u<height;u++)
            {
                while (q>=0 && f3DTest(x,t[q],s[q],z,ind,g2) > f3DTest(x,t[q],u,z,ind,g2))
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
                    w = SepPlusOne3DTest(x,s[q],u,z,ind,g2);
                    if (w<height)
                    {
                        q++;
                        s[q]=u;
                        t[q]=w;
                    }
                }
            }
            
            for (int u=height-1; u>=0; u--)
            {
                out[ind.getIndex(x,u,z)]= f3DTest(x,u,s[q],z,ind,g2);
                if (out[ind.getIndex(x,u,z)] > maxDist)
                    maxDist = out[ind.getIndex(x,u,z)];
                if (u==t[q])
                    q--;
            }
        }
    }
    printf("maxdist:%d\n",maxDist);
    
    int newmax=0;
    double e = 15;
    double b = 30;
    double m = 444000;
    double a = INV_A;
    for (int i=0; i<width*height*depth; i++)
    {
        out[i] = pow(b-std::min(out[i]*(b/m),b),e)*a/(pow(b,e));
        if (out[i]>newmax)
            newmax=out[i];
    }
    
    QVector<QRgb> default_color_table;
    for (int i=0; i<255; i++)
    {
        default_color_table.append(qRgb(i,i,i));
    }
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(255,255,255));
//    }
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(255,255,255));
//    }
    for (int z=0; z<depth; z++)
    {
        QImage debug(width,height,QImage::Format_Indexed8);
        QImage debug2(width,height,QImage::Format_Indexed8);
        
        debug.setColorTable(default_color_table);
        debug2.setColorTable(default_color_table);
        for (int x=0; x<width; x++)
        {
            for (int y=0; y<debug.height(); y++)
            {
                debug.setPixel(x,y,(int)((out[ind.getIndex(x,y,z)]/((double)newmax))*254));
                if (src[ind.getIndex(x,y,z)])
                    debug2.setPixel(x,y,0);
                else
                    debug2.setPixel(x,y,254);
            }
            
        }
        QString debugfile = "./dist_3d/layer_";
        QString debugfile2 = "./output/layer_";
        QString num;
        num.setNum(z);
        debugfile.append(num);
        debugfile.append(".ppm");
        debugfile2.append(num);
        debugfile2.append(".ppm");
        debug.save(debugfile);
        debug2.save(debugfile2);
    }
}

int DistanceTransform::f3DTest(int x, int y, int i, int z, Indexer3D &ind, int* g)
{
    if (g[ind.getIndex(x,i,z)]==INT_POS_INFINITY || y==INT_POS_INFINITY)
        return INT_POS_INFINITY;
    return pow((y-i),2) + pow(g[ind.getIndex(x,i,z)],2);
}

int DistanceTransform::SepPlusOne3DTest(int x, int i, int u, int z, Indexer3D &ind, int* g)
{
    if (g[ind.getIndex(x,u,z)] == INT_POS_INFINITY)// && g[i+y*m] != INT_POS_INFINITY)
    {
        return INT_POS_INFINITY;
    }
    return 1 + ((u*u)-(i*i)+pow(g[ind.getIndex(x,u,z)],2)-pow(g[ind.getIndex(x,i,z)],2)) / (2*(u-i));
}

///test//////////////

void DistanceTransform::computeKDInverseDistanceMap(const bool* in, int* out, int k, const int* dim)
{
    IndexerKD ind(k,dim);
    int pass[k];
    ComputeEDT3D(in,out,dim,ind,k,pass);
    
//    printf("maxdist:%d,    inf:%d\n",maxDist,INT_POS_INFINITY);
    int newmax=0;
//    int newmax=0;
//    double e = 15;
//    double b = 30;
//    double m = 1156;
//    double a = INV_A;
    int total=1;
    for (int i=0; i<k; i++)
    {
        total*=dim[i];
    }
    for (int i=0; i<total; i++)
    {
//        out[i] = pow(b-std::min(out[i]*(b/m),b),e)*a/(pow(b,e));
        if (out[i]>newmax)
            newmax=out[i];
    }
    printf("new maxdist:%d\n",newmax);
    
    QVector<QRgb> default_color_table;
    for (int i=0; i<255; i++)
    {
        default_color_table.append(qRgb(i,i,i));
    }
    default_color_table.append(qRgb(255,0,0));
    default_color_table.append(qRgb(0,255,0));
//    for (int z=0; z<dim[2]; z++)
    {
        QImage debug(dim[0],dim[1],QImage::Format_Indexed8);
        QImage debug2(dim[0],dim[1],QImage::Format_Indexed8);
        
        debug.setColorTable(default_color_table);
        debug2.setColorTable(default_color_table);
        for (int x=0; x<dim[0]; x++)
        {
            for (int y=0; y<dim[1]; y++)
            {
                int pass[3];
                pass[0]=x; pass[1]=y; //pass[2]=z;
                int v = (int)((out[ind.getIndex(pass)]/((double)newmax))*254);
                if (out[ind.getIndex(pass)]==INT_POS_INFINITY)
                {
                    v=255;
                }
                else if (v >255)
                {
//                    printf ("error:%d\n",out[ind.getIndex(pass)]);
                    v=255;
                }
                if (v <0)
                {
                    v=256;
                    printf ("error:%d\n",out[ind.getIndex(pass)]);
                }
                debug.setPixel(x,y,v);
                if (in[ind.getIndex(pass)])
                    debug2.setPixel(x,y,0);
                else
                    debug2.setPixel(x,y,254);
            }
            
        }
        QString debugfile = "./dist_3d/layer_";
        QString debugfile2 = "./output/layer_";
        QString num;
//        num.setNum(z);
        debugfile.append(num);
        debugfile.append(".ppm");
        debugfile2.append(num);
        debugfile2.append(".ppm");
        debug.save(debugfile);
        debug2.save(debugfile2);
    }
}
//<http://www.csd.uwo.ca/~olga/Courses/Fall2009/9840/Chosen/linearExactEucl.pdf>
int DistanceTransform::ComputeEDT(const bool* I, int* D, int k, const int* n, const IndexerKD &ind, int d, int* i)
{
    int maxDist=0;
    if (d==1)
    {
        for (i[0]=0; i[0]<n[0]; i[0]++)
        {
            if (I[ind.getIndex(i)])
                D[ind.getIndex(i)]=0;
            else
                D[ind.getIndex(i)]=INT_POS_INFINITY;
        }
    }
    else
    {
        for (i[d-1]=0; i[d-1]<n[d-1]; i[d-1]++)
        {
            int pass[k];
            copyArray(i,pass,k);
            ComputeEDT(I,D,k,n,ind,d-1,pass);
        }
    }
    
    int temp = recursiveFor(I,D,k,n,ind,d,i,0);
    if (temp>maxDist)
        maxDist=temp;
    
    return maxDist;
}

int DistanceTransform::recursiveFor(const bool* I, int* D, int k, const int* n, const IndexerKD &ind, int d, int* i, int level)
{
    int maxDist=0;
    for (i[level]=0; i[level]<n[level]; i[level]++)
    {
        if (level<d-2)
        {
            int temp = recursiveFor(I,D,k,n,ind,d,i,level+1);
            if (temp>maxDist)
                maxDist=temp;
        }
        else
        {
            int pass[k];
            copyArray(i,pass,k);
            int temp = VoronoiEDT(I,D,k,n,ind,d,pass);
            if (temp>maxDist)
                maxDist=temp;
        }
    }
    return maxDist;
}

int DistanceTransform::VoronoiEDT(const bool* I, int* D, int k, const int* n, const IndexerKD &ind, int d, int* j)
{
    int maxDist=0;
    int l=0;
    int f[n[d-1]];
    int g[2*n[d-1]];
    int h[2*n[d-1]];
    int X[n[d-1]][k];
    
    for(int i=0; i<n[d-1]; i++)
    {
        copyArray(j,X[i],k);
        X[i][d-1]=i;//no +1, this is an index
        f[i]=D[ind.getIndex(X[i])];
        if (f[i]!=INT_POS_INFINITY)
        {
            if (l<2)
            {
                l++;
                g[l]=f[i];
                h[l]=i+1;
            }
            else
            {
                while (l>=2 && RemoveEDT(g[l-1],g[l],f[i],h[l-1],h[l],i+1))
                {
                    l--;
                }
                l++;
                g[l]=f[i];
                h[l]=i+1;
            }
        }
    }
    
    int n_s=l;
    if (n_s==0)
        return maxDist;
    
    l=1;
    for (int i=1; i<=n[d-1]; i++)
    {
        while (l<n_s && g[l]+pow(h[l]-i,2) > g[l+1]+pow(h[l+1]-i,2))
            l++;
        D[ind.getIndex(X[i-1])]=g[l]+(int)pow(h[l]-i,2);
        if (D[ind.getIndex(X[i-1])]>maxDist)
            maxDist=D[ind.getIndex(X[i-1])];
    }
    return maxDist;
}

bool DistanceTransform::RemoveEDT(int dis_sqr_u_Rd, int dis_sqr_v_Rd, int dis_sqr_w_Rd, int u_d, int v_d, int w_d)
{
    int a = v_d-u_d;
    int b = w_d-v_d;
    int c = w_d-u_d;
    return c*dis_sqr_v_Rd-b*dis_sqr_u_Rd-a*dis_sqr_w_Rd-a*b*c > 0;
}

void DistanceTransform::copyArray(int* from, int* to, int c)
{
    for (int i=0; i<c; i++)
        to[i]=from[i];
}

void DistanceTransform::ComputeEDT3D(const bool* I, int* D, const int* n, const IndexerKD &ind, int d, int* i)
{
    if (d==1)
    {
        for (i[0]=0; i[0]<n[0]; i[0]++)
        {
            if (I[ind.getIndex(i)])
                D[ind.getIndex(i)]=0;
            else
                D[ind.getIndex(i)]=INT_POS_INFINITY;
        }
    }
    else
    {
        for (i[d-1]=0; i[d-1]<n[d-1]; i[d-1]++)
        {
            int pass[3];
            copyArray(i,pass,3);
            ComputeEDT3D(I,D,n,ind,d-1,pass);
        }
    }
    
    for (i[0]=0; i[0]<n[0]; i[0]++)
    {
        for (i[1]=0; i[1]<n[1]; i[1]++)
        {
            int pass[3];
            copyArray(i,pass,3);
            VoronoiEDT3D(I,D,n,ind,d,pass);
        }
    }
}

void DistanceTransform::VoronoiEDT3D(const bool* I, int* D, const int* n, const IndexerKD &ind, int d, int* j)
{
    int l=0;
    int f[n[d-1]];
    int g[2*n[d-1]];
    int h[2*n[d-1]];
    int X[n[d-1]][3];
    
    for(int i=0; i<n[d-1]; i++)
    {
        copyArray(j,X[i],3);
        X[i][d-1]=i;//no +1, this is an index
        f[i]=D[ind.getIndex(X[i])];
        if (f[i]!=INT_POS_INFINITY)
        {
            if (l<2)
            {
                l++;
                g[l]=f[i];
                h[l]=i+1;
            }
            else
            {
                while (l>=2 && RemoveEDT(g[l-1],g[l],f[i],h[l-1],h[l],i+1))
                {
                    l--;
                }
                l++;
                g[l]=f[i];
                h[l]=i+1;
            }
        }
    }
    
    int n_s=l;
    if (n_s==0)
        return;
    
    l=1;
    for (int i=1; i<=n[d-1]; i++)
    {
        while (l<n_s && g[l]+pow(h[l]-i,2) > g[l+1]+pow(h[l+1]-i,2))
            l++;
        D[ind.getIndex(X[i-1])]=g[l]+(int)pow(h[l]-i,2);
    }
}


////NEW////
////////// Functions F and Sep for the SDT labelling
/** 
 **************************************************
 * @b F
 * @param x 
 * @param i 
 * @param gi2 
 * @return Definition of a parabola
 **************************************************/
long F(int x, int i, long gi2)
{
  return sum((x-i)*(x-i), gi2);
}

/** 
 **************************************************
 * @b Sep
 * @param i 
 * @param u 
 * @param gi2 
 * @param gu2 
 * @return The abscissa of the intersection point between two parabolas
 **************************************************/
long Sep(int i, int u, long gi2, long gu2) {
  return intdivint(sum( sum((long) (u*u - i*i),gu2), opp(gi2) ), 2*(u-i));
}
/////////
/** 
 **************************************************
 * @b phaseSaitoX
 * @param V Input volume
 * @param sdt_x SDT along the x-direction
 **************************************************/
//First step of  the saito  algorithm 
// (Warning   : we  store the  EDT instead of the SDT)
void phaseSaitoX(const double* V, long *sdt_x, const Indexer3D &ind, int sizeX, int sizeY, int sizeZ)
{
  for (int z = 0; z < sizeZ ; z++) 	    
    for (int y = 0; y < sizeY ; y++) 
      {
	if (V[ind.getIndex(0,y,z)]>0) 
    {
	    sdt_x[ind.getIndex(0,y,z)] = 10 - V[ind.getIndex(0,y,z)]*10;
    }
	else 	    
	  sdt_x[ind.getIndex(0,y,z)]=INFTY;  
	  
	// Forward scan
	for (int x = 1; x < sizeX ; x++) 	    
	  if (V[ind.getIndex(x,y,z)]>0 && 10 - V[ind.getIndex(x,y,z)]*10 < sum(1, sdt_x[ind.getIndex(x-1,y,z)]))
      {
	    sdt_x[ind.getIndex(x,y,z)] = 10 - V[ind.getIndex(x,y,z)]*10;
      }
	  else 
	    sdt_x[ind.getIndex(x,y,z)]=sum(1, sdt_x[ind.getIndex(x-1,y,z)]);	  
	
	//Backward scan
	for (int x = sizeX -2; x >= 0; x--)      
	  if (sdt_x[ind.getIndex(x+1,y,z)] < sdt_x[ind.getIndex(x,y,z)]) 
	    sdt_x[ind.getIndex(x,y,z)]=sum(1, sdt_x[ind.getIndex(x+1,y,z)]);
      }
}

/** 
 **************************************************
 * @b phaseSaitoY
 * @param sdt_x the SDT along the x-direction
 * @param sdt_xy the SDT in the xy-slices
 **************************************************/
//Second      Step   of    the       saito   algorithm    using    the
//[Meijster/Roerdnik/Hesselink] optimization
void phaseSaitoY(const long *sdt_x, long *sdt_xy, const Indexer3D &ind, int sizeX, int sizeY, int sizeZ)
{
  
  int s[sizeY]; //Center of the upper envelope parabolas
  int t[sizeY]; //Separating index between 2 upper envelope parabolas 
  int q; 
  int w;

  for ( int z = 0; z<sizeZ; z++) 	    
    for ( int x = 0; x < sizeX; x++) 
      {
	q=0;
	s[0] = 0;
	t[0] = 0;
	
	//Forward Scan
	for (int u=1; u < sizeY ; u++) 
	  {
	    while ((q >= 0) &&
		   (F(t[q],s[q],prod(sdt_x[ind.getIndex(x,s[q],z)],sdt_x[ind.getIndex(x,s[q],z)])) > 
		    F(t[q],u,prod(sdt_x[ind.getIndex(x,u,z)],sdt_x[ind.getIndex(x,u,z)])))
		   ) 
	      q--;
	    
	    if (q<0) 
	    {
            q=0;
            s[0]=u;
	    }
	    else 
	      {
		w = 1 + Sep(s[q],
			    u,
			    prod(sdt_x[ind.getIndex(x,s[q],z)],sdt_x[ind.getIndex(x,s[q],z)]),
			    prod(sdt_x[ind.getIndex(x,u,z)],sdt_x[ind.getIndex(x,u,z)]));
	
		if (w < sizeY) 
		  {
		    q++;
		    s[q]=u;
		    t[q]=w;
		  }
	      }
	  }

	//Backward Scan
	for (int u = sizeY-1; u >= 0; --u) 
	  {
	    sdt_xy[ind.getIndex(x,u,z)] = F(u,s[q],prod(sdt_x[ind.getIndex(x,s[q],z)],sdt_x[ind.getIndex(x,s[q],z)]));	      
	    if (u==t[q]) 
	      q--;
	  }
      }
}

/** 
 **************************************************
 * @b phaseSaitoZ
 * @param sdt_xy the SDT in the xy-slices
 * @param sdt_xyz the final SDT
 **************************************************/
//Third   Step      of     the    saito   algorithm     using      the
//[Meijster/Roerdnik/Hesselink] optimization
//Modified by Brian to fold the z dimension
void phaseSaitoZ(const long *sdt_xy, long *sdt_xyz, const Indexer3D &ind, int sizeX, int sizeY, int sizeZ)
{
  
  int s[sizeZ*2]; //Center of the upper envelope parabolas
  int t[sizeZ*2]; //Separating index between 2 upper envelope parabolas 
  int q; 
  int w;
    
//  long max=0;
  
  for ( int y = 0; y<sizeY; y++) 	    
    for ( int x = 0; x < sizeX; x++) 
      {
	q=0;
	s[0] = 0;
	t[0] = 0;
	
	//Forward Scan
	for (int u=1; u < sizeZ*2 ; u++) 
    {
	    while ((q >= 0) &&
               (F(t[q],s[q], sdt_xy[ind.getIndex(x,y,mod(s[q],sizeZ))]) > 
                F(t[q],u,sdt_xy[ind.getIndex(x,y,mod(u,sizeZ))]))
               ) 
	      q--;
	    
	    if (q<0) 
	    {
            q=0;
            s[0]=u;
	    }
	    else 
	    {
            w = 1 + Sep(s[q],
                    u,
                    sdt_xy[ind.getIndex(x,y,mod(s[q],sizeZ))],
                    sdt_xy[ind.getIndex(x,y,mod(u,sizeZ))]);
        
            if (w < sizeZ) 
            {
                q++;
                s[q]=u;
                t[q]=w;
            }
	    }
	}
    
    
    
	//Backward Scan
	for (int u = (sizeZ*2)-1; u >= 0; --u) 
	  {
        if (u>=sizeZ || F(u,s[q],sdt_xy[ind.getIndex(x,y,mod(s[q],sizeZ))]) < sdt_xyz[ind.getIndex(x,y,mod(u,sizeZ))])
        {
            sdt_xyz[ind.getIndex(x,y,mod(u,sizeZ))] = F(u,s[q],sdt_xy[ind.getIndex(x,y,mod(s[q],sizeZ))]);	      
        }
	    if (u==t[q]) 
	      q--;
        
//        if (sdt_xyz[ind.getIndex(x,y,mod(u,sizeZ))] > max)
//            max=sdt_xyz[ind.getIndex(x,y,mod(u,sizeZ))];
	  }
    }
//  printf("3d max=%d\n",max);
}

//<http://tc18.liris.cnrs.fr/code_data_set/Code/SEDT/>
void DistanceTransform::compute3DInverseDistanceMapNew(const double* src, long* out, int width, int height, int depth, const BPixelCollection &img)
{
    Indexer3D ind(width,height);
    long* sdt_x = new long[width*height*depth];
    long* sdt_xy = new long[width*height*depth];
    phaseSaitoX(src,sdt_x,ind,width,height,depth);
    phaseSaitoY(sdt_x,sdt_xy,ind,width,height,depth);
    phaseSaitoZ(sdt_xy,out,ind,width,height,depth);
    
    delete[] sdt_x;
    delete[] sdt_xy;
    
    
    long newmax=0;
    long newmin=INT_POS_INFINITY;
//    double e = 100;//60;
//    double b = 200;
//    double m = 10000;
//    double a = INV_A;
//    for (int i=0; i<width*height*depth; i++)
//    {
//        out[i] = pow(b-std::min(out[i]*(b/m),b),e)*a/(pow(b,e)) + 1;
//        if (out[i]>newmax)
//            newmax=out[i];
//    }
    
    double e = 350;
    double b = 3;
    double m = 10000;
    double a = INV_A;
    for (int i=0; i<width*height*depth; i++)
    {
        //       pow(b-std::min(out[index]*(b/m),b),e)*a/(pow(b,e)) + 1 + std::min(cc_size,max_cc_size);
        out[i] = pow(b,e*std::max(std::min(1-out[i]*(1/m),1.0),0.0))*a/(pow(b,e)) + 1;
        if (out[i]>newmax)
            newmax=out[i];
        if (out[i]<newmin)
            newmin=out[i];
    }
    
    
    ///debug from hereon down
//    printf("3d newmin=%d  newmax=%d\n",newmin,newmax);
#if SHOW_VIZ_DIST
    //visulazation
    cv::Mat cloud(1,width*height*depth, CV_32FC3);
    cv::Point3f* anglePoints = cloud.ptr<cv::Point3f>();
    cv::Mat color_cloud(1,width*height*depth, CV_8UC3);
    
    QVector<QRgb> default_color_table;
    for (int i=0; i<255; i++)
    {
        default_color_table.append(qRgb(i,i,i));
    }
    for (int z=0; z<depth; z++)
    {
        QImage debug(width,height,QImage::Format_Indexed8);
        QImage debug2(width,height,QImage::Format_Indexed8);
        
        
        
        debug.setColorTable(default_color_table);
        debug2.setColorTable(default_color_table);
        for (int x=0; x<width; x++)
        {
            for (int y=0; y<debug.height(); y++)
            {
                if (img.pixel(x,y))
                {
                    debug.setPixel(x,y,(int)((out[ind.getIndex(x,y,z)]/((double)newmax))*254));
                    debug2.setPixel(x,y,src[ind.getIndex(x,y,z)]*254);
                    
                    //                if (src[ind.getIndex(x,y,z)]>0)
                    //if ((int)((out[ind.getIndex(x,y,z)]/((double)newmax))*254) >25)
                    {
                        anglePoints[x+width*y+width*height*z].x=(float)x;
                        anglePoints[x+width*y+width*height*z].y=(float)y;
                        anglePoints[x+width*y+width*height*z].z=(float)z;
                        //                    printf("Write color %f\n",src[ind.getIndex(x,y,z)]*254);
                    }
                    
                    
                    color_cloud.at<cv::Vec3b>(0,x+width*y+width*height*z)[0]=(int)((out[ind.getIndex(x,y,z)]/((double)newmax))*254);
                    color_cloud.at<cv::Vec3b>(0,x+width*y+width*height*z)[1]=(int)((out[ind.getIndex(x,y,z)]/((double)newmax))*254);
                    color_cloud.at<cv::Vec3b>(0,x+width*y+width*height*z)[2]=(int)((out[ind.getIndex(x,y,z)]/((double)newmax))*254);
                }
            }
            
        }
//        QString debugfile = "./dist_3d/layer_";
//        QString debugfile2 = "./angleimage/layer_";
//        QString num;
//        num.setNum(z);
//        debugfile.append(num);
//        debugfile.append(".ppm");
//        debugfile2.append(num);
//        debugfile2.append(".ppm");
//        debug.save(debugfile);
//        debug2.save(debugfile2);
    }
    cv::viz::Viz3d window("distMap");
    
    cv::viz::WCoordinateSystem axis(20.0);
    window.showWidget("axis",axis);
    cv::viz::WCloud cloudImage(cloud,color_cloud);
    window.showWidget("cloud",cloudImage);
    
    
    
    window.spin();
#endif
}
