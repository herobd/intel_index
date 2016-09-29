import sys
import re

if len(sys.argv)<4:
    print 'usage: '+sys.argv[0]+' names.csv freqThresh out.txt'
    exit(0)


with open(sys.argv[1],'r') as f:
    names = f.read().splitlines()
out = open(sys.argv[3],'w')
freqThresh = int(sys.argv[2])

for line in names:
    #UT,A,9740,18416,162257,190413
    #m = re.match(r'(.*) (\d+) (\d+) (\d+) (\d+) (.*)',line.strip())
    #if m is not None:
        m=line.strip().lower().split(',')
        name = m[1]
        freq = int(m[5])
        if freq>=freqThresh:
            out.write(name+'\n')

out.close()
