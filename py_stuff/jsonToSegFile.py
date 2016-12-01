import json
import re
import cv2

corpusFile = 'ben_cattss_c_corpus.gtp'
segTestFile = 'segment_test.json'
segTrainFile = 'segment_training.json'
outFile = 'segmentation_corpus2.csv'

FH = open(corpusFile)
corpus = FH.read().splitlines()
FH.close()
FH = open(segTestFile)
jsonTest = FH.read()
FH.close()
FH = open(segTrainFile)
jsonTrain = FH.read()
FH.close()

#save images

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
            words.append({'imageFile':imageFile, 'x1':x1, 'y1':y1, 'x2':x2, 'y2':y2, 'page': pageCount})
    return words

def parseJSON(inst):
    #print(inst)
    origFile = inst['ori_image']
    m = re.match(r'.*/(\w+\.jpg)',origFile)
    assert m is not None
    origFile = m.group(1)
    x1 = int(inst['org_bbox'][0])
    y1 = int(inst['org_bbox'][1])
    x2 = int(inst['org_bbox'][2])
    y2 = int(inst['org_bbox'][3])

    scale = inst['scale_factor']

    bounds=[]
    for letter in inst['alignment']:
        start = min(letter[0][0],letter[1][0])
        end = max(letter[0][0],letter[1][0])
        bounds.append({'start':int(x1 + start/scale), 'end':int(x1 + end/scale), 'conf':letter[0][1]})

    return { 'x2':x2, 'y2':y2, 'bounds':bounds, 'gt': inst['gt'], 'imageFile': origFile, 'x1':x1, 'y1':y1}


corpus = parseGTP(corpus)
j = json.loads(jsonTest)
j = j + json.loads(jsonTrain)

final = [None]*len(corpus)
for inst in j:
    w = parseJSON(inst)
    for i in range(len(corpus)):
        w2=corpus[i]
        #print(w)
        #print(w2)
        if w['imageFile']==w2['imageFile'] and w['x1']==w2['x1'] and w['y1']==w2['y1'] and w['x2']==w2['x2'] and w['y2']==w2['y2']:
            w['page']=w2['page']
            #print(i,len(final))
            final[i]=w
            break


out = open(outFile,'w')
out.write('gt,page,x1,y1,x2,y2,start1,end1,conf1,start2,...\n')
curImageFile=''
image=None
for w in final:

    if w['imageFile']!=curImageFile:
        curImageFile=w['imageFile']
        image = cv2.imread('BenthamDatasetR0-Images/Images/Pages/'+curImageFile)

    subImage = image[w['y1']:w['y2']+1,w['x1']:w['x2']+1,:]
    out.write(w['gt']+','+str(w['page'])+','+str(w['x1'])+','+str(w['y1'])+','+str(w['x2'])+','+str(w['y2']))
    c=0
    print('\n'+w['gt'])
    for letter in w['bounds']:
        subImage[:,letter['start']-w['x1'],c%3]=0
        subImage[:,letter['end']-w['x1'],c%3]=0
        subImage[:,letter['start']-w['x1']:1+letter['end']-w['x1'],c%3]=0.5*subImage[:,letter['start']-w['x1']:1+letter['end']-w['x1'],c%3]
        if c%3==0:
            color='yellow'
        elif c%3==1:
            color='magenta'
        else:
            color='cyan'
        print(w['gt'][c]+': '+color)
        print(letter['conf'])
        c+=1
        out.write(','+str(letter['start'])+','+str(letter['end'])+','+str(letter['conf']))
    cv2.imshow('bounds',subImage)
    cv2.waitKey()
    out.write('\n')
out.close
