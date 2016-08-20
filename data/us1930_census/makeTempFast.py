import sys
from matplotlib import pyplot as plt
import matplotlib.image as mpimg
import numpy as np
class ClickHandler:
    def __init__(self, line, out):
        self.fieldList=["PR_NAME","PR_RELATIONSHIP","PR_SEX","PR_AGE","PR_MARITAL_STATUS","PR_BIRTHPLACE","PR_FTHR_BIRTHPLACE","PR_MTHR_BIRTHPLACE"]
        self.fpTop=[()]*len(self.fieldList)
        self.fpBot=[()]*len(self.fieldList)
        self.rpLeft=[()]*50
        self.rpRight=[()]*50
        self.i=0
        self.boxIndex=0
        self.on='top'
        self.line = line
        self.out = out
        self.last;
        for f in self.fieldList:
            out.write(f+',')
        out.write('\n')
        self.cid = line.figure.canvas.mpl_connect('button_press_event', self)
        print (str(self.recordIndex)+': '+self.fieldList[self.fieldIndex])

    def __call__(self, event):
        if event.inaxes!=self.line.axes or event.button!=3: return
        #out.write(str(int(event.xdata))+' '+str(int(event.ydata))+' ')

        self.boxIndex+=1
        if self.on=='top':
            if (self.boxIndex==2):
                self.last=(int(event.xdata),int(event.ydata))
            else:
                fpTop[self.i]=(self.last,(int(event.xdata),int(event.ydata)))
                self.boxIndex=0
                self.i+=1
                if (self.i==len(self.fieldList)):
                    self.i=0
                    self.on='bot'
                    print(self.on,':',self.fieldList[self.i])
                


        self.i+=1
        if self.boxIndex==2:
            self.boxIndex=0
            self.fieldIndex+=1
            if self.fieldIndex==len(self.fieldList):
                out.write('\n')
                self.fieldIndex=0
                self.recordIndex+=1
            if self.recordIndex==50:
                print ('--DONE--')
            else:
                print (str(self.recordIndex)+': '+self.fieldList[self.fieldIndex])

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
