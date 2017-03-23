import sys
import re
import cv2
import math
import numpy as np
from scipy import ndimage
import matplotlib.pyplot as plt 

def find_center_of_mass(img):
    profile = np.sum(255 - img, axis=1)
    center_of_mass = ndimage.measurements.center_of_mass(profile)[0]
    return center_of_mass

if len(sys.argv)<6:
    print 'usage: '+sys.argv[0]+' in.gtp inDir baselineH out.txt outDir [bigger]'
    exit(0)

inDir = sys.argv[2]

scale_mu=0.57 #difference between baseline height and std
baseline_height= int(sys.argv[3]) #20

outFile = sys.argv[4]
outDir = sys.argv[5]

bigger = len(sys.argv)>6
if bigger:
    print("bigger")

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
from collections import defaultdict
stds_list = defaultdict(list)

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
        count+=1
       

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

        stds_list[curFim].append(std)
        stds[curFim]+=std
        counts[curFim]+=1
        
        imageT = (fim,x1,y1,x2,y2,label)


        imagesByPageId[curPageId].append(imageT)
        images.append(imageT)


    else:
        print('parse error on: '+line.strip())
        exit(1)

    #if len(stds)>10:
    #    break

        #for word in OoV:
        #print word

stdsLis=[]
for k,v in counts.iteritems():
    stds[k]/=0.0+v
    stdsLis.append(stds[k])
    #print(len(stds_list[k]))
    plt.hist(stds_list[k])
    #plt.show()
    plt.savefig('hist_'+k+'.png')
    plt.clf()

plt.hist(stdsLis)
plt.savefig('hist_pages.png')
plt.clf()

imgCount=0
minSize=99999999
out = open(outFile,'w')
for imageT in images:
    fim,x1,y1,x2,y2,label = imageT
    
    if curFim is not fim:
        curFim=fim
        cur=cv2.imread(inDir+'/'+fim,0)
    img = cur[y1:y2+1,x1:x2+1]
    author_avg_std = stds[fim]
    avg_line = author_avg_std / scale_mu
    hpercent = baseline_height / avg_line
    if bigger:
        hpercent *= 2
    if (img.shape[0] > 6 and img.shape[1] > 6) or bigger:
        while 1:
            if hpercent==0 or img.shape[0]==0 or img.shape[1]==0:
                print(imageT)
                print(x1,y1,x2,y2)
                print(hpercent,img.shape[0],img.shape[1])
            img = cv2.resize(img,None,fx=hpercent, fy=hpercent, interpolation = cv2.INTER_CUBIC)
            if img.shape[0] < minSize:
                minSize=img.shape[0]
            redo=False
            if img.shape[0] < 40:
                y1 = max(0,y1-10)
                y2 = min(cur.shape[0]-1,y2+10)
                redo=True
                #cv2.imwrite(outDir+"/error"+str(imgCount)+".png",img)
                #print("error "+str(imgCount))
                #continue
            if img.shape[1] < 40:
                x1 = max(0,x1-10)
                x2 = min(cur.shape[1]-1,x2+10)
                redo=True
            if redo:
                img = cur[y1:y2+1,x1:x2+1]
            else:
                break
        #    cv2.imshow('small img',img)
        #    cv2.waitKey()
    imgName = "word_"+str(imgCount)+".png"
    imgCount+=1

    cv2.imwrite(outDir+"/"+imgName,img)
    out.write(imgName+" "+label+"\n")
out.close()
print("min size: "+str(minSize))
