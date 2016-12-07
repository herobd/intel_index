#!/usr/bin/python

#This takes a GTP file and acts as a tool to segment each word image into characters

import re
import xml.etree.ElementTree as ET
import os
import sys
from StringIO import StringIO
import cv2

#gtpFile = 'ben_cattss_c_corpus.gtp'
#imDir = 'BenthamDatasetR0-Images/Images/Pages/'
#outFile = 'manual_segmentations.csv'

gtpFile = 'seg_names_corpus.gtp'
imDir = './'
outFile = 'manual_segmentations.csv'

def showControls():
    print(' -----------------------------------------------')
    print('| CONTROLS:                                     |')
    print('| * set new boundary:               right-click |')
    print('| * reset current boundary:         left-click  |')
    print('| * set boundary to image edge:     middle-click|')
    print('| * confirm segmentaion (of word):  enter       |')
    print('| * undo:                           backspace   |')
    print('| * start previous word over:       backspace(2)|')
    print('| * start current word over:        delete      |')
    print('| * change transcription for image: insert      |')
    print('| * exit:                           esc         |')
    print('|                                               |')
    print(' -----------------------------------------------')


def parseGTP(gtp):
    pageCount=0
    words=[]
    curImageFile=''
    orig=None
    for line in gtp:
        m = re.match(r'(.*\.\w+) (\d+) (\d+) (\d+) (\d+) (.*)',line.strip())
        if m is not None:
            imageFile=m.group(1)
            if curImageFile=='':
                curImageFile=imageFile
            else:
                if curImageFile != imageFile:
                    pageCount+=1
                    curImageFile=imageFile
            x1=int(m.group(2))
            y1=int(m.group(3))
            x2=int(m.group(4))
            y2=int(m.group(5))
            label=m.group(6)
            words.append({'imageFile':imageFile, 'x1':x1, 'y1':y1, 'x2':x2, 'y2':y2, 'page': pageCount, 'gt':label})
    return words


curImageFile=''
orig=None
segPts = []
image=None
colorIdx=0
wordLen=0
#leadColor=[(200, 255, 0), (0, 145, 255), (105, 0, 205)]
#color = [(70, 255, 0), (0, 205, 255), (255, 0, 205)]
color = [(.5, 1, 0), (0, .5, 1), (1, 0, .5), (.75,.75,0), (0,.75,.75), (.75,0,.75)]
#color = [(160, 255, 0), (0, 160, 255), (255, 0, 160), (200,200,0), (0,200,200), (200,0,200)]
bimage=None
def clicker(event, x, y, flags, param):
        # grab references to the global variables
        global segPts, image, bimage, colorIdx, wordLen

        if event == cv2.EVENT_LBUTTONDOWN:
            if len(segPts)>0:
                #change last boundary
                image=bimage.copy()
                segPts[-1]=x
                ll=max(0,segPts[-1]-1)
                rr=min(image.shape[1], segPts[-1]+1)
                image[:,ll:rr,0] = color[(colorIdx+len(color)-1)%len(color)][0] * image[:,ll:rr,0]
                image[:,ll:rr,1] = color[(colorIdx+len(color)-1)%len(color)][1] * image[:,ll:rr,1]
                image[:,ll:rr,2] = color[(colorIdx+len(color)-1)%len(color)][2] * image[:,ll:rr,2]
                cv2.imshow("image", image)

        elif event == cv2.EVENT_RBUTTONDOWN:
                # a new boundary
                if len(segPts)<wordLen+1:
                    bimage=image.copy()
                    segPts.append(x)

                    ll=max(0,segPts[-1]-1)
                    rr=min(image.shape[1], segPts[-1]+1)
                    image[:,ll:rr,0] = color[colorIdx][0] * image[:,ll:rr,0]
                    image[:,ll:rr,1] = color[colorIdx][1] * image[:,ll:rr,1]
                    image[:,ll:rr,2] = color[colorIdx][2] * image[:,ll:rr,2]
                    cv2.imshow("image", image)
                    colorIdx = (colorIdx+1)%len(color)

        elif event == cv2.EVENT_MBUTTONDOWN:
                # set edge boundary
                if len(segPts)==wordLen:
                    bimage=image.copy()
                    segPts.append(image.shape[1]-1)

                    ll=max(0,segPts[-1]-1)
                    rr=min(image.shape[1], segPts[-1]+1)
                    image[:,ll:rr,0] = color[colorIdx][0] * image[:,ll:rr,0]
                    image[:,ll:rr,1] = color[colorIdx][1] * image[:,ll:rr,1]
                    image[:,ll:rr,2] = color[colorIdx][2] * image[:,ll:rr,2]
                    cv2.imshow("image", image)
                    colorIdx = (colorIdx+1)%len(color)
                elif len(segPts)==wordLen+1:
                    image=bimage.copy()
                    segPts[-1]=(image.shape[1]-1)

                    ll=max(0,segPts[-1]-1)
                    rr=min(image.shape[1], segPts[-1]+1)
                    image[:,ll:rr,0] = color[(colorIdx+len(color)-1)%len(color)][0] * image[:,ll:rr,0]
                    image[:,ll:rr,1] = color[(colorIdx+len(color)-1)%len(color)][1] * image[:,ll:rr,1]
                    image[:,ll:rr,2] = color[(colorIdx+len(color)-1)%len(color)][2] * image[:,ll:rr,2]
                    cv2.imshow("image", image)
                if len(segPts)==0:
                    bimage=image.copy()
                    segPts.append(0)

                    ll=max(0,segPts[-1]-1)
                    rr=min(image.shape[1], segPts[-1]+1)
                    image[:,ll:rr,0] = color[colorIdx][0] * image[:,ll:rr,0]
                    image[:,ll:rr,1] = color[colorIdx][1] * image[:,ll:rr,1]
                    image[:,ll:rr,2] = color[colorIdx][2] * image[:,ll:rr,2]
                    cv2.imshow("image", image)
                    colorIdx = (colorIdx+1)%len(color)
                elif len(segPts)==1:
                    image=bimage.copy()
                    segPts[-1]=0

                    ll=max(0,segPts[-1]-1)
                    rr=min(image.shape[1], segPts[-1]+1)
                    image[:,ll:rr,0] = color[(colorIdx+len(color)-1)%len(color)][0] * image[:,ll:rr,0]
                    image[:,ll:rr,1] = color[(colorIdx+len(color)-1)%len(color)][1] * image[:,ll:rr,1]
                    image[:,ll:rr,2] = color[(colorIdx+len(color)-1)%len(color)][2] * image[:,ll:rr,2]
                    cv2.imshow("image", image)

