#!/usr/bin/python

import re
import xml.etree.ElementTree as ET
import os
import sys
from StringIO import StringIO
import cv2


xmlDir = 'BenthamDatasetR0-GT/PAGE/' 
trainFile = 'BenthamDatasetR0-GT/Partitions/Train.lst'
validFile = 'BenthamDatasetR0-GT/Partitions/Validation.lst'
testFile = 'BenthamDatasetR0-GT/Partitions/Test.lst'
outTrainFile = 'ben_train.gtp'
outValidFile = 'ben_valid.gtp'
outTestFile = 'ben_test.gtp'
imgDir = 'BenthamDatasetR0-Images/Images/Pages/'

if len(sys.argv)>1:
    xmlDir = sys.argv[1]
if len(sys.argv)>2:
    trainFile = sys.argv[2]
if len(sys.argv)>3:
    validFile = sys.argv[3]
if len(sys.argv)>4:
    testFile = sys.argv[4]
if len(sys.argv)>5:
    outTrainFile = sys.argv[5]
if len(sys.argv)>6:
    outValidFile = sys.argv[6]
if len(sys.argv)>7:
    outTestFile = sys.argv[7]
if len(sys.argv)>8:
    imgDir = sys.argv[8]
if imgDir[-1]!='/':
    imgDir+='/'

def showControls():
    print(' -----------------------------------------------')
    print('| CONTROLS:                                     |')
    print('| * set left border:                left-click  |')
    print('| * set right border:               right-click |')
    print('| * confirm segmentaion:            enter       |')
    print('| * start line over (undo):         backspace   |')
    print('| * change transcription for image: insert      |')
    print('| * shift upper bound up:           page up     |')
    print('| * shift upper bound down:         page down   |')
    print('| * shift lower bound up:           arrow up    |')
    print('| * shift lower bound down:         arrow down  |')
    print('| * exit:                           esc         |')
    print('|                                               |')
    print('| If you finish a line and need to undo it,     |')
    print('| youll need to simply <esc> out of the program |')
    print('| and start that page over (if you havent       |')
    print('| finished the page already).                   |')
    print('| If you need to unto a page, youll have to     |')
    print('| <esc> and manually delete all the entries     |')
    print('| from the *.gtp file for that page.            |')
    print('| This can be done by opening the file with     |')
    print('| the command: (in terminal)                    |')
    print('|            vim <filename>                     |')
    print('| Then typing the command:                      |')
    print('|           :%s/115_009_004.*\\n//               |')
    print('| (This is a regex)                             |')
    print('| Close and save the file by typing:            |')
    print('|                ZZ        (capital)            |')
    print(' -----------------------------------------------')

curImageFile=''
orig=None

