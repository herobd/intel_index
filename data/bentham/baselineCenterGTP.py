import sys
import re
import cv2
import math
import numpy as np

if len(sys.argv)<6:
    print 'This program takes two gtp files and normalizes them by center of mass (ink). It additionally makes them all the same height, taking the heighest image as the rule'
    print 'usage: '+sys.argv[0]+' imgDir in1.gtp in2.gtp out1.gtp out2.gtp'
    exit(0)

imgDir=sys.argv[1]

with open(sys.argv[2],'r') as f:
    file1 = f.read().splitlines()
with open(sys.argv[3],'r') as f:
    file2 = f.read().splitlines()

#outFile1=open(sys.argv[4],'w')
outFile2=open(sys.argv[5],'w')

def lookAt(data):
    maxH=0
    sumH=0
    count=0.0
    for line in data:
        m = re.match(r'(.*\.\w+) (\d+) (\d+) (\d+) (\d+) (.*)',line.strip())
        if m is not None:
            count+=1
            x1=int(m.group(2))
            y1=int(m.group(3))
            x2=int(m.group(4))
            y2=int(m.group(5))


            h=y2-y1 +1
            sumH+=h
            if h>maxH:
                maxH=h

        else:
            print('parse error on: '+line.strip())
            exit(1)
    return maxH , sumH/count

def getVariance(avgH,data):
    sumH=0
    count=0.0
    for line in data:
        m = re.match(r'(.*\.\w+) (\d+) (\d+) (\d+) (\d+) (.*)',line.strip())
        if m is not None:
            count+=1
            x1=int(m.group(2))
            y1=int(m.group(3))
            x2=int(m.group(4))
            y2=int(m.group(5))


            h=y2-y1 +1
            sumH+= (avgH-h)*(avgH-h)

        else:
            print('parse error on: '+line.strip())
            exit(1)
    return sumH/count

def center(out,data,tH):
    curImFile=''
    im=None
    for line in data:
        m = re.match(r'(.*\.\w+) (\d+) (\d+) (\d+) (\d+) (.*)',line.strip())
        if m is not None:
            x1=int(m.group(2))
            y1=int(m.group(3))
            x2=int(m.group(4))
            y2=int(m.group(5))

            imFile=m.group(1)
            label=m.group(6)

            h=y2-y1 +1
            if imFile is not curImFile:
                curImFile=imFile
                im = cv2.imread(imgDir+imFile,cv2.IMREAD_GRAYSCALE)
                if im is None:
                    print "Failed to open: "+imgDir+imFile
            if im is None:
                print 'read error: '+curImFile+' / '+imFile
            wordIm = im[y1:y2+1, x1:x2+1]
            #invert so ink is high value
            wordIm = 255-wordIm

            rowsSummed = np.sum(wordIm,axis=1)
            total = np.sum(rowsSummed)

            mid=0
            for r in range(0,h):
                mid += r*rowsSummed[r]/total
            
            inImMid=y1+mid

            ny1=int(math.ceil(inImMid-(tH/2)))
            ny2=int(math.floor(inImMid+(tH/2)))
            out.write(imFile+' '+str(x1)+' '+str(ny1)+' '+str(x2)+' '+str(ny2)+' '+label+'\n')
            #print('mid: '+str(mid))
            #print('old y1:'+str(y1)+', new:'+str(ny1))
            #print('old y2:'+str(y2)+', new:'+str(ny2))
            #wordIm = im[ny1:ny2+1, x1:x2+1]
            #cv2.imshow("word",wordIm)
            #cv2.waitKey()

        else:
            print('parse error on: '+line.strip())
            exit(1)


maxH1,avgH1= lookAt(file1)
maxH2,avgH2= lookAt(file2)

maxH = max(maxH1,maxH2)
avgH = (avgH1+avgH2)/2.0

variance1 = getVariance(avgH,file1)
variance2 = getVariance(avgH,file2)
stdDev = math.sqrt((variance1+variance2)/2)

#print('maxH: '+str(maxH))
#print('1sd: '+str(avgH+stdDev))
print('1.5sd: '+str(avgH+1.5*stdDev))

#center(outFile1,file1,avgH+1.5*stdDev)
center(outFile2,file2,avgH+1.5*stdDev)

#outFile1.close()
outFile2.close()
