#ifndef BIMAGE_H
#define BIMAGE_H
#include <QImage>
#include <QMap>
#include "Constants.h"
#include "bpartition.h"
#include <assert.h>
#include "bpartition.h"
#include "BPixelCollection.h"

class BPartition;

struct bPixel
{
    bool val;
    QMap<BPartition*, float> ownership;
};

class BImage : public BPixelCollection
{
public:
    BImage();
    BImage(const QImage &src);
    BImage(int width, int height);
    BImage(const BImage &other);
//    BImage(const BPartition* src1, const BPartition* src2);
    BImage copy();
    ~BImage();
    
    int width() const;
    int height() const;
    
    void save(const QString& filepath);
    void saveOwners(const QString& filepath);
    
    bool pixel(const QPoint &p) const;
    bool pixel(int x, int y) const;
    bPixel pixelFull(const QPoint &p) const;
    bPixel pixelFull(int x, int y) const;
    float pixelOwnerPortion(const QPoint &p, BPartition* owner) const;
    float pixelOwnerPortion(int x, int y, BPartition* owner) const;
    BPartition* pixelMajorityOwner(const QPoint &p) const;
    BPartition* pixelMajorityOwner(int x, int y) const;
    
    void setPixel(const QPoint &p, bool val);
    void setPixel(int x, int y, bool val);
    void setPixelFull(const QPoint &p, const bPixel &strct);
    void setPixelFull(int x, int y, const bPixel &strct);
    void setPixelOwner(const QPoint &p, BPartition* owner, float portion);
    void setPixelOwner(int x, int y, BPartition* owner, float portion);
    
//    void setPixelCombineOwner(int x, int y, BPartition* to, BPartition* with);
    
    QImage getImage();
    QImage getOwnersImage();
    
    void claimOwnership(BPartition* claimer, float amount);
    
    BPartition* getFullPartition();
    
    BImage makeImage() const;
    bool pixelIsMine(int x, int y) const;
    
private:
    bPixel** pixels;
    int myWidth;
    int myHeight;
};

#endif // BIMAGE_H
