import sys
from matplotlib import pyplot as plt
import matplotlib.image as mpimg
import numpy as np
class ClickHandler:
    def __init__(self, line, out):
        self.fieldList=["PR_NAME","PR_RELATIONSHIP","PR_SEX","PR_AGE","PR_MARITAL_STATUS","PR_BIRTHPLACE","PR_FTHR_BIRTHPLACE","PR_MTHR_BIRTHPLACE"]
        self.fieldIndex=0
        self.recordIndex=0
        self.boxIndex=0
        self.line = line
        self.out = out
        for f in self.fieldList:
            out.write(f+',')
        out.write('\n')
        #self.xs = list(line.get_xdata())
        #self.ys = list(line.get_ydata())
        self.cid = line.figure.canvas.mpl_connect('button_press_event', self)
        print (str(self.recordIndex)+': '+self.fieldList[self.fieldIndex])

    def __call__(self, event):
        #print('click', event)
        if event.inaxes!=self.line.axes or event.button!=3: return
        #self.xs.append(event.xdata)
        #self.ys.append(event.ydata)
        #self.line.set_data(self.xs, self.ys)
        #self.line.figure.canvas.draw()
        out.write(str(int(event.xdata))+' '+str(int(event.ydata))+' ')
        self.boxIndex+=1
        if self.boxIndex==2:
            self.boxIndex=0
            out.write(',')
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
