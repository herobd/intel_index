import sys
import re
import cv2
import math
import numpy as np
from scipy import ndimage

def find_center_of_mass(img):
    profile = np.sum(255 - img, axis=1)
    center_of_mass = ndimage.measurements.center_of_mass(profile)[0]
    return center_of_mass

if len(sys.argv)<6:
    print 'usage: '+sys.argv[0]+' in.gtp inDir baselineH out.txt outDir'
    exit(0)

inDir = sys.argv[2]

scale_mu=0.57 #difference between baseline height and std
baseline_height= int(sys.argv[3]) #20

outFile = sys.argv[4]
outDir = sys.argv[5]

with open(sys.argv[1],'r') as f:
    gtp = f.read().splitlines()
count=0
maxCount=20
curFim=''
cur=None
imagesByPageId={}
images=[]
#pageIds={}
curPageId=-1
stds={}
counts={}
for line in gtp:
    #print line
    if count>maxCount:
        break
    m = re.match(r'(.*\.\w+) (\d+) (\d+) (\d+) (\d+) (.*)',line.strip())
    if m is not None:
        found=False
        fim = m.group(1)
        x1 = int(m.group(2))
        y1 = int(m.group(3))
        x2 = int(m.group(4))
        y2 = int(m.group(5))
        label=(m.group(6)) #.split()

        if curFim is not fim:
            curFim=fim
            curPageId+=1
            imagesByPageId[curPageId] =[]
            #pageIds[fim]=curPageId
            cur=cv2.imread(inDir+'/'+fim,0)
            stds[curFim]=0
            counts[curFim]=0
        img = cur[y1:y2+1,x1:x2+1]

        profile = np.sum(255 - img, axis=1)
        center_of_mass = ndimage.measurements.center_of_mass(profile)[0]

        distances = center_of_mass - np.array([float(i) for i in range(0, len(profile))])
        std = (profile * (distances ** 2.0)).sum() / profile.sum()
        std = np.sqrt(std)

        stds[curFim]+=std
        counts[curFim]+=1
        
        imageT = (fim,x1,y1,x2,y2,label)


        imagesByPageId[curPageId].append(imageT)
        images.append(imageT)


    else:
        print('parse error on: '+line.strip())
        exit(1)

        #for word in OoV:
        #print word

for k,v in counts.iteritems():
    stds[k]/=v

imgCount=0
out = open(outFile,'w')
for imageT in images:
    fim,x1,y1,x2,y2,label = imageT
    
    if curFim is not fim:
        curFim=fim
        cur=cv2.imread(fim)
    img = cur[y1:y2+1,x1:x2+1]
    author_avg_std = stds[fim]
    avg_line = author_avg_std / scale_mu
    hpercent = baseline_height / avg_line
    if img.shape[0] > 6 and img.shape[1] > 6:
        img = cv2.resize(img,None,fx=hpercent, fy=hpercent, interpolation = cv2.INTER_CUBIC)
    imgName = "word_"+str(imgCount)+".png"
    imgCount+=1

    cv2.imwrite(outDir+"/"+imgName)
    out.write(imgName+" "+label+"\n")
out.close()
