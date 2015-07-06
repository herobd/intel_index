#!/usr/bin/python

import sys
import random
import re

pathDic=sys.argv[1]
pathNgram=sys.argv[2]
recallRate=float(sys.argv[3])
pathCorpus=sys.argv[4]
pathOut=sys.argv[5]

class Word:
	def __init__(self,text):
		self.text=text
		self.spotted=[]
		self.effected=False
		#self.transcribed=False
	def spot(self,ngram,loc):
		i=0
		while(i<len(self.spotted) and self.spotted[i][1]<loc):
			i+=1
		if i>=len(self.spotted) or self.spotted[i][1] != loc:
			self.spotted.insert(i,(ngram,loc))
		
pad=2 # or 1 for either side (not both)


fDic = open(pathDic, 'r')
dictionary = []
for line in fDic.xreadlines():
   dictionary.append(line.strip().lower())
fDic.close()

#get capitalized versions
#fDic = open(pathDic, 'r')
#for line in fDic.xreadlines():
#   dictionary.append(line.strip().capitalize)
#fDic.close()
		
def dicLookup(w):
	exp = '\\A'
	curPos=0
	for (ngram,pos) in w.spotted:
		difPos = pos-curPos
		if pad%2==0:
			exp += '[a-z]{'+str(max(0,difPos-pad/2))+','+str(difPos+pad/2)+'}'
		else:
			side=random.randint(0,1)
			exp += '[a-z]{'+ str(max(0,difPos-pad/2 + side-1)) +','+str(difPos+pad/2 + side) +'}'
		exp += ngram
		curPos = pos + len(ngram)
	toFill = len(w.text)-(curPos)
	if pad%2==0:
		exp += '[a-z]{'+str(max(0,toFill-pad/2))+','+str(toFill+pad/2)+'}'
	else:
		side=random.randint(0,1)
		exp += '[a-z]{'+ str(max(0,toFill-pad/2 + side-1)) +','+str(toFill+pad/2 + side) +'}'
	exp += '\\Z'
	#print exp
	find = re.compile(exp,re.I)
	matches=0
	for d in dictionary:
		if find.match(d):
			matches += 1
	return matches


#test = Word('word')
#test.spot('ord',1)
#test.spot('en',3)
#print dicLookup(test)

#exit()###end test





#precisionRate=X we assume perfect precition because of user correction

#ngrams = ['th', 'he', 'er', 'an', 're', 'on', 'at', 'or', 'es', 'en', 'te', 'nd', 'ed', 'ar', 'to', 'ti', 'st', 'ng', 'nt', 'it']
#ngrams = ['the', 'and', 'ing', 'ion', 'ent', 'her', 'for', 'hat', 'his', 'tha']

fN = open(pathNgram, 'r')
ngrams = []
for line in fN.xreadlines():
   ngrams.append(line.strip().lower())
fN.close()

fC = open(pathCorpus, 'r')
words = []
for line in fC.xreadlines():
   words.append(Word(re.sub(r'\W','',line.strip()).lower()))
fC.close()

fO = open(pathOut, 'w')

startNum = len(words)
print 'startNum='+str(startNum)
count=0
while (len(words)/float(startNum) > .1):
	for ngram in ngrams:
		#effectedWords = []
		for w in words:
			#print 'looking for '+ngram+' in '+w.text
			for loc in [m.start() for m in re.finditer(ngram, w.text)]:
				if random.random() < recallRate:
					#loc = w.text.find(ngram)
					w.spot(ngram,loc)
					w.effected=True
					#print ngram + ' found in ' + w.text
		
		words = [w for w in words if ((not w.effected) or dicLookup(w)>10)]
		for w in words:
			w.effected=False
		done = (float(startNum)-len(words))/float(startNum)
		print 'spot ' +str(count)+ ': ' +str(done*100)+'% transcribed'
		fO.write(str(count)+','+str(done*100)+'\n')
		
		count += 1
			
fO.close()
