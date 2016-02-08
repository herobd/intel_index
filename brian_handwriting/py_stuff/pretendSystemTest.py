#!/usr/bin/python

import sys
import random
import re

#test = [1,2,3,4,5]
#for i in test[:-1]:
#	print i

if (len(sys.argv)==1 or len(sys.argv)>14):
	print 'Usage: python pretendSystemTest [dictionaryfile] [ngramfile] [recall rate] [confidence range: 0-(2?)] [corpus file] [out file] (optionals: [stats out file] [stop threshold] [leftover words file] [ngramApprovalListSize] [transcribeListSize] [pruneNgrams] [accuracy])'
	sys.exit()

#path to dictionary file, expectsd word on each newline
pathDic=sys.argv[1]

#path to ngram list, expects ngram in each newline
pathNgram=sys.argv[2]

#recall rate [0.0 - 1.0]
recallRate=float(sys.argv[3])

#pad is the confidence range, 0 being perfect knowledge
#2 for both sides or 1 for either side (odd values cannot be even on both sides)
pad=int(sys.argv[4]) 

#path to the corpus we are "transcribing", expects word on each newline
pathCorpus=sys.argv[5]

#path to output file
pathOut=sys.argv[6]

pathStats=None
if (len(sys.argv)>7):
	pathStats=sys.argv[7]

stopThresh=.9
pathLeftoverWords=None
if (len(sys.argv)>8):
	stopThresh=float(sys.argv[8])
if (len(sys.argv)>9):
	pathLeftoverWords=sys.argv[9]

ngramApprovalListSize=6
transcribeListSize=10
accuracy=.5
pruneNgrams=.05
if (len(sys.argv)>10):
	ngramApprovalListSize=int(sys.argv[10])
if (len(sys.argv)>11):
	transcribeListSize=int(sys.argv[11])
if (len(sys.argv)>12):
	pruneNgrams=float(sys.argv[12])
if (len(sys.argv)>13):
	accuracy=float(sys.argv[13])

countThresh=250

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
		



fDic = open(pathDic, 'r')
dictionary = []
for line in fDic.xreadlines():
   wordFixed = re.sub(r'\W','',line.strip()).lower();
   dictionary.append(wordFixed)
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

OoV=0 #out of vocabulary

fC = open(pathCorpus, 'r')
words = []
for line in fC.xreadlines():
   wordfixed=re.sub(r'\W','',line.strip()).lower()
   words.append(Word(wordfixed))
   found=False
   for d in dictionary:
	if d == wordfixed:
		found=True
		break
   if not found:
	#print wordfixed
  	OoV+=1
fC.close()

stats=[]

fO = open(pathOut, 'w')

startNum = len(words)
print 'startNum='+str(startNum)
print 'out of vocabulary = ' + str(OoV)
count=0

totalNumUserApprovalTasks=0
totalNumNgramsNeedingApproval=0

fO.write('iterations,completion,completion(no OoV)\n')
while (len(words)/float(startNum) > 1.0-stopThresh and count<countThresh):
	for ngram in ngrams:
		if (len(words)/float(startNum) <= 1.0-stopThresh or count>=countThresh):
			break
		#effectedWords = []
		thisStat = [0,0,0,0,0,0]
		numSpotted=0
		for w in words:
			#print 'looking for '+ngram+' in '+w.text
			for loc in [m.start() for m in re.finditer(ngram, w.text)]:
				if random.random() < recallRate:
					#loc = w.text.find(ngram)
					w.spot(ngram,loc)
					w.effected=True
					numSpotted+=1
					#print ngram + ' found in ' + w.text
		
		
		#lookup effected words and remove if 10 or less possibilities
		words = [w for w in words if ((not w.effected) or dicLookup(w)>transcribeListSize)]
		
		
		
		numNeedingApproval = int(numSpotted*(1/accuracy)*(1-pruneNgrams))
		totalNumNgramsNeedingApproval += numNeedingApproval
		totalNumUserApprovalTasks += numNeedingApproval/ngramApprovalListSize;
		
		thisStat[5] = startNum - len(words)
		for w in words:
			w.effected=False
			lettersSpotted=0
			for spotting in w.spotted:
				lettersSpotted += len(spotting[0])
			ratioSpotted = float(lettersSpotted)/float(len(w.text))
			if (ratioSpotted<.2):
				thisStat[0]+=1
			elif (ratioSpotted<.4):
				thisStat[1]+=1
			elif (ratioSpotted<.6):
				thisStat[2]+=1
			elif (ratioSpotted<.8):
				thisStat[3]+=1
			else:
				thisStat[4]+=1
		stats.append(thisStat)

		done = (float(startNum)-len(words))/float(startNum)
		doneNoOoV = (float(startNum-OoV)-(len(words)-OoV))/float(startNum-OoV)
		if count%25==0:
			print 'spot ' +str(count)+ ': ' +str(done*100)+'% transcribed ('+str(doneNoOoV*100)+ '% w/o OoV)'
			
		fO.write(str(count)+','+str(done*100)+','+str(doneNoOoV*100)+'\n')
		
		count += 1

print 'num words not transcribed = '+str(len(words))
fO.write('totalNumUserApprovalTasks = '+str(totalNumUserApprovalTasks)+'\n')
print 'totalNumUserApprovalTasks = '+str(totalNumUserApprovalTasks)
fO.write('totalNumNgramsNeedingApproval = '+str(totalNumNgramsNeedingApproval)+'\n')
print 'totalNumNgramsNeedingApproval = '+str(totalNumNgramsNeedingApproval)
fO.close()

taskSecVer=5
taskSecTran=5
taskSecLeft=10
numVerTran = 2
numVerLeft = 2
hoursTask = (totalNumUserApprovalTasks*taskSecVer+((startNum-len(words))*numVerTran)*taskSecTran)/(60.0*60.0)
hoursLeft = (len(words)*taskSecLeft*numVerLeft)/(60.0*60.0)
#print 'estimated hours (triple approve) = '+str(((totalNumUserApprovalTasks+(startNum-len(words))*3)*taskSec+len(words)*taskSec*(2*2))/(60.0*60.0))
print 'estimated hours (double approve) = '+str(hoursTask+hoursLeft)
print 'estimated hours tasks = '+str(hoursTask)
print 'estimated hours leftover = '+str(hoursLeft)

if pathStats is not None:
	fO = open(pathStats,'w')
	for stat in stats:
		#print stat
		for num in stat[:-1]:
			fO.write(str(num)+',')
		fO.write(str(stat[-1])+'\n')
	fO.close()

if pathLeftoverWords is not None:
	fO = open(pathLeftoverWords,'w')
	for w in words:
		fO.write(w.text + ': ')
		for spotting in w.spotted[:-1]:
			fO.write(spotting[0] + ', ')
		if (len(w.spotted)>0):
			fO.write(w.spotted[-1][0] + '\n')
		else:
			fO.write('\n')

	fO.close()

