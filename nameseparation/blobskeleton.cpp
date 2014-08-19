#include "blobskeleton.h"

BlobSkeleton::BlobSkeleton(const BPixelCollection* src)
{
    this->src=src;
    assignments = new int*[src->width()];
    for (int x=0; x<src->width(); x++)
    {
        assignments[x] = new int[src->height()];
        for (int y=0; y<src->height(); y++)
            assignments[x][y]=-1;
    }
    
    QPoint startPoint = findStartPoint();
    blobFill(startPoint);
    draw("test");
}

BlobSkeleton::~BlobSkeleton()
{
    for (int x=0; x<src->width(); x++)
    {
        delete[] assignments[x];
    }
    delete[] assignments;
}

QPoint BlobSkeleton::findStartPoint()
{
    QPoint seed;
    for (int y=0; y<src->height(); y++)
    {
        for (int xDelta=0; xDelta<src->width()/2; xDelta++)
        {
            if (src->pixel(xDelta+(src->width()/2),y))
            {
                    seed.setX(xDelta+(src->width()/2));
                    seed.setY(y);
                    y=src->height();
                    break;
            }
            else if (src->pixel((-xDelta)+(src->width()/2),y))
            {
                    seed.setX((-xDelta)+(src->width()/2));
                    seed.setY(y);
                    y=src->height();
                    break;
            }
              
        }
    }
    BImage mark=src->makeImage();
    QVector<QPoint> border;
    border.append(seed);
    
    QVector<QPoint> collection;
    int furthestDistSqrd=0;
    int killTokenLoc=-1;
    int sumX=0;
    int sumY=0;
    
    while(!border.empty())
    {
        QPoint toAdd = border.front();
        border.pop_front();
        
        if (toAdd.x()==-1)//hit kill token
            break;
        
        int myFurthestDistSqrd=0;
        foreach (QPoint p, collection)
        {
            int distSqrd=pow(p.x()-toAdd.x(),2) + pow(p.y()-toAdd.y(),2);
            if (distSqrd>myFurthestDistSqrd)
                myFurthestDistSqrd=distSqrd;
        }
        
        
        if (collection.size()==0 || std::max(myFurthestDistSqrd,furthestDistSqrd)/(1.0*collection.size()) <= ECCENTRICITY_LIMIT)
        {
            if (killTokenLoc>=0)
            {
                border.remove(--killTokenLoc);//remove killToken
                killTokenLoc=-1;
            }
            
            if (myFurthestDistSqrd>furthestDistSqrd)
                furthestDistSqrd=myFurthestDistSqrd;
            
            collection.push_back(toAdd);
            sumX+=toAdd.x();
            sumY+=toAdd.y();
            
            // 0 1 2 neighbor id table
            // 3 4 5
            // 6 7 8
            int tableIndex=8;
            for (int cc=0; cc<9; cc++)
            {
                tableIndex=(tableIndex+2)%9;
                if (tableIndex==4)
                    continue;
                
                int xDelta=(tableIndex%3)-1;
                int yDelta=(tableIndex/3)-1;
                int x = toAdd.x()+xDelta;
                int y = toAdd.y()+yDelta;
                if (x>=0 && x<mark.width() && y>=0 && y<mark.height())
                {
                    if (mark.pixel(x,y))
                    {
                        QPoint p(x,y);
                        border.append(p);
                        mark.setPixel(p,false);
                    }
                }
            }
          
            
        }
        else if (killTokenLoc<0)
        {
            killTokenLoc=border.size();
            QPoint killToken(-1,-1);
            border.push_back(killToken);
            border.push_back(toAdd);
        }
        else
        {
            killTokenLoc--;
            border.push_back(toAdd);
        }
        
    }
    QPoint ret(sumX/collection.size(),sumY/collection.size());
    return ret;
}

