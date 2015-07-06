#!/usr/bin/python
import os
import sys

#ngrams = ['th', 'he', 'er', 'an', 're', 'on', 'at', 'or', 'es', 'en', 'te', 'nd', 'ed', 'ar', 'to', 'ti', 'st', 'ng', 'nt', 'it']
ngrams = ['the', 'and', 'ing', 'ion', 'ent', 'her', 'for', 'hat', 'his', 'tha']

locations={};
for ngram in ngrams:
	locations[ngram]=[];
dirr=sys.argv[1];


for i in os.listdir(dirr):
    
    if (i.startswith('w_') and i.endswith(".pgm")): 
        f = open(dirr+i, 'r')
        f.readline()
        f.readline()
        text=f.readline()
        text=text[1:]
        
        for ngram in ngrams:
        	if ngram in text:
        		locations[ngram].append(i);
        
        f.close()
        		
fo = open(sys.argv[2],'w')
for ngram in ngrams:
	fo.write(ngram+'\n')
	fo.write(','.join(locations[ngram]) +'\n');
fo.close()

fo = open(sys.argv[3],'w')
for ngram in ngrams:
	fo.write(ngram+'\n')
	fo.write(','.join(locations[ngram][0:5]) +'\n');
fo.close()
		
