import sys
import re
import cv2
import math

if len(sys.argv)<7:
    print 'usage: '+sys.argv[0]+' in.gtp inDir minH maxH out.txt outDir'
    exit(0)

inDir = sys.argv[2]

minH = int(sys.argv[3]) #30
maxH = int(sys.argv[4]) #200

outFile = sys.argv[5]
outDir = sys.argv[6]

with open(sys.argv[1],'r') as f:
    gtp = f.read().splitlines()
count=0
curFim=''
cur=None
imagesByPageId={}
images=[]
#pageIds={}
curPageId=-1

out = open(outFile,'w')
for line in gtp:
    #print line
    #if count>maxCount:
    #    break
    m = re.match(r'(.*\.\w+) (\d+) (\d+) (\d+) (\d+) (.*)',line.strip())
    if m is not None:
        found=False
        fim = m.group(1)
        x1 = int(m.group(2))
        y1 = int(m.group(3))
        x2 = int(m.group(4))
        y2 = int(m.group(5))
        label=(m.group(6)) #.split()

        #if count==20 or count==28 or count==36 or count==43 or count==59 or count==76 or count==103 or count==139 or count==176 or count==180 or count==188 or count==192 or count==209 or count==293:
        #    print('err h: '+str(1+y2-y1))
       

        if curFim is not fim:
            curFim=fim
            curPageId+=1
            #pageIds[fim]=curPageId
            cur=cv2.imread(inDir+'/'+fim,0)

        minHDif = minH-(1+ y2-y1)
        if minHDif>0:
            y1 = int(max(0, y1-math.floor(minHDif/2)))
            y2 = int(min(cur.shape[0]-1,y2+math.ceil(minHDif/2)))

        img = cur[y1:y2+1,x1:x2+1]

        if (1+y2-y1)>maxH:
            ratio = maxH/(1.0+y2-y1)
            img = cv2.resize(img,None,fx=ratio, fy=ratio, interpolation = cv2.INTER_CUBIC)

        imgName = "word_"+str(count)+".png"
        count+=1

        cv2.imwrite(outDir+"/"+imgName,img)
        out.write(imgName+" "+label+"\n")


    else:
        print('parse error on: '+line.strip())
        exit(1)

out.close()
