#ifndef BPARTITION_H
#define BPARTITION_H

#include "BPixelCollection.h"
#include "bimage.h"


//class BImage;
//class BPixelCollection;

class BPartition : public BPixelCollection
{
public:
    BPartition(const BImage* ofImage, bool whole=false);
    BPartition(const BPartition* ofImage, bool whole=false);
    BPartition(const BPixelCollection* ofImage, bool whole=false);
    ~BPartition();
    BPartition& operator=( const BPartition& other );
    const BPixelCollection* getSrc() const;
    
    int width() const;
    int height() const;
    int getXOffset();
    int getYOffset();
    bool pixel(const QPoint &p) const;
    bool pixel(int x, int y) const;
    bool pixelSrc(int src_x, int src_y) const;
    bool pixelSrcIgnoreOff(int src_x, int src_y) const;
//    bool pixelIsMine(const QPoint &p) const;
    bool pixelIsMineSrc(int src_x, int src_y) const;
    bool pixelIsMineSrc(const QPoint &src_p) const;
    bool pixelIsMine(int x, int y) const;
//    void setPixelOwner(const QPoint &p, BPartition* owner, float portion);
//    void setPixelOwner(int x, int y, BPartition* owner, float portion);
//    void changedPortion(const QPoint &, float newPortion, float oldPortion);
//    void changedPortion(int x, int y, float newPortion, float oldPortion);
    BImage makeImage() const;
    
    void addPixelFromSrc(const QPoint &src_p);
    void addPixelFromSrc(int src_x, int src_y);
    void removePixel(const QPoint &src_p);
    void removePixel(int src_x, int src_y);
    void joinInto(BPartition* other);
    void join(BPartition* other);
//    void remove();
    void clear(BPartition* const pixelsToRemove);
    void shiftDownLayer();
   
    void trim(int offLeftSide, int offRightSide, int offTopSide, int offBottomSide);
    
//    bool rebase();
    void changeSrc(const BPixelCollection* newSrc, int srcXOffset, int srcYOffset);
//    void makeFull();
    int id(){return myId;}
    
private:
    int leftX;
    int rightX;
    int upperY;
    int lowerY;
    const BPixelCollection* src;
    const BImage * rootSrc;
    //unsigned int* myPixels; //flatten 2D array of bits for whole src img
    bool** myPixels;
    
    void initMyPixels(bool s);
    inline void setMyPixelTrue(int src_x, int src_y);
    inline void setMyPixelFalse(int src_x, int src_y);
    
    int myId;
    static int idCounter;
    
};

#endif // BPARTITION_H