def makeGTP(xmlList,outFile):
    global curImageFile, orig, imgDir, imgExt
    
    print ('start on: '+outFile)
    writeGTP=False

    try:
        check = open(outFile,'r')
        did = check.read().splitlines()
        didI=0
        check.close()
    except IOError:
        writeGTP=True

    
    out = open(outFile,'a')
    xmls = open(xmlList)
    pagesInSet=0
    for i in xmls.readlines():
      i=i.strip()          
      #print i
      imageId = i
      with open(xmlDir+i+'.xml') as xmlF:
          xml = xmlF.read()
      it = ET.iterparse(StringIO(xml))
      for _, el in it:
            if '}' in el.tag:
                el.tag = el.tag.split('}', 1)[1]
      root = it.root
      #tree = ET.parse(xmlDir+i+'.xml')
      #root = tree.getroot()
      for page in root.iter('Page'):
        #print 'found page'
        wordsFromThisPage=False
        imageFile = page.get('imageFilename')
        firstOff=True
        todo=[]
        for line in page.iter('TextLine'):
            #print 'a line'
            textE = line.find('TextEquiv')
            textTag = textE.find('Unicode')
            text = textTag.text.strip()
            if text is None or len(text)==0:
                textTag = textE.find('PlainText')
                text = textTag.text.strip()
            text = re.sub(r'[-_~\[\]]',' ',text)
            text = re.sub(r'[^\w\d\s&()]','',text)
            text = re.sub(r'\(',' ( ',text)
            text = re.sub(r'\)',' ) ',text)
            text = re.sub(r'\s+',' ',text)
            words = text.lower().strip().split(' ')
            wordBoxes = []
            for wordTag in line.iter('Word'):
                maxX=0
                minX=9999999
                maxY=0
                minY=9999999
                for point in wordTag.iter('Point'):
                    x=int(point.get('x'))
                    y=int(point.get('y'))
                    if x>maxX:
                        maxX=x
                    if x<minX:
                        minX=x
                    if y>maxY:
                        maxY=y
                    if y<minY:
                        minY=y
                wordBoxes.append( (minX,minY,maxX,maxY) )
            if (len(wordBoxes)==0):
                continue

            if not wordsFromThisPage:
                if not writeGTP:
                    if didI>=len(did):
                        writeGTP=True
                    else:
                        m = re.match(r'(.*\.\w+) (\d+) (\d+) (\d+) (\d+) (.*)',did[didI].strip())
                        didImageFile = m.group(1)
                        if imageFile != didImageFile:
                            writeGTP=True
                        else:
                            didI+=1
                            while didI<len(did):
                                m = re.match(r'(.*\.\w+) (\d+) (\d+) (\d+) (\d+) (.*)',did[didI].strip())
                                if m is not None:
                                    didImageFile = m.group(1)
                                    if imageFile == didImageFile:
                                        didI+=1
                                    else:
                                        break
                                else:
                                    print('parse error on did['+str(didI)+']= '+did[didI].strip())
                                    exit(1)
            wordsFromThisPage=True
            if writeGTP:
                wordBoxes = sorted(wordBoxes, key=lambda box: box[0])
                if len(words)==3 and len(wordBoxes)==4 and text=='to commit it':
                    wordBoxes = wordBoxes[:-1]
                if len(wordBoxes) < len(words):
                    newWords=[]
                    for w in words:
                        if re.match('[&\(\)]',w):
                            continue
                        newWords.append(w)
                    words=newWords


                #print text
                #print words
                #print len(words)
                #print len(wordBoxes)
                if len(words) != len(wordBoxes):
                    if imageFile != curImageFile:
                        orig = cv2.imread(imgDir+imageFile)
                        if orig is None:
                            print('ERROR, couldnt open: '+imgDir+imageFile+imgExt)
                            exit(0)
                        curImageFile=imageFile
                    if firstOff:
                        showControls()
                        firstOff=False
                    words, wordBoxes = segmenter(text,words,wordBoxes,orig)
                todo.append( (words,wordBoxes) )

        for (words, wordBoxes) in todo:
            for wi in range(len(words)):
                if re.match('[&\(\)]',words[wi]) or len(words[wi])==0:
                    continue
                out.write(imageFile+' '+str(wordBoxes[wi][0])+' '+str(wordBoxes[wi][1])+' '+str(wordBoxes[wi][2])+' '+str(wordBoxes[wi][3])+' '+words[wi]+'\n')
        if wordsFromThisPage:
            pagesInSet+=1
            print 'Page '+str(pagesInSet) + ' ('+imageFile+') finished.'

    out.close();
    return pagesInSet

refPt = []
image=None
colorIdx=0
leadColor=[(200, 255, 0), (0, 145, 255), (105, 0, 205)]
color = [(70, 255, 0), (0, 205, 255), (255, 0, 205)]
bimage=None
def clicker(event, x, y, flags, param):
        # grab references to the global variables
        global refPt, image, bimage, colorIdx

        # if the left mouse button was clicked, record the starting
        # (x, y) coordinates and indicate that cropping is being
        # performed
        if event == cv2.EVENT_LBUTTONDOWN:
                image=bimage.copy()
                #if len(refPt)>0:
                refPt[0]=x
                #else:
                #    refPt = [x]
                cv2.rectangle(image, (refPt[0],0), (refPt[-1],image.shape[0]), leadColor[colorIdx], 2)
                cv2.imshow("image", image)

        # check to see if the left mouse button was released
        elif event == cv2.EVENT_RBUTTONDOWN:
                # record the ending (x, y) coordinates and indicate that
                # the cropping operation is finished
                if len(refPt)>0:
                    if len(refPt)>1:
                        refPt.pop()
                    image=bimage.copy()
                    refPt.append(x)

                    # draw a rectangle around the region of interest
                    cv2.rectangle(image, (refPt[0],0), (refPt[1],image.shape[0]), color[colorIdx], 2)
                    cv2.imshow("image", image)

