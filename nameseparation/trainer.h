#ifndef TRAINER_H
#define TRAINER_H

#include <QString>
#include <QImage>

class Trainer
{
public:
    
    
    static void trainTwoWordSeparation(QString dir);
    static void trainTwoWordSeparationDumb(QString dir);
    static void trainIcdar(QString icdarRoot, int only);
};

#endif // TRAINER_H