def segmenter(word):
    global colorIdx, bimage, image, segPts, wordLen
    wordLen = len(word['gt'])
    #print((x1,y1,x2,y2))
    redo=True
    while redo: #undo loop
        redo=False
        x1=word['x1']
        y1=word['y1']
        x2=word['x2']
        y2=word['y2']
        image = orig[y1:y2+1, x1:x2+1].copy()
        #bimage = orig[y1:y2+1, x1:x2+1].copy()

        #lastX=0
        colorIdx=0
        #wi=0
        print('Image[!]: '+word['gt'])
        
        segPts = []
        #cv2.rectangle(image, (refPt[0],0), (refPt[0],image.shape[0]), leadColor[colorIdx], 2)
        cv2.imshow("image", image)
        while True:
            # display the image and wait for a keypress
            key = cv2.waitKey(33) #?? & 0xFF
            if key == 10 and len(segPts)==wordLen+1: #eneter
                #toWrite+=filePath+' '+str(x1+refPt[0])+' '+str(y1)+' '+str(x1+refPt[1])+' '+str(y2)+' '+name+'\n'
                #niewWordSeg.append(( (x1+refPt[0]), y1, (x1+refPt[1]), y2));
                #bimage=image.copy()
                #lastX=refPt[1]
                toWrite = word['gt']+','+str(word['page'])+','+str(word['x1'])+','+str(word['y1'])+','+str(word['x2'])+','+str(word['y2'])
                for i in range(len(segPts)-1):
                    toWrite+=','+str(segPts[i]+x1)+','+str(segPts[i+1]+x1)
                toWrite+='\n';
                return toWrite, False, False
            elif key == 65288: #backspace
                if len(segPts)>0:
                    image = orig[y1:y2+1, x1:x2+1].copy()
                    segPts.pop()
                    colorIdx=0
                    for i in range(len(segPts)):
                        ll=max(0,segPts[i]-1)
                        rr=min(image.shape[1], segPts[i]+1)
                        image[:,ll:rr,0] = color[colorIdx][0] * image[:,ll:rr,0]
                        image[:,ll:rr,1] = color[colorIdx][1] * image[:,ll:rr,1]
                        image[:,ll:rr,2] = color[colorIdx][2] * image[:,ll:rr,2]
                        colorIdx+=1
                    cv2.imshow("image", image)
                else:
                    return '', True, False
            elif key == 65535: #del
                if len(segPts)>0:
                    print('[CLEAR]')
                    redo=True
                    break
                else:
                    return '', True, False
            elif key == 65379: #insert
                print("Enter new transcription for whole image. Don't include punctuation/specail marks.")
                gt=raw_input(':')
                #newWords = gt.split()
                redo=True
                break
            elif key == 27: #esc
                print('esc')
                return '', False, True
                #exit(0)
                #break

    #return newWords, newWordBoxes

cv2.namedWindow("image")
cv2.setMouseCallback("image", clicker)

didCount=0
try:
    check = open(outFile,'r')
    did = check.read().splitlines()
    didCount=len(did)
    check.close()
except IOError:
    print ('making new out')

out = open(outFile,'a')

i=didCount
#pageCount=-1
prevSeg=''
seg=''
showControls()
inF = open(gtpFile,'r')
gtp = inF.read().splitlines()
words = parseGTP(gtp)
end=False
while i<len(words) and not end:
    if i%10==9:
        showControls()
    print(str(i)+' of '+str(len(words)))
    if curImageFile != words[i]['imageFile']:
        #pageCount+=1
        orig = cv2.imread(imDir+words[i]['imageFile'])
        assert orig is not None
        curImageFile=words[i]['imageFile']
    seg, undo, end = segmenter(words[i])
    
    if undo and i>0:
        prevSeg=''
        print(str(i-1)+' of '+str(len(words)))
        #pp=pageCount
        tmpOrig=orig
        if curImageFile != words[i-1]['imageFile']:
            #pp-=1
            orig = cv2.imread(imDir+words[i-1]['imageFile'])
        prevSeg, undo, end = segmenter(words[i-1])
        orig=tmpOrig
    else:
        out.write(prevSeg)
        prevSeg=seg
        seg=''
        i+=1
out.write(prevSeg)
out.write(seg)