void BlobSkeleton::blobFill(const QPoint &begin)
{
//    QVector<QPoint> centersOfMass;
    
            
    
    BImage mark = src->makeImage();
    QVector<QPoint> startPoints;
    startPoints.push_back(begin);
    
    while(!startPoints.empty())
    {
        QPoint startPoint = startPoints.front();
        startPoints.pop_front();
        
        if (!mark.pixel(startPoint))
            continue;
        mark.setPixel(startPoint,false);
//        printf("Using start point: (%d,%d)\n",startPoint.x(),startPoint.y());
        
        unsigned int myRegionId = regions.size();
        
        QVector<QPoint> border;
        QVector<QPoint> collection;
        border.push_back(startPoint);
        
        int furthestDistSqrd=0;
        int killTokenLoc=-1;
        int sumX=0;
        int sumY=0;
        QSet<unsigned int> neighborRegions;
        
        
        while(!border.empty())
        {
            QPoint toAdd = border.front();
            border.pop_front();
            
            if (toAdd.x()==-1)//hit kill token
                break;
            
            int myFurthestDistSqrd=0;
            foreach (QPoint p, collection)
            {
                int distSqrd=pow(p.x()-toAdd.x(),2) + pow(p.y()-toAdd.y(),2);
                if (distSqrd>myFurthestDistSqrd)
                    myFurthestDistSqrd=distSqrd;
            }
            
            
            if (collection.size()==0 || std::max(myFurthestDistSqrd,furthestDistSqrd)/(1.0*collection.size()) <= ECCENTRICITY_LIMIT)
            {
                if (killTokenLoc>=0)
                {
                    border.remove(--killTokenLoc);//remove killToken
                    killTokenLoc=-1;
                }
                
                if (myFurthestDistSqrd>furthestDistSqrd)
                    furthestDistSqrd=myFurthestDistSqrd;
                
                collection.push_back(toAdd);
                assignments[toAdd.x()][toAdd.y()] = myRegionId;
                sumX+=toAdd.x();
                sumY+=toAdd.y();
                
                // 0 1 2 neighbor id table
                // 3 4 5
                // 6 7 8
                int tableIndex=8;
                for (int cc=0; cc<9; cc++)
                {
                    tableIndex=(tableIndex+2)%9;
                    if (tableIndex==4)
                        continue;
                    
                    int xDelta=(tableIndex%3)-1;
                    int yDelta=(tableIndex/3)-1;
                    int x = toAdd.x()+xDelta;
                    int y = toAdd.y()+yDelta;
                    if (x>=0 && x<mark.width() && y>=0 && y<mark.height())
                    {
                        if (mark.pixel(x,y))
                        {
                            QPoint p(x,y);
                            border.append(p);
                            mark.setPixel(p,false);
                        }
                        else if (src->pixel(x,y) && 
                                 myRegionId != assignments[x][y] && 
                                 0 <= assignments[x][y])
                        {
                            neighborRegions.insert(assignments[x][y]);
                        }
                        else if (src->pixel(x,y) &&
                                 NO_ASSIGMENT == assignments[x][y])
                        {
                            for (int mDelta=-1; mDelta<2; mDelta+=2)
                            {
                                if (x+mDelta>=0 && x+mDelta<mark.width() &&
                                    src->pixel(x+mDelta,y) && 
                                    myRegionId != assignments[x+mDelta][y] && 
                                    0 <= assignments[x+mDelta][y])
                                {
                                    neighborRegions.insert(assignments[x+mDelta][y]);
                                }
                                if (y+mDelta>=0 && y+mDelta<mark.height() &&
                                         src->pixel(x,y+mDelta) && 
                                         myRegionId != assignments[x][y+mDelta] && 
                                         0 <= assignments[x][y+mDelta])
                                {
                                    neighborRegions.insert(assignments[x][y+mDelta]);
                                }
                            }
                        }
                    }
                }
              
                
            }
            else if (killTokenLoc<0)
            {
                killTokenLoc=border.size();
                QPoint killToken(-1,-1);
                border.push_back(killToken);
                border.push_back(toAdd);
            }
            else
            {
                killTokenLoc--;
                border.push_back(toAdd);
            }
            
        }
        
        if (collection.size() >= MIN_REGION_SIZE)
        {
            tracePoint centerOfMass(sumX/collection.size(),sumY/collection.size());
            
            regions.append(collection);
    //        printf("region %d found %d neighbors\n",myRegionId,neighborRegions.size());
            foreach (unsigned int regionId, neighborRegions)
            {
                double angle = atan2((centerOfMass.y-centersOfMass[regionId].y),(centerOfMass.x-centersOfMass[regionId].x));
                if (angle < 0)
                    angle += PI;
                double distance = sqrt(pow(centerOfMass.x-centersOfMass[regionId].x,2) + pow(centerOfMass.y-centersOfMass[regionId].y,2));
                centerOfMass.connectedPoints.append(regionId);
                centerOfMass.angleBetween.append(angle);
                centerOfMass.distanceBetween.append(distance);
                
                centersOfMass[regionId].connectedPoints.append(myRegionId);
                centersOfMass[regionId].angleBetween.append(angle);
                centersOfMass[regionId].distanceBetween.append(distance);
            }
            centersOfMass.append(centerOfMass);
        }
        else
        {
            
            foreach (QPoint p, collection)
            {
                QSet<unsigned int> localNeighboringRegions;
                assignments[p.x()][p.y()]=NO_ASSIGMENT;
                for (int mDelta=-1; mDelta<2; mDelta+=2)
                {
                    if (p.x()+mDelta>=0 && p.x()+mDelta<mark.width() &&
                        src->pixel(p.x()+mDelta,p.y()) && 
                        myRegionId != assignments[p.x()+mDelta][p.y()] &&
                        0 <= assignments[p.x()+mDelta][p.y()])
                    {
                        localNeighboringRegions.insert(assignments[p.x()+mDelta][p.y()]);
                    }
                    if (p.y()+mDelta>=0 && p.y()+mDelta<mark.height() &&
                             src->pixel(p.x(),p.y()+mDelta) && 
                             myRegionId != assignments[p.x()][p.y()+mDelta] &&
                             0 <= assignments[p.x()][p.y()+mDelta])
                    {
                        localNeighboringRegions.insert(assignments[p.x()][p.y()+mDelta]);
                    }
                }
                
                foreach (unsigned int regionId1, localNeighboringRegions)
                {
                    foreach (unsigned int regionId2, localNeighboringRegions)
                    {
                        if (regionId1!=regionId2 && !centersOfMass[regionId1].connectedPoints.contains(regionId2))
                        {
                            double angle = atan2((centersOfMass[regionId1].y-centersOfMass[regionId2].y),(centersOfMass[regionId1].x-centersOfMass[regionId2].x));
                            if (angle < 0)
                                angle += PI;
                            double distance = sqrt(pow(centersOfMass[regionId1].x-centersOfMass[regionId2].x,2) + pow(centersOfMass[regionId1].y-centersOfMass[regionId2].y,2));
                            
                            centersOfMass[regionId1].connectedPoints.append(regionId2);
                            centersOfMass[regionId1].angleBetween.append(angle);
                            centersOfMass[regionId1].distanceBetween.append(distance);
                            
                            centersOfMass[regionId2].connectedPoints.append(regionId1);
                            centersOfMass[regionId2].angleBetween.append(angle);
                            centersOfMass[regionId2].distanceBetween.append(distance);
                        }
                    }
                }
            }
        }
        
//        printf("Center of mass found: (%d, %d)\n",centerOfMass.x,centerOfMass.y);
        
        foreach (QPoint notAdded, border)//reset points not added
        {
            if (mark.pixel(notAdded))
            {
//                printf("border (%d,%d) slipped\n",notAdded.x(),notAdded.y());
                continue;
            }
            
            QVector<QPoint> neighbors;
            neighbors.push_back(notAdded);
            mark.setPixel(notAdded,true);
            int sumX=0;
            int sumY=0;
            int count=0;
            
            while(!neighbors.empty())
            {
                QPoint cur = neighbors.front();
                neighbors.pop_front();
                sumX+=cur.x();
                sumY+=cur.y();
                count++;
                
                QPoint up(cur.x(),cur.y()-1);
                QPoint down(cur.x(),cur.y()+1);
                QPoint left(cur.x()-1,cur.y());
                QPoint right(cur.x()+1,cur.y());
                QPoint lu(cur.x()-1,cur.y()-1);
                QPoint ld(cur.x()-1,cur.y()+1);
                QPoint ru(cur.x()+1,cur.y()-1);
                QPoint rd(cur.x()+1,cur.y()+1);
                if (cur.y()>0 && !mark.pixel(up) && border.contains(up))
                {
                    neighbors.append(up);
                    mark.setPixel(up,true);
                }
                if (cur.y()+1<mark.height() && !mark.pixel(down) && border.contains(down))
                {
                    neighbors.append(down);
                    mark.setPixel(down,true);
                }
                if (cur.x()>0 && !mark.pixel(left)  && border.contains(left))
                {
                    neighbors.append(left);
                    mark.setPixel(left,true);
                }
                if (cur.x()+1<mark.width() && !mark.pixel(right) && border.contains(right))
                {
                    neighbors.append(right);
                    mark.setPixel(right,true);
                }
                if (cur.x()>0 && cur.y()>0 && !mark.pixel(lu) && border.contains(lu))
                {
                    neighbors.append(lu);
                    mark.setPixel(lu,true);
                }
                if (cur.x()>0 && cur.y()+1<mark.height() && !mark.pixel(ld) && border.contains(ld))
                {
                    neighbors.append(ld);
                    mark.setPixel(ld,true);
                }
                if (cur.x()+1<mark.width() && cur.y()>0 && !mark.pixel(ru) && border.contains(ru))
                {
                    neighbors.append(ru);
                    mark.setPixel(ru,true);
                }
                if (cur.x()+1<mark.width() && cur.y()+1<mark.height() && !mark.pixel(rd) && border.contains(rd))
                {
                    neighbors.append(rd);
                    mark.setPixel(rd,true);
                }
            }
            
            QPoint centerOfMassBorderConnectedComponent(sumX/count, sumY/count);
            if (border.contains(centerOfMassBorderConnectedComponent))
            {
                startPoints.push_back(centerOfMassBorderConnectedComponent);
            }
            else
            {
                int shortestDistance=INT_POS_INFINITY;
                QPoint closestPoint;
                foreach (QPoint p, border)
                {
                    int dist = pow(centerOfMassBorderConnectedComponent.x()-p.x(),2) + pow(centerOfMassBorderConnectedComponent.y()-p.y(),2);
                    if (dist<shortestDistance)
                    {
                        shortestDistance=dist;
                        closestPoint=p;
                    }
                    
                }
                startPoints.push_back(closestPoint);
            }
            
            
//            mark.setPixel(notAdded,true);
            
//            //this is a dumb way, just testing to see if it works
//            startPoints.push_back(notAdded);
        }
        border.clear();
        
    }
    
    
    
    
}


