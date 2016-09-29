import sys
import re
import cv2
import math
import numpy as np

if len(sys.argv)<3:
    print 'usage: '+sys.argv[0]+' in.gtp lexicon.txt'
    exit(0)


with open(sys.argv[1],'r') as f:
    gtp = f.read().splitlines()
with open(sys.argv[2],'r') as f:
    lexicon = f.read().splitlines()

for i in range(len(lexicon)):
    lexicon[i] = lexicon[i].strip().lower()

maxCount=10000
match=0
count=0.0
#OoV=[]
for line in gtp:
    if count>maxCount:
        break
    m = re.match(r'(.*\.\w+) (\d+) (\d+) (\d+) (\d+) (.*)',line.strip())
    if m is not None:
        found=False
        labels=(m.group(6)).split()

        for label in labels:
            if label=='-':
                continue
            count+=1
            for word in lexicon:
                if word == label.lower():
                    match+=1
                    found=True
                    break
            #if not found:
            #    print label


    else:
        print('parse error on: '+line.strip())
        exit(1)

        #for word in OoV:
        #print word

print 'In vocab: '+str(100*match/count)+'%'
print 'Out of vocab: '+str(100*(count-match)/count)+'%'