def segmenter(gt, words, wordBoxes, orig):
    global colorIdx, bimage, image, refPt
    newWords=words
    newWordBoxes=[]
    #print((x1,y1,x2,y2))
    redo=True
    while redo: #undo loop
        redo=False
        x1=999999
        y1=999999
        x2=-1
        y2=-1
        for wi in range(len(wordBoxes)):
            if wordBoxes[wi][0]<x1:
                x1=wordBoxes[wi][0]
            if wordBoxes[wi][1]<y1:
                y1=wordBoxes[wi][1]
            if wordBoxes[wi][2]>x2:
                x2=wordBoxes[wi][2]
            if wordBoxes[wi][3]>y2:
                y2=wordBoxes[wi][3]
        image = orig[y1:y2+1, x1:x2+1].copy()
        bimage = orig[y1:y2+1, x1:x2+1].copy()

        lastX=0
        colorIdx=0
        wi=0
        print('Image[!]: '+gt)
        for name in newWords:
            print('CROP: '+name)
            refPt = [lastX]
            cv2.rectangle(image, (refPt[0],0), (refPt[0],image.shape[0]), leadColor[colorIdx], 2)
            cv2.imshow("image", image)
            while True:
                # display the image and wait for a keypress
                key = cv2.waitKey(33) #?? & 0xFF
                if key == 10 and len(refPt)==2: #eneter
                    #toWrite+=filePath+' '+str(x1+refPt[0])+' '+str(y1)+' '+str(x1+refPt[1])+' '+str(y2)+' '+name+'\n'
                    newWordBoxes.append(( (x1+refPt[0]), y1, (x1+refPt[1]), y2));
                    bimage=image.copy()
                    colorIdx = (colorIdx+1)%len(color)
                    lastX=refPt[1]
                    break
                elif key == 65288: #backspace
                    print('[CLEAR]')
                    redo=True
                    break
                elif key == 65379: #insert
                    print("Enter new transcription for whole image. Don't include punctuation/specail marks.")
                    gt=raw_input(':')
                    newWords = gt.split()
                    redo=True
                    break
                elif key == 65365: #page up
                    y1-=5
                    y1 = max(0,y1)
                    #redo=True
                    #if (lastX>=50):
                        #    x1 = max(0,x1+lastX-50)
                    #    lastX=50
                    image = orig[y1:y2, x1:x2].copy()
                    bimage = orig[y1:y2, x1:x2].copy()
                    refPt = [lastX]
                    cv2.rectangle(image, (refPt[0],0), (refPt[0],image.shape[0]), leadColor[colorIdx], 2)
                    cv2.imshow("image", image)
                    #break
                elif key == 65366: #page down
                    y1+=5
                    y1 = min(y2-1,y1)
                    #redo=True
                    #if (lastX>=50):
                    #    x1 = max(0,x1+lastX-50)
                    #    lastX=50
                    image = orig[y1:y2, x1:x2].copy()
                    bimage = orig[y1:y2, x1:x2].copy()
                    refPt = [lastX]
                    cv2.rectangle(image, (refPt[0],0), (refPt[0],image.shape[0]), leadColor[colorIdx], 2)
                    cv2.imshow("image", image)
                    #break
                elif key == 65362: #up ar
                    y2-=5
                    y2 = max(y1+1,y2)
                    #redo=True
                    #if (lastX>=50):
                    #    x1 = max(0,x1+lastX-50)
                    #    lastX=50
                    image = orig[y1:y2, x1:x2].copy()
                    bimage = orig[y1:y2, x1:x2].copy()
                    refPt = [lastX]
                    cv2.rectangle(image, (refPt[0],0), (refPt[0],image.shape[0]), leadColor[colorIdx], 2)
                    cv2.imshow("image", image)
                    #break
                elif key == 65364: #down ar
                    y2+=5
                    y2 = min(orig.shape[0]-1,y2)
                    #redo=True
                    #if (lastX>=50):
                        #    x1 = max(0,x1+lastX-50)
                    #    lastX=50
                    image = orig[y1:y2, x1:x2].copy()
                    bimage = orig[y1:y2, x1:x2].copy()
                    refPt = [lastX]
                    cv2.rectangle(image, (refPt[0],0), (refPt[0],image.shape[0]), leadColor[colorIdx], 2)
                    cv2.imshow("image", image)
                    #break
                elif key == 27: #esc
                    print('esc')
                    
                    exit(0)
                    break
            if redo:
                break

    return newWords, newWordBoxes

cv2.namedWindow("image")
cv2.setMouseCallback("image", clicker)

nTr=makeGTP(trainFile,outTrainFile)
print ('FINISH TRAIN')
nV=makeGTP(validFile,outValidFile)
print ('FINISH VALID')
nTe=makeGTP(testFile,outTestFile)
print ('FINISH TEST')
print(str(nTr)+' pages in train')
print(str(nV)+' pages in valid')
print(str(nTe)+' pages in test')
