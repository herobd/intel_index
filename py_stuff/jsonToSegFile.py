import json
import re

corpusFile = 'ben_cattss_c_corpus.gtp'
segTestFile = 'segment_test.json'
segTrainFile = 'segment_training.json'
outFile = 'segmentation_corpus.csv'

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
pageCount=0

def parseGTP(gtp):
    global count
    words=[]
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
        bounds.append({'start':(x1 + letter[0][0]/scale), 'end':(x1 + letter[1][0]/scale), 'conf':letter[0][1]})

    return { 'x2':x2, 'y2':y2, 'bounds':bounds, 'gt': inst['gt'], 'imageFile': origFile, 'x1':x1, 'y1':y1}


corpus = parseGTP(corpus)
j = json.loads(jsonTest)
j = j + json.loads(jsonTrain)

final = []*len(corpus)
for inst in j:
    w = parseJSON(inst)
    for i in range(len(corpus)):
        w2=corpus[i]
        #print(w)
        #print(w2)
        if w['imageFile']==w2['imageFile'] and w['x1']==w2['x1'] and w['y1']==w2['y1'] and w['x2']==w2['x2'] and w['y2']==w2['y2']:
            w.page=w2.page
            final[i]=w
            break


out = open(outFile,'w')
out.write('gt,page,x1,y1,x2,y2,start1,end1,conf1,start2,...')
for w in final:
    out.write(w.gt+','+w.page+','+w.x1+','+w.y1+','+w.x2+','+w.y2)
    for letter in w.bounds:
        out.write(','+letter.start+','+letter.end+','+letter.conf)
    out.write('\n')
out.close
