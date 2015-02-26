#include "imageaverager.h"
#include <stdio.h>

ImageAverager::ImageAverager()
{
}

ImageAverager::ImageAverager(int width, int height)
{
    final_width=width;
    final_height=height;
    for (int i=0; i<256; i++)
    {
        default_color_table.append(qRgb(i,i,i));
    }
}

//Averages the images in dir, assumes the images are named with indexing numbers
QImage ImageAverager::averageImages(QString dir, QVector<int> picIds)
{
    QImage avg(final_width,final_height,QImage::Format_Indexed8);
    avg.setColorTable(default_color_table);
    
    QVector<QVector<int> > totals;
    for (int i=0; i<final_width; i++)
    {
        QVector<int> nextRow(final_height);
        totals.append(nextRow);
        for (int j=0; j<final_height; j++)
        {
            totals[i][j]=0;
        }
    }
    
    foreach(int index, picIds)
    {
        QString qFileName;
        qFileName.sprintf("%03d.pgm", index);
        QString path = dir;
        path.append(qFileName);
        QImage toAdd(path);
        std::string ppp = qPrintable(path);
        printf("(%s)\n",ppp.c_str());
        
        int x_offset = final_width/2 - toAdd.width()/2;
        
        for (int i=0; i<toAdd.width(); i++)
        {
            for (int j=0; j<toAdd.height(); j++)
            {
                totals[i+x_offset][j] += 255 - qGray(toAdd.pixel(i,j));
                
            }
        }
    }
    for (int j=0; j<final_height; j++)
    {
        for (int i=0; i<final_width; i++)
        {
            
            avg.setPixel(i,j,255 - totals[i][j]/picIds.size());
        }
    }
    return avg;
}

//Creates probabilty map based on an averaged image
QVector<QVector<double> > ImageAverager::produceProbabilityMap(const QImage &src)
{
    QVector<QVector<double> > ret(src.width());
    for (int i=0; i<src.width(); i++)
    {
        for (int j=0; j<src.height(); j++)
        {
            double val = (255-qGray(src.pixel(i,j)))/255.0;
            double leftVal = val;
            double rightVal = val;
            double aboveVal = val;
            double belowVal = val;
            
            if (i>0)
                leftVal = (255-qGray(src.pixel(i-1,j)))/255.0;
            
            if (i<src.width()-1)
                rightVal = (255-qGray(src.pixel(i+1,j)))/255.0;
            
            if (j>0)
                aboveVal = (255-qGray(src.pixel(i,j-1)))/255.0;
            
            if (j<src.height()-1)
                belowVal = (255-qGray(src.pixel(i,j+1)))/255.0;
            
            ret[i].append(.5*val + .125*(leftVal+rightVal+belowVal+aboveVal));
        }
    }
    
    ///test///
//    QImage test = src.copy(0,0,src.width(),src.height());
//    for (int i=0; i<src.width(); i++)
//    {
//        for (int j=0; j<src.height(); j++)
//        {
//            test.setPixel(i,j,255*ret[i][j]);
//        }
//    }
//    test.save("./probability_map.ppm");
    return ret;
}
