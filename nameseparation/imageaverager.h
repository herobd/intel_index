#ifndef IMAGEAVERAGER_H
#define IMAGEAVERAGER_H

#include <QImage>

class ImageAverager
{
public:
    ImageAverager();
    ImageAverager(int width, int height);
    QImage averageImages(QString dir, QVector<int> picIds);
    
private:
    int final_width;
    int final_height;
    QVector<QRgb> default_color_table;
};

#endif // IMAGEAVERAGER_H
