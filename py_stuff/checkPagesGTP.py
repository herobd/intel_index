#!/bin/python
import random
import sys
import re
import cv2
import math
import numpy as np


if len(sys.argv)<3:
    print 'usage: '+sys.argv[0]+' in.gtp out.gtp [start fim]'
    exit(0)


def getLI(line,cur):
    m = re.match(r'(.*\.\w+) (\d+) (\d+) (\d+) (\d+) (.*)',line)
    if m is not None:
        fim = m.group(1)
        x1 = int(m.group(2))
        y1 = int(m.group(3))
        x2 = int(m.group(4))
        y2 = int(m.group(5))
        label=(m.group(6)) #.split()
        
        #x2=x1+maxWidth-1
        #print(x1,x2)
        ye = min(y2+16,cur.shape[0]-1)
        im = np.copy(cur[y1:ye,:])
        if x1>0:
            im[:,:x1-1,1:3]=0
        if x2<im.shape[1]:
            im[:,x2+1:,1:3]=0
        if ye!=cur.shape[0]-1:
            im[-15:,:,1:3]=0
            im[-15:,:,0]/=2
        return label, im
    return None,None

def approve(linesForPage,cur,maxWidth):
    #cv2.imshow('full',cur)
    apim=None
    aplabel=None
    print ('--------------------------------------')

    black=None
    label,im = getLI(linesForPage[0].strip(),cur)
    if label is not None:
        #print im.shape
        if apim is None:
            apim=im
            aplabel=label
            #black=np.zeros((10,apim.shape[1],3), np.uint8)
            #cv2.imshow('b',black)
            #black=np.zeros((10,apim.shape[1],apim.shape[2])).astype(int)
            #black.fill(255)
        else:
            #apim = np.append(np.append(apim,black,axis=0),im,axis=0)
            aplabel+='\n\n\n'+label
    #cv2.imshow('s',im)
    #cv2.imshow('a',apim)
    if len(linesForPage)>1:
        if len(linesForPage)>2:
            for i in range(3):
                x=random.randint(1,len(linesForPage)-2)
                label,im = getLI(linesForPage[x].strip(),cur)
                if label is not None:
                    #print im.shape
                    if apim is None:
                        apim=im
                        aplabel=label
                    else:
                        #apim=np.append(apim,black,axis=0)
                        apim=np.append(apim,im,axis=0)
                        #apim = np.append(np.append(apim,black,axis=0),im,axis=0)
                        aplabel+='\n\n\n'+label
        label,im = getLI(linesForPage[-1].strip(),cur)
        if label is not None:
            #print im.shape
            if apim is None:
                apim=im
                aplabel=label
            else:
                apim=np.append(apim,im,axis=0)
                aplabel+='\n\n\n'+label
    
    print(aplabel)
    print ('--------------------------------------')
    cv2.imshow('cur',apim)
    while True:
        key = cv2.waitKey()
        if key==10 or key==65363:
            return 1
        elif key==65288 or key==65361:
            return 0
        elif key==27:
            return -1


with open(sys.argv[1],'r') as f:
    gtp = f.read().splitlines()
startfim = ''
if len(sys.argv)>3:
    startfim=sys.argv[3]
    out = open(sys.argv[2],'a')
else:
    out = open(sys.argv[2],'w')
count=0
maxCount=20
curFim=''
cur=None
linesForPage=[]
maxWidth=0
counter=len(gtp)
for line in gtp:
    #print line
    m = re.match(r'(.*\.\w+) (\d+) (\d+) (\d+) (\d+) (.*)',line.strip())
    if m is not None:
        fim = m.group(1)
        x1 = int(m.group(2))
        #y1 = int(m.group(3))
        x2 = int(m.group(4))
        #y2 = int(m.group(5))
        #label=(m.group(6)) #.split()
        if startfim!='':
            if fim==startfim:
                startfim=''
            else:
                continue

        if curFim != fim:
            if len(linesForPage) > 0:
                print('doing '+curFim + '    '+str(counter))
                print('of '+str(len(linesForPage)))
                a = approve(linesForPage,cur,maxWidth)
                if a==-1:
                    out.close()
                    exit(1)
                if a==1:
                    for l in linesForPage:
                        out.write(l+'\n')
                    print('APPROVED '+curFim)
                else:
                    print('DISCARDED '+curFim)

            linesForPage=[]
            maxWidth=0
            curFim=fim
            cur=cv2.imread(fim)
        linesForPage.append(line)
        if x2-x1 +1 > maxWidth:
            maxWidth=x2-x1 +1

   

    else:
        print('parse error on: '+line.strip())
        exit(1)
    counter-=1

if len(linesForPage) > 0:
    a = approve(linesForPage,cur)
    if a==-1:
        out.close()
        exit(1)
    if a==1:
        for l in linesForPage:
            out.write(l)
        print('APPROVED '+curFim)
    else:
        print('DISCARDED '+curFim)

out.close()
