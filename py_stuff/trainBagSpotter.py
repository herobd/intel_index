
bagged = []

for i in os.listdir(os.getcwd()):
	if i.endswith(".JPG") and not i.startswith('.'): 
        img = cv2.imread(i,cv2.CV_LOAD_IMAGE_GRAYSCALE)
        #print i
        #img = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
        kp, des = sift.detectAndCompute(img,None)
        #print kp
        #all_kp=np.append(all_kp,kp)
        all_des=np.append(all_des,des)
        continue
    else:
        continue
