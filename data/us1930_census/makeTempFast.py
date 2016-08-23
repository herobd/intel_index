import sys
from matplotlib import pyplot as plt
import matplotlib.image as mpimg
import numpy as np
class ClickHandler:
    def __init__(self, line, out):
        self.fieldList=["PR_NAME","PR_RELATIONSHIP","PR_SEX","PR_AGE","PR_MARITAL_STATUS","PR_BIRTHPLACE","PR_FTHR_BIRTHPLACE","PR_MTHR_BIRTHPLACE"]
        self.fpTop=[()]*len(self.fieldList)
        self.fpBot=[()]*len(self.fieldList)
        self.rpLeft=[()]*51
        self.rpRight=[()]*51
        self.i=0
        self.boxIndex=0
        self.on='top'
        self.line = line
        self.out = out
        self.last=()
        for f in self.fieldList:
            out.write(f+',')
        out.write('\n')
        self.cid = line.figure.canvas.mpl_connect('button_press_event', self)
        #print (str(self.recordIndex)+': '+self.fieldList[self.fieldIndex])
        print(self.on,':',self.fieldList[self.i])
    
    def writeFile(self):
        for r in range(50):
            for f in range(len(self.fieldList)):
                interTL = self.intersection(self.fpTop[f][0], self.fpBot[f][0],
                                            self.rpLeft[r], self.rpRight[r])
                interBR = self.intersection(self.fpTop[f][1], self.fpBot[f][1],
                                            self.rpLeft[r+1], self.rpRight[r+1])
                out.write(str(interTL[0])+' '+str(interTL[1])+' '+str(interBR[0])+' '+str(interBR[1])+',')
            out.write('\n')

    def intersection(self,up_p,lo_p,le_p,ri_p):
        p1 = np.array([up_p[0], up_p[1], 1])
        p2 = np.array([lo_p[0], lo_p[1], 1])
        l1 = np.cross(p1,p2)
        p1 = np.array([le_p[0], le_p[1], 1])
        p2 = np.array([ri_p[0], ri_p[1], 1])
        l2 = np.cross(p1,p2)
        ip = np.cross(l1,l2)
        return (ip[0]/ip[2],ip[1]/ip[2])

    def __call__(self, event):
        if event.inaxes!=self.line.axes or event.button!=3: return
        #out.write(str(int(event.xdata))+' '+str(int(event.ydata))+' ')

        self.boxIndex+=1
        if self.on=='top':
            if (self.boxIndex==1):
                self.last=(int(event.xdata),int(event.ydata))
                sys.stdout.write(',')
                sys.stdout.flush()
            else:
                self.fpTop[self.i]=(self.last,(int(event.xdata),int(event.ydata)))
                self.boxIndex=0
                self.i+=1
                if (self.i==len(self.fieldList)):
                    self.i=0
                    self.on='bot'
                print(self.on,':',self.fieldList[self.i])
                
        elif self.on=='bot':
            if (self.boxIndex==1):
                self.last=(int(event.xdata),int(event.ydata))
                sys.stdout.write(',')
                sys.stdout.flush()
            else:
                self.fpBot[self.i]=(self.last,(int(event.xdata),int(event.ydata)))
                self.boxIndex=0
                self.i+=1
                if (self.i==len(self.fieldList)):
                    self.i=0
                    self.on='left'
                print(self.on,':',self.fieldList[self.i])

        elif self.on=='left':
                self.rpLeft[self.i]=(int(event.xdata),int(event.ydata))
                self.boxIndex=0
                self.i+=1
                if (self.i==51):
                    self.i=0
                    self.on='right'
                print(self.on,':',str(self.i))

        elif self.on=='right':
                self.rpRight[self.i]=(int(event.xdata),int(event.ydata))
                self.boxIndex=0
                self.i+=1
                if (self.i==51):
                    self.i=0
                    self.on='done'
                    print ('DONE')
                    self.writeFile()
                else:
                    print(self.on,':',str(self.i))


img=mpimg.imread(sys.argv[1])
out=open(sys.argv[2],'w')
fig = plt.figure()
ax = fig.add_subplot(111)
ax.set_title('click')
#line, = ax.plot([0], [0])  # empty line
im = ax.imshow(img)
cl = ClickHandler(im,out)

plt.show()
out.close()
