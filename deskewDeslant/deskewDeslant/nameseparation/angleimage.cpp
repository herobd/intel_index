#include "angleimage.h"

AngleImage::AngleImage(const BPixelCollection* ofImage, int numOfBins, double minVal, double maxVal)
{
    src=ofImage;
    this->numOfBins = numOfBins;
    minValue=minVal;
    maxValue=maxVal;
    
//    angles = new QMap<double, double>*[src->width()];
    binStr = new QMap<int, double>*[src->width()];
    for (int x=0; x<src->width(); x++)
    {
//        angles[x] = new QMap<double, double>[src->height()];
        binStr[x] = new QMap<int, double>[src->height()];
    }
    
    //Do it all yourself
    init2();
    
}

AngleImage::~AngleImage()
{
    for (int i=0; i<src->width(); i++)
    {
//        delete[] angles[i];
        delete[] binStr[i];
    }
//    delete[] angles;
    delete[] binStr;
}

void AngleImage::init()
{
    QVector<QPoint> refPoints;
    QVector<double > refSlopesM;
    QVector<double > refLengthsM;
    
//   //readfile 
//    QVector<tracePoint> tracePoints;
    
//    std::ofstream myfile ("matrix.txt");
//    if (myfile.is_open())
//    {
//        myfile << width() << " ";
//        myfile << height();
//        myfile << "\n";
//        for (int i =0; i<width(); i++)
//        {
//            for (int j = 0; j<height(); j++) {
//                myfile << pixel(i,j) << " ";
//            }
//            myfile << "\n";
//        }
//        myfile.close();
//    }
//    else printf("Unable to open file\n");
////2.2
//    system("java -Djava.io.tmpdir=/home/brian/tmp/ -jar ~/intel_index/nameseparation/ScottsCode/slopeGen.jar matrix slopedata 2.2 4");
    
//    std::ifstream infile("slopedata.txt");
//    std::string line;
//    getline(infile, line);
//    QRegExp rei("(\\d+)(?:[^\\d]+)(\\d+)(?:[^\\d]+)(\\d+)");
//    QString qLine(line.c_str());
//    rei.indexIn(qLine);
//    tracePoint init;
//    init.x=rei.cap(2).toInt();
//    init.y=rei.cap(3).toInt();
//    tracePoints.append(init);
    
//    //(id)(x)(y)...
//    QRegExp re("(\\d+)(?:[^\\d]+)(\\d+)(?:[^\\d]+)(\\d+)+((?:[^\\d]+)(\\d+))");
//    while (getline(infile, line))
//    {
//        QString qLine(line.c_str());
//        re.indexIn(qLine);
//        tracePoint nextPoint;
//        int index=re.cap(1).toInt()-1;
////        printf("read index %d\n",index);
//        if (index >= tracePoints.size())
//        {
//            nextPoint.x=re.cap(2).toInt()-1;
//            nextPoint.y=re.cap(3).toInt()-1;
//            for (int i=4; i<re.captureCount(); i++)
//            {
//                int connectionId = re.cap(i).toInt()-1;
//                double angle = atan2((nextPoint.y-tracePoints[connectionId].y),(nextPoint.x-tracePoints[connectionId].x));
//    //            double angle = re.cap(6).toDouble();
//                if (angle < 0)
//                    angle += PI;
//                double distance = sqrt(pow(nextPoint.x-tracePoints[connectionId].x,2) + pow(nextPoint.y-tracePoints[connectionId].y,2));
//                nextPoint.connectedPoints.append(connectionId);
//                nextPoint.angleBetween.append(angle);
//                nextPoint.distanceBetween.append(distance);
//                tracePoints.append(nextPoint);
                
//                tracePoints[connectionId].connectedPoints.append(index);
//                tracePoints[connectionId].angleBetween.append(angle);
//                tracePoints[connectionId].distanceBetween.append(distance);
//            }
            
//        }
//        else
//        {
//            printf("ERROR repeat index %d read in\n",index);
////            int last = re.cap(4).toInt();
////            double angle = re.cap(6).toDouble();
////            if (angle < 0)
////                angle += 180;
////            tracePoints[index].connectedPoints.append(last);
////            tracePoints[index].angleBetween.append(angle);
////            tracePoints[last].connectedPoints.append(index);
////            tracePoints[last].angleBetween.append(angle);
//        }
//    }
    
    
//    QImage hand("/home/brian/test/marked_overlappingletters21.ppm");
//    skeleton.initHand(src,hand);
    skeleton.init(src);
    
    QVector<bool> visited(skeleton.numberOfVertices());
    
    //debug
//    if (visited.size()==0)
//        src->makeImage().save("./ttt.ppm");
    
    for (int i=0; i<visited.size(); i++)
    {
        visited[i]=false;
    }
    QVector<int> pointStack;
    pointStack.append(0);
    while (!pointStack.empty())
    {
        int curIndex=pointStack.back();
        pointStack.pop_back();
        if (visited[curIndex])
            continue;
        visited[curIndex]=true;
        
//        QPoint toAdd(tracePoints[curIndex].x,tracePoints[curIndex].y);
        
//        QVector<double> slope;
        for (int i=0; i<skeleton[curIndex].connectedPoints().size(); i++)
        {
//            slope.append(tracePoints[curIndex].angleBetween[i]);
            if (!visited[skeleton[curIndex].connectedPoints()[i]])
            {
                unsigned int curRegionId = skeleton[curIndex].connectedPoints()[i];
                pointStack.append(curRegionId);
                int midX = (skeleton[curIndex].x + skeleton[curRegionId].x)/2;
                int midY = (skeleton[curIndex].y + skeleton[curRegionId].y)/2;
                QPoint mid(midX,midY);
                if (!pixel(midX,midY))
                {
                    //find closest
                    mid = findClosestPoint(mid);
                }
                
                if(mid.x()>2000 || mid.y()>2000 || mid.x()<0)
                {
                    printf("Error on midpoint for (%d,%d) and (%d,%d)\n",skeleton[curIndex].x,skeleton[curIndex].y,skeleton[curRegionId].x,skeleton[curRegionId].y);
                    continue;
                }
                
                refPoints.append(mid);
                double slopeMid=skeleton[curIndex].angleBetween(curRegionId);
//                slopeMid.append(tracePoints[curIndex].angleBetween[i]);
                refSlopesM.append(slopeMid);
                refLengthsM.append(skeleton[curIndex].distanceBetween(curRegionId));
            }
        }
//        refPoints.append(toAdd);
//        refSlopesM.append(slope);
        
        
    }
    
    QVector<QPoint> edgeStack;
    QVector<double> angleEdgeStack;
    QVector<double> distanceEdgeStack;
    
    double FILL_RADIOUS_CONST = 0.75;
    for (int i=0; i<refPoints.size(); i++)
    {
        QVector<QPoint> workingStack;
        QVector<double> distStack;
        workingStack.append(refPoints[i]);
        distStack.append(0);
        double angle = refSlopesM[i];
        setPixelSlope(refPoints[i].x(),refPoints[i].y(),angle,1);
        
        double fillRadious = FILL_RADIOUS_CONST * refLengthsM[i];
        
        while(!workingStack.empty())
        {
            QPoint cur = workingStack.front();
            workingStack.pop_front();
            double curDist = distStack.front();
            distStack.pop_front();
            
            if (curDist > fillRadious)
            {
                edgeStack.append(cur);
                angleEdgeStack.append(angle);
                distanceEdgeStack.append(curDist);
            }
            else
            {
                double newDist = curDist+1;
                double str = (newDist+1)/(2*newDist);
                if (cur.x()<width()-1 && noStrongerAngleForPixel(cur.x()+1,cur.y(),angle,str))
                {
                    QPoint pp(cur.x()+1,cur.y());
                    workingStack.push_back(pp);
                    
                    distStack.push_back(newDist);
                    
                    setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.y()<height()-1 && noStrongerAngleForPixel(cur.x(),cur.y()+1,angle,str))
                {
                    QPoint pp(cur.x(),cur.y()+1);
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.x()>0 && noStrongerAngleForPixel(cur.x()-1,cur.y(),angle,str))
                {
                    QPoint pp(cur.x()-1,cur.y());
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.y()>0 && noStrongerAngleForPixel(cur.x(),cur.y()-1,angle,str))
                {
                    QPoint pp(cur.x(),cur.y()-1);
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                //diagonals
                newDist = curDist+SQRT_2;
                str = (newDist+1)/(2*newDist);
                if (cur.x()<width()-1 && cur.y()<height()-1 && noStrongerAngleForPixel(cur.x()+1,cur.y()+1,angle,str))
                {
                    QPoint pp(cur.x()+1,cur.y()+1);
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.y()<height()-1 && cur.x()>0 && noStrongerAngleForPixel(cur.x()-1,cur.y()+1,angle,str))
                {
                    QPoint pp(cur.x()-1,cur.y()+1);
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.x()<width()-1 && cur.y()>0 && noStrongerAngleForPixel(cur.x()+1,cur.y()-1,angle,str))
                {
                    QPoint pp(cur.x()+1,cur.y()-1);
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.y()>0 && cur.x()>0 && noStrongerAngleForPixel(cur.x()-1,cur.y()-1,angle,str))
                {
                    QPoint pp(cur.x()-1,cur.y()-1);
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    setPixelSlope(pp.x(),pp.y(),angle,str);
                }
            }
        }
    }
    
    
    //after fill
    while (!edgeStack.empty())
    {
        QPoint cur = edgeStack.front();
        edgeStack.pop_front();
        double curDist = distanceEdgeStack.front();
        distanceEdgeStack.pop_front();
        double angle = angleEdgeStack.front();
        angleEdgeStack.pop_front();
        

        if (cur.x()<width()-1 && noAnglesForPixel(cur.x()+1,cur.y()))
        {
            QPoint pp(cur.x()+1,cur.y());
            edgeStack.push_back(pp);
            double newDist = curDist+1;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.y()<height()-1 && noAnglesForPixel(cur.x(),cur.y()+1))
        {
            QPoint pp(cur.x(),cur.y()+1);
            edgeStack.push_back(pp);
            double newDist = curDist+1;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.x()>0 && noAnglesForPixel(cur.x()-1,cur.y()))
        {
            QPoint pp(cur.x()-1,cur.y());
            edgeStack.push_back(pp);
            double newDist = curDist+1;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.y()>0 && noAnglesForPixel(cur.x(),cur.y()-1))
        {
            QPoint pp(cur.x(),cur.y()-1);
            edgeStack.push_back(pp);
            double newDist = curDist+1;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        //diagonals
        if (cur.x()<width()-1 && cur.y()<height()-1 && noAnglesForPixel(cur.x()+1,cur.y()+1))
        {
            QPoint pp(cur.x()+1,cur.y()+1);
            edgeStack.push_back(pp);
            double newDist = curDist+SQRT_2;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.y()<height()-1 && cur.x()>0 && noAnglesForPixel(cur.x()-1,cur.y()+1))
        {
            QPoint pp(cur.x()-1,cur.y()+1);
            edgeStack.push_back(pp);
            double newDist = curDist+SQRT_2;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.x()<width()-1 && cur.y()>0 && noAnglesForPixel(cur.x()+1,cur.y()-1))
        {
            QPoint pp(cur.x()+1,cur.y()-1);
            edgeStack.push_back(pp);
            double newDist = curDist+SQRT_2;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.y()>0 && cur.x()>0 && noAnglesForPixel(cur.x()-1,cur.y()-1))
        {
            QPoint pp(cur.x()-1,cur.y()-1);
            edgeStack.push_back(pp);
            double newDist = curDist+SQRT_2;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        
    }
}

void AngleImage::init2()//has smarter filling algorithm (doesnt do mid points, but whole line)
{
    QVector<QPoint> refPoints;
    QVector<QPoint> refPointsS;
    QVector<QPoint> refPointsE;
    QVector<double > refSlopesM;
    QVector<double > refLengthsM;
    
    
    
//    QImage hand("/home/brian/test/marked_overlappingletters21.ppm");
//    skeleton.initHand(src,hand);
    skeleton.init(src);
    
    if (skeleton.numberOfVertices()==0)
        return;
    
    QVector<bool> visited(skeleton.numberOfVertices());
    
    //debug
//    if (visited.size()==0)
//        src->makeImage().save("./ttt.ppm");
    
    for (int i=0; i<visited.size(); i++)
    {
        visited[i]=false;
    }
    QVector<int> pointStack;
    pointStack.append(0);
    while (!pointStack.empty())
    {
        int curIndex=pointStack.back();
        pointStack.pop_back();
        if (visited[curIndex])
            continue;
        visited[curIndex]=true;
        
//        QPoint toAdd(tracePoints[curIndex].x,tracePoints[curIndex].y);
        
//        QVector<double> slope;
        for (int i=0; i<skeleton[curIndex].connectedPoints().size(); i++)
        {
//            slope.append(tracePoints[curIndex].angleBetween[i]);
            if (!visited[skeleton[curIndex].connectedPoints()[i]])
            {
                unsigned int curRegionId = skeleton[curIndex].connectedPoints()[i];
                pointStack.append(curRegionId);
                int midX = (skeleton[curIndex].x + skeleton[curRegionId].x)/2;
                int midY = (skeleton[curIndex].y + skeleton[curRegionId].y)/2;
                QPoint mid(midX,midY);
                QPoint start(skeleton[curIndex].x,skeleton[curIndex].y);
                QPoint end(skeleton[curRegionId].x,skeleton[curRegionId].y);
                if (!pixel(midX,midY))
                {
                    //find closest
                    mid = findClosestPoint(mid);
                }
                
                if(mid.x()>2000 || mid.y()>2000 || mid.x()<0)
                {
                    printf("Error on midpoint for (%d,%d) and (%d,%d)\n",skeleton[curIndex].x,skeleton[curIndex].y,skeleton[curRegionId].x,skeleton[curRegionId].y);
                    continue;
                }
                
                refPoints.append(mid);
                refPointsS.append(start);
                refPointsE.append(end);
                double slopeMid=skeleton[curIndex].angleBetween(curRegionId);
//                slopeMid.append(tracePoints[curIndex].angleBetween[i]);
                refSlopesM.append(slopeMid);
                refLengthsM.append(skeleton[curIndex].distanceBetween(curRegionId));
            }
        }
//        refPoints.append(toAdd);
//        refSlopesM.append(slope);
        
        
    }
    
    QVector<QPoint> edgeStack;
    QVector<double> angleEdgeStack;
    QVector<double> distanceEdgeStack;
    
    double HARD_FILL_RADIOUS_CONST = 1;
    for (int i=0; i<refPoints.size(); i++)
    {
        QVector<QPoint> workingStack;
        QVector<double> distStack;
        double angle = refSlopesM[i];
        QPoint mid=  refPoints[i];
        QPoint start=  refPointsS[i];
        QPoint end=  refPointsE[i];
        double slope = (end.y()*1.0-start.y())/(end.x()-start.x());
        if (fabs(slope) > 1)
        {
            slope = 1/slope;
            double x=start.x();
            if (end.y()>start.y())
                for (int y=start.y(); y<=end.y(); y++)
                {
                    QPoint cur((int)x,y);
                    workingStack.append(cur);
                    distStack.append(0);
                    x+=slope;
                }
            else
                for (int y=start.y(); y>=end.y(); y--)
                {
                    QPoint cur((int)x,y);
                    workingStack.append(cur);
                    distStack.append(0);
                    x-=slope;
                }
        }
        else
        {
            double y=start.y();
            if (end.x()>start.x())
                for (int x=start.x(); x<=end.x(); x++)
                {
                    QPoint cur(x,(int) y);
                    workingStack.append(cur);
                    distStack.append(0);
                    y+=slope;
                }
            else
                for (int x=start.x(); x>=end.x(); x--)
                {
                    QPoint cur(x,(int) y);
                    workingStack.append(cur);
                    distStack.append(0);
                    y-=slope;
                }
        }
        
        
        setPixelSlope(refPoints[i].x(),refPoints[i].y(),angle,1);
        
        double fillRadious = HARD_FILL_RADIOUS_CONST;// * refLengthsM[i];
        
        while(!workingStack.empty())
        {
            QPoint cur = workingStack.front();
            workingStack.pop_front();
            double curDist = distStack.front();
            distStack.pop_front();
//            printf("curDist=%f\n",curDist);
            if (curDist > fillRadious)
            {
                edgeStack.append(cur);
                angleEdgeStack.append(angle);
                distanceEdgeStack.append(curDist);
            }
            else
            {
                double newDist = curDist+1;
                double str = (newDist+1)/(2*newDist);
                if (cur.x()<width()-1 && noStrongerAngleForPixel(cur.x()+1,cur.y(),angle,str))
                {
                    QPoint pp(cur.x()+1,cur.y());
                    workingStack.push_back(pp);
                    
                    distStack.push_back(newDist);
                    
                    setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.y()<height()-1 && noStrongerAngleForPixel(cur.x(),cur.y()+1,angle,str))
                {
                    QPoint pp(cur.x(),cur.y()+1);
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.x()>0 && noStrongerAngleForPixel(cur.x()-1,cur.y(),angle,str))
                {
                    QPoint pp(cur.x()-1,cur.y());
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.y()>0 && noStrongerAngleForPixel(cur.x(),cur.y()-1,angle,str))
                {
                    QPoint pp(cur.x(),cur.y()-1);
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                //diagonals
                newDist = curDist+SQRT_2;
                str = (newDist+1)/(2*newDist);
                if (cur.x()<width()-1 && cur.y()<height()-1 && noStrongerAngleForPixel(cur.x()+1,cur.y()+1,angle,str))
                {
                    QPoint pp(cur.x()+1,cur.y()+1);
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.y()<height()-1 && cur.x()>0 && noStrongerAngleForPixel(cur.x()-1,cur.y()+1,angle,str))
                {
                    QPoint pp(cur.x()-1,cur.y()+1);
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.x()<width()-1 && cur.y()>0 && noStrongerAngleForPixel(cur.x()+1,cur.y()-1,angle,str))
                {
                    QPoint pp(cur.x()+1,cur.y()-1);
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.y()>0 && cur.x()>0 && noStrongerAngleForPixel(cur.x()-1,cur.y()-1,angle,str))
                {
                    QPoint pp(cur.x()-1,cur.y()-1);
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    setPixelSlope(pp.x(),pp.y(),angle,str);
                }
            }
        }
    }
    
    
    //after fill
    while (!edgeStack.empty())
    {
        QPoint cur = edgeStack.front();
        edgeStack.pop_front();
        double curDist = distanceEdgeStack.front();
        distanceEdgeStack.pop_front();
        double angle = angleEdgeStack.front();
        angleEdgeStack.pop_front();
        

        if (cur.x()<width()-1 && noAnglesForPixel(cur.x()+1,cur.y()))
        {
            QPoint pp(cur.x()+1,cur.y());
            edgeStack.push_back(pp);
            double newDist = curDist+1;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.y()<height()-1 && noAnglesForPixel(cur.x(),cur.y()+1))
        {
            QPoint pp(cur.x(),cur.y()+1);
            edgeStack.push_back(pp);
            double newDist = curDist+1;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.x()>0 && noAnglesForPixel(cur.x()-1,cur.y()))
        {
            QPoint pp(cur.x()-1,cur.y());
            edgeStack.push_back(pp);
            double newDist = curDist+1;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.y()>0 && noAnglesForPixel(cur.x(),cur.y()-1))
        {
            QPoint pp(cur.x(),cur.y()-1);
            edgeStack.push_back(pp);
            double newDist = curDist+1;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        //diagonals
        if (cur.x()<width()-1 && cur.y()<height()-1 && noAnglesForPixel(cur.x()+1,cur.y()+1))
        {
            QPoint pp(cur.x()+1,cur.y()+1);
            edgeStack.push_back(pp);
            double newDist = curDist+SQRT_2;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.y()<height()-1 && cur.x()>0 && noAnglesForPixel(cur.x()-1,cur.y()+1))
        {
            QPoint pp(cur.x()-1,cur.y()+1);
            edgeStack.push_back(pp);
            double newDist = curDist+SQRT_2;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.x()<width()-1 && cur.y()>0 && noAnglesForPixel(cur.x()+1,cur.y()-1))
        {
            QPoint pp(cur.x()+1,cur.y()-1);
            edgeStack.push_back(pp);
            double newDist = curDist+SQRT_2;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.y()>0 && cur.x()>0 && noAnglesForPixel(cur.x()-1,cur.y()-1))
        {
            QPoint pp(cur.x()-1,cur.y()-1);
            edgeStack.push_back(pp);
            double newDist = curDist+SQRT_2;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        
    }
}

bool AngleImage::pixel(const QPoint &p) const
{
    assert(p.x()>=0 && p.x()<width() && p.y()>=0 && p.y()<height());

    return src->pixel(p.x(),p.y()   );
}
bool AngleImage::pixel(int x, int y) const
{
    assert(x>=0 && x<width() && y>=0 && y<height());

    return src->pixel(x,y);
}
int AngleImage::width() const
{
    return src->width();
}
int AngleImage::height() const
{
    return src->height();
}

void AngleImage::setPixelSlope(const QPoint &p, double angle, double strength)
{
    setPixelSlope(p.x(),p.y(),angle,strength);
}

void AngleImage::setPixelSlope(int x, int y, double angle, double strength)
{
    assert(x>=0 && x<width() && y>=0 && y<height());
//    if (!angles[x][y].contains(angle))
//        angles[x][y][angle]=strength;
//    else
//        angles[x][y][angle]=std::max(strength,angles[x][y][angle]);
    int bin = getBinForAngle(angle);
    if (!binStr[x][y].contains(bin))
        binStr[x][y][bin]=strength;
    else
        binStr[x][y][bin]=std::max(strength,binStr[x][y][bin]);
    
//    double portionLeft=portion;
//    if (angles[x][y].contains(-1))
//    {
//        portionLeft = max(portionLeft-angles[x][y][-1],0);
//        angles[x][y][-1]-=portion-portionLeft;
//        if (angles[x][y][-1]==0)
//            angles[x][y].remove(-1);
//    }
//    if (portionLeft>0)
//    {
//        if (angles[x][y].size() > 0)
//        {
//            double old = 0;
//            if (angles[x][y].contains(angle))
//            {
//                old = pixels[x][y][angle];
//            }
//            double converter = (1-portionLeft)/(1-old);
//            QMap<double,double>::iterator i = angles[x][y].begin();
//            while(i != angles[x][y].end())
//            {
//                if (i.key() != angle)
//                {
//                    double oldother = i.value();
//                    if (oldother!=0)
//                    {
//                        i.value()=oldother*converter;
//                    }
//                }
//                ++i;
//            }
//        }
        
//    }
//    pixels[x][y][angle]=portion;
}



BImage AngleImage::makeImage() const
{
    return src->makeImage();
}

bool AngleImage::pixelIsMine(int x, int y) const{return true;}

void AngleImage::setNumOfBinsMinValMaxVal(int numOfBins, double minVal, double maxVal)
{
    this->numOfBins = numOfBins;
    minValue=minVal;
    maxValue=maxVal;
}

int AngleImage::getNumOfBins() const
{
    return numOfBins;
}

//returns the relative strength to the highest
QMap<int,double> AngleImage::getBinsAndStrForPixel(int x, int y) const
{
    QMap<int,double> ret;
    double totalStr=0;
    double maxStr=0;
//    foreach (double str, angles[x][y].values())
//    {
//        totalStr+=str;
//        if (str>maxStr)
//            maxStr=str;
//    }

//    foreach (double angle, angles[x][y].keys())
//    {
//        int bin = getBinForAngle(angle);
//        double strength = angles[x][y][angle]/maxStr;
//        ret[bin]=strength;//this overwrites previous data
//    }
    
    foreach (double str, binStr[x][y].values())
    {
        totalStr+=str;
        if (str>maxStr)
            maxStr=str;
    }

    foreach (int bin, binStr[x][y].keys())
    {
        double strength = binStr[x][y][bin]/maxStr;
        ret[bin]=strength;//this overwrites previous data
    }

    return ret;
}

int AngleImage::getBinForAngle(double angle) const
{
    return (int)((angle-minValue)*((numOfBins-1)/(maxValue-minValue)));
}

bool AngleImage::noStrongerAngleForPixel(int x, int y, double angle, double strength) const
{
//    if (pixel(x,y))
//    {
//        if(!angles[x][y].contains(angle))
//            return true;
//        else
//            return angles[x][y][angle]<strength;
//    }
    int bin = getBinForAngle(angle);
    if (pixel(x,y))
    {
        if(!binStr[x][y].contains(bin))
            return true;
        else
            return binStr[x][y][bin]<strength;
    }
    return false;
}

bool AngleImage::noAnglesForPixel(int x, int y) const
{
//    return pixel(x,y) && angles[x][y].empty();
    return pixel(x,y) && binStr[x][y].empty();
}


//QPoint AngleImage::findClosestPoint(QPoint &start)
//{
//    QVector<QPoint> searchQueue;
//    searchQueue.append(start);
//    BImage mark(src->width(),src->height());
//    for (int x=0; x<mark.width(); x++)
//    {
//        for (int y=0; y<mark.height(); y++)
//        {
//            mark.setPixel(x,y,true);
//        }
//    }
//    mark.setPixel(start,false);
//    while (!searchQueue.empty())
//    {
//        QPoint cur = searchQueue.front();
//        searchQueue.pop_front();
//        if (pixel(cur))
//            return cur;
        
//        QPoint up(cur.x(),cur.y()-1);
//        QPoint down(cur.x(),cur.y()+1);
//        QPoint left(cur.x()-1,cur.y());
//        QPoint right(cur.x()+1,cur.y());
//        QPoint lu(cur.x()-1,cur.y()-1);
//        QPoint ld(cur.x()-1,cur.y()+1);
//        QPoint ru(cur.x()+1,cur.y()-1);
//        QPoint rd(cur.x()+1,cur.y()+1);
//        if (cur.y()>0 && mark.pixel(up))
//        {
//            searchQueue.append(up);
//            mark.setPixel(up,false);
//        }
//        if (cur.y()+1<mark.height() && mark.pixel(down))
//        {
//            searchQueue.append(down);
//            mark.setPixel(down,false);
//        }
//        if (cur.x()>0 && mark.pixel(left))
//        {
//            searchQueue.append(left);
//            mark.setPixel(left,false);
//        }
//        if (cur.x()+1<mark.width() && mark.pixel(right))
//        {
//            searchQueue.append(right);
//            mark.setPixel(right,false);
//        }
//        if (cur.x()>0 && cur.y()>0 &&mark.pixel(lu))
//        {
//            searchQueue.append(lu);
//            mark.setPixel(lu,false);
//        }
//        if (cur.x()>0 && cur.y()+1<mark.height() && mark.pixel(ld))
//        {
//            searchQueue.append(ld);
//            mark.setPixel(ld,false);
//        }
//        if (cur.x()+1<mark.width() && cur.y()>0 && mark.pixel(ru))
//        {
//            searchQueue.append(ru);
//            mark.setPixel(ru,false);
//        }
//        if (cur.x()+1<mark.width() && cur.y()+1<mark.height() && mark.pixel(rd))
//        {
//            searchQueue.append(rd);
//            mark.setPixel(rd,false);
//        }
        
//    }
//    printf("findClosestPointOn failed to find point\n");
//    QPoint x(-1,-1);
//    return x;
//}

//////////////////


