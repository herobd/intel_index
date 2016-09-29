#!/usr/bin/python
import os
import sys
import re
from random import shuffle

#ngrams = ['th', 'he', 'er', 'an', 're', 'on', 'at', 'or', 'es', 'en', 'te', 'nd', 'ed', 'ar', 'to', 'ti', 'st', 'ng', 'nt', 'it']
ngrams = ['the', 'and', 'ing', 'ion', 'ent', 'her', 'for', 'hat', 'his', 'tha']

locations={};
for ngram in ngrams:
	locations[ngram]=[];
filtered_queries=sys.argv[1];


f = open(filtered_queries, 'r')
i=1;
for line in f.xreadlines():
	#print line;
	match = re.search(r"(?:\S+) (?:\S+) (?:\S+) (?:\S+) (?:\S+) (\S+) (?:\S+)",line)
	#match = re.search(r"(?:.*) (?:.*) (?:.*) (?:.*) (?:.*) (.*) (?:.*)",line)
	text = match.group(1);
	
	#text=text[1:]

	for ngram in ngrams:
		if ngram in text.lower():
			locations[ngram].append('wordimg_'+str(i));
	i+=1;
f.close()

for ngram in ngrams:
	shuffle(locations[ngram]);
        		
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
		