void BlobSkeleton::draw(QString name)
{
    //coloring
    BImage img = src->makeImage();
    QImage lines = img.getImage();
    QVector<QRgb> colorTable=lines.colorTable();
    int NUM_VALS = 245;
    int ctOffset=colorTable.size();
    for (int i=0; i<NUM_VALS; i++)
    {
        QColor color;
        color.setHsv(360*(i*1.0/NUM_VALS),255,255);
        colorTable.append(color.rgb());
    }
    colorTable.append(qRgb(155,155,155));
    
    lines.setColorTable(colorTable);
    
    QVector<BPartition*> parts;
    for (int i=0; i<centersOfMass.size(); i++)
    {
        QVector<QPoint> region = regions[i];
        
        if (region.size()<MIN_REGION_SIZE)
        {
            
//            foreach(QPoint p, region)
//            {
//                img.setPixel(p,false);
//            }
            continue;
        }
        
        img.setPixel(centersOfMass[i].x,centersOfMass[i].y,false);
        BPartition* newPart = new BPartition(&img);
        foreach(QPoint p, region)
        {
            newPart->addPixelFromSrc(p);
        }
        img.claimOwnership(newPart,1);
        
//        if (lines.pixel(centersOfMass[i].x,centersOfMass[i].y)!=blue)
        {
            
            for (int j=0; j<centersOfMass[i].connectedPoints.size(); j++)
            {
                unsigned int index = centersOfMass[i].connectedPoints[j];
                
                double angle = centersOfMass[i].angleBetween[j];
//                printf("Hue used: %d\n",(int)(360*(angle/PI)));
                
                //draw line
                
                if (centersOfMass[i].x != centersOfMass[index].x)
                {
                    double shiftAngle = angle;
                    if (shiftAngle>HALF_PI)
                        shiftAngle-=PI;
                    double slope = tan(shiftAngle);
//                    printf("line of slope %f\n",slope);
                    int start = std::min(centersOfMass[i].x,centersOfMass[index].x);
                    int end = std::max(centersOfMass[i].x,centersOfMass[index].x);
                    double y;
                    if (start==centersOfMass[i].x)
                        y= centersOfMass[i].y;
                    else
                        y= centersOfMass[index].y;
                    for (int x=start; x<end; x++)
                    {
                        lines.setPixel(x,(int)y,(int)(NUM_VALS*(angle/PI)+ctOffset));
                        for (int yDelta=std::min(1,(int)ceil(slope)); yDelta<std::max(1,(int)ceil(slope)); yDelta++)
                            lines.setPixel(x,(int)y + yDelta,(int)(NUM_VALS*(angle/PI)+ctOffset));
                        y+=slope;
                    }
                    
                }
                else
                {
                    int start = std::min(centersOfMass[i].y,centersOfMass[index].y);
                    int end = std::max(centersOfMass[i].y,centersOfMass[index].y);
                    int x =centersOfMass[i].x;
                    for (int y=start; y<=end; y++)
                    {
                        lines.setPixel(x,y,NUM_VALS/2);
                    }
                }
                lines.setPixel(centersOfMass[index].x,centersOfMass[index].y,NUM_VALS+ctOffset);
            }
            lines.setPixel(centersOfMass[i].x,centersOfMass[i].y,NUM_VALS+ctOffset);
        }
        
    }
    img.saveOwners("./" + name + "_blob.ppm");
    lines.save("./" + name + "_lines.ppm");
    
    foreach(BPartition* d,parts)
    {
        delete d;
    }
}

int BlobSkeleton::regionIdForPoint(const QPoint &p)
{
    return assignments[p.x()][p.y()];
}

int BlobSkeleton::regionIdForPoint(int x, int y)
{
    return assignments[x][y];
}
