#!/usr/bin/python

import re
import xml.etree.ElementTree as ET
import os
import sys
from StringIO import StringIO
import cv2


#imgDir = 'BenthamDatasetR0-Images/Images/Pages/'
#saveDor = 'words/'

if len(sys.argv)>8:
    corpusFile = sys.argv[1]
    trainFile = sys.argv[2]
    outTrain1File = sys.argv[3]
    outValid1File = sys.argv[4]
    outTrain2File = sys.argv[5]
    outValid2File = sys.argv[6]
    imgDir = sys.argv[7]
    saveDir = sys.argv[8]
else:
    print 'usage: '+sys.argv[0]+' corpus.gtp train.gtp toTrain1.json toValid1.json(corpus) toTrain2.json(corpus) toValid2.json imgDir saveDir'
    exit(0)
if imgDir[-1]!='/':
    imgDir+='/'
if saveDir[-1]!='/':
    saveDir+='/'

setHeight=None
if len(sys.argv)>9:
    setHeight = int(sys.argv[9])

corpusFH = open(corpusFile)
corpus = corpusFH.read().splitlines()
corpusFH.close()
trainFH = open(trainFile)
train = trainFH.read().splitlines()
trainFH.close()

#save images
count=0

def parse(gtp):
    global count
    json=[]
    curImageFile=''
    orig=None
    for line in gtp:
        m = re.match(r'(.*\.\w+) (\d+) (\d+) (\d+) (\d+) (.*)',line.strip())
        if m is not None:
            imageFile=m.group(1)
            x1=int(m.group(2))
            y1=int(m.group(3))
            x2=int(m.group(4))
            y2=int(m.group(5))
            label=m.group(6)

            if imageFile != curImageFile:
                orig = cv2.imread(imgDir+imageFile)
                if orig is None:
                    print('ERROR, couldnt open: '+imgDir+imageFile+imgExt)
                    exit(0)
                curImageFile=imageFile

            image = orig[y1:y2+1, x1:x2+1]
            if setHeight is not None:
                image = cv2.resize(image, (image.shape[1],setHeight))

            imageFile = saveDir +'{:0>6}'.format(str(count)) + '.png'
            count+=1
            #cv2.imwrite(imageFile,image)

            json.append('{h:'+str(image.shape[0])+',w:'+str(image.shape[1])+',gt:"'+label+'", dataset_id:"NAMES",image_path:"'+imageFile+'"}')
    return json

trainJSON = parse(train)
corpusJSON = parse(corpus)

outTrain = open(outTrain1File,'w')
for j in trainJSON:
    outTrain.write(j+',')
outTrain.close()

outValid = open(outValid1File,'w')
for j in corpusJSON:
    outValid.write(j+',')
outValid.close()

total = len(corpusJSON) + len(trainJSON)
forTrain = int(total*0.85)


outTrain = open(outTrain2File,'w')
for j in corpusJSON:
    outTrain.write(j+',')

i=0
while i+len(corpusJSON)<forTrain:
    outTrain.write(trainJSON[i]+',')
    i+=1
outTrain.close()

outValid = open(outValid2File,'w')
while i<len(trainJSON):
    outValid.write(trainJSON[i]+',')
    i+=1
outValid.close()
