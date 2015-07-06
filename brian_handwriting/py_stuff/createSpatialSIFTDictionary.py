import numpy as np
import os
import cv2
import math

"""img = cv2.imread('../E.png',0)
cv2.imshow('image',img)
cv2.waitKey(0)
cv2.destroyAllWindows()"""

def find_center_of_mass(img):
	pixSum=0
	xSum=0
	ySum=0
	for x in range(0,img.shape[0]):
		for y in range(0,img.shape[1]):
			pixSum += img[x,y]
			xSum += x*img[x,y]
			ySum += y*img[x,y]
	
	return (xSum/pixSum,ySum/pixSum)


size=100

#all_kp = np.empty([0,128])
all_des = np.empty([0,128])
sift = cv2.SIFT(0,3,.14,5)
count=0
for i in os.listdir(os.getcwd()):
    
    if (count<1000) and i.endswith(".JPG") and not i.startswith('.'): 
        count+=1
        big_img = cv2.imread(i,cv2.CV_LOAD_IMAGE_GRAYSCALE)
        img = cv2.resize(big_img, (size,size))
        center_of_mass = find_center_of_mass(img)
        
        img = cv2.cvtColor(img,cv2.COLOR_GRAY2RGB)
        cv2.circle(img,center_of_mass,2,(255,0,0))
        cv2.imshow('image',img)
        cv2.waitKey(0)
        continue
        
        #print i
        #img = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
        kp, des = sift.detectAndCompute(img,None)
        #print kp
        #all_kp=np.append(all_kp,kp)
        if (count == 1):
            all_des=des
        else:
            all_des=np.append(all_des,des, axis=0)
        continue
    else:
        continue

"""
#print len(all_des)
#print all_des.shape
#print des.shape
K= 7
c,bestLabels , centriods = cv2.kmeans(all_des,K,(cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER,300,.9),15,cv2.KMEANS_RANDOM_CENTERS)

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
"""
