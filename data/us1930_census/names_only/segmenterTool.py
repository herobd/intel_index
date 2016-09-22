import cv2
import sys
import re

print('CONTROLS:')
print('* set left border:                left-click')
print('* set right border:               right-click')
print('* confirm segmentaion:            enter')
print('* undo segmentaions:              backspace')
print('* change transcription for image: insert')
print('* exit:                           esc')
namesFileName='names.gtp'
outGTPName='seg_names.gtp'

refPt = []
image=None

colorIdx=0
leadColor=[(200, 255, 0), (0, 145, 255), (105, 0, 205)]
color = [(70, 255, 0), (0, 205, 255), (255, 0, 205)]

def clicker(event, x, y, flags, param):
        # grab references to the global variables
        global refPt, image
 
        # if the left mouse button was clicked, record the starting
        # (x, y) coordinates and indicate that cropping is being
        # performed
        if event == cv2.EVENT_LBUTTONDOWN:
                #if len(refPt)>0:
                image=bimage.copy()
                refPt = [x]
                cv2.rectangle(image, (refPt[0],0), (refPt[0],image.shape[0]), leadColor[colorIdx], 2)
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


nameGTP = open(namesFileName,'r')
nameGTPLines = nameGTP.read().splitlines() 
nameGTP.close()
writeSeg=False
try:
    segGTP = open(outGTPName,'r')
    segGTPLines = segGTP.read().splitlines() 
    segGTPLen = len(segGTPLines)
    segGTP.close()
    if segGTPLen==0:
        writeSeg=True
except IOError:
    writeSeg=True
segGTP = open(outGTPName,'a')
segI = 0
curFilePath=''

cv2.namedWindow("image")
cv2.setMouseCallback("image", clicker)

for i in range(len(nameGTPLines)):
    m = re.match(r'(.*\.\w+) (\d+) (\d+) (\d+) (\d+) (.*)',nameGTPLines[i].strip())
    if m:
        filePath = m.group(1)
        x1 = int(m.group(2))
        y1 = int(m.group(3))
        x2 = int(m.group(4))
        y2 = int(m.group(5))
        gt = m.group(6)

        if gt[0]=='-':
            gt = gt[1:]
        names = gt.split()

        if not writeSeg:
            if segI>=segGTPLen:
                #print('on: '+str(segI)+'>'+str(segGTPLen))
                writeSeg=True
            else:
                m = re.match(r'(.*\.\w+) (\d+) (\d+) (\d+) (\d+) (.*)',segGTPLines[segI].strip())
                segY1 = int(m.group(3))
                if segY1 != y1:
                    #print('on: '+segFilePath+'!='+ filePath)
                    writeSeg=True
                else:
                    segI+=1
                    while segI<segGTPLen:
                        m = re.match(r'(.*\.\w+) (\d+) (\d+) (\d+) (\d+) (.*)',segGTPLines[segI].strip())
                        segY1 = int(m.group(3))
                        if segY1 == y1:
                            segI+=1
                        else:
                            break
            
        #if segI>=segGTPLen:
        if writeSeg:
            if filePath != curFilePath:
                orig = cv2.imread(filePath)
                curFilePath=filePath

            redo=True
            while redo: #undo loop
                redo=False
                image = orig[y1:y2, x1:x2].copy()
                bimage = orig[y1:y2, x1:x2].copy()
                
                toWrite=''
                lastX=0
                colorIdx=0
                for name in names:
                    print('CROP: '+name)
                    refPt = [lastX]
                    cv2.rectangle(image, (refPt[0],0), (refPt[0],image.shape[0]), leadColor[colorIdx], 2)
                    cv2.imshow("image", image)
                    while True:
                        # display the image and wait for a keypress
                        key = cv2.waitKey(33) #?? & 0xFF
                        if key == 10 and len(refPt)==2: #eneter
                            toWrite+=filePath+' '+str(x1+refPt[0])+' '+str(y1)+' '+str(x1+refPt[1])+' '+str(y2)+' '+name+'\n'
                            bimage=image.copy()
                            colorIdx = (colorIdx+1)%len(color)
                            lastX=refPt[1]
                            break
                        elif key == 65288: #backspace
                            print('CLEAR')
                            redo=True
                            break
                        elif key == 65379: #insert
                            print("Enter new transcription for whole image. Don't include punctuation/specail marks.")
                            gt=raw_input(':')
                            names = gt.split()
                            redo=True
                            break
                        elif key == 27: #esc
                            #segGTP.close()
                            toWrite=''
                            print('esc '+str(i))
                            segGTP.close()
                            exit(0)
                            break
                    if redo:
                        break
                
            #write all the names
            segGTP.write(toWrite)
            segI+=len(names)


segGTP.close()
        


