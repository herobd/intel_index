import numpy as np
import os
import cv2
import math

"""img = cv2.imread('../E.png',0)
cv2.imshow('image',img)
cv2.waitKey(0)
cv2.destroyAllWindows()"""

#all_kp = np.empty([0,128])
all_des = np.empty([0,128])
#detector = cv2.SIFT(0,3,.04,5)
detector = cv2.SURF(10000,2,4,True,True)
count=0;
for i in os.listdir(os.getcwd()):
    
    if (count<2000) and i.endswith(".JPG") and not i.startswith('.'): 
        count+=1
        img = cv2.imread(i,cv2.CV_LOAD_IMAGE_GRAYSCALE)
        cv2.resize(img, (0,0),dst=img,fx=2,fy=2)
        toss, img = cv2.threshold(img,120.0,255,cv2.THRESH_BINARY)
        #img = cv2.resize(big_img, (big_img.shape[0]/2,big_img.shape[1]/2))
        #print i
        #img = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
        kp, des = detector.detectAndCompute(img,None)
        
        #for index in range(len(kp)-1,-1):
        #	#if kp[index].size < 4:
        #	if kp[index].size > 10:
        #		del des[index]
        
        if (count == 1):
            all_des=des
        else:
            all_des=np.append(all_des,des, axis=0)
        continue
    else:
        continue

#print len(all_des)
#print all_des.shape
#print des.shape
K= 4000
c,bestLabels , centriods = cv2.kmeans(all_des,K,(cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER,1200,.9),20,cv2.KMEANS_RANDOM_CENTERS)

counter = [0] * K
for label in bestLabels:
	counter[label]+=1

#np.set_printoptions(precision=5, edgeitems=200, suppress=True)
#print centriods
for i in range(0,K):
	line = str(centriods[i,0])
	
	for f in range(1,128):
		line +=","+str(centriods[i,f])
	line +=","+str(math.log((len(all_des)*1.0)/counter[i]))
	print line
