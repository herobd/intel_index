import os
import sys
out = open(sys.argv[1],'w')
for root, dirs, files in os.walk(sys.argv[2]):
    for d in dirs:
        for root2, dirs2, files2 in os.walk(root+d):
            for f in files2:
                fileN=f.split('.')
                out.write(root2+'/'+f+',images/'+fileN[0]+'.jpg\n')

out.close()
