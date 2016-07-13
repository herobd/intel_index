function data = prepare_data_learning_subword(opts,data)
% Splits the words in the different subsets (train, validation and test)

% opts.fileSets contains the indexes of the subset that each word belongs
if strcmp(opts.dataset,'IAM_lines')
  numTr = 41710;
  rest = 11344;
  if (numTr+rest == size(data.words,2))
    data.idxTrain = zeros(numTr+rest,1)
    data.idxTrain(1:30000)=1;
    data.idxValidation = zeros(numTr+rest,1)
    data.idxValidation(30001:numTr)=1;
    data.idxTest = zeros(numTr+rest,1)
    data.idxTest(numTr+1:end)=1;
    data.idxTrain=logical(data.idxTrain);
    data.idxValidation=logical(data.idxValidation);
    data.idxTest=logical(data.idxTest);
  else
      disp([ 'error, size off ' num2str(size(data.words,1)) '    and in case ' num2str(size(data.words,2))]);
    exit
  end
else
  load(opts.fileSets,'idxTrain','idxValidation','idxTest');
  data.idxTrain = idxTrain;
  data.idxValidation = idxValidation;
  data.idxTest = idxTest;
end
% Words, labels, PHOCS and classes indexes are splitted in the different
% subsets according to the indexes
data.wordsTr = data.words(data.idxTrain);
data.numWTr = length(data.wordsTr);
data.wordsVa = data.words(data.idxValidation);
data.numWVa = length(data.wordsVa);
data.wordsTe = data.words(data.idxTest);
data.numWTe = length(data.wordsTe);

data.labelsTr = {data.wordsTr(:).gttext};
data.labelsVa = {data.wordsVa(:).gttext};
data.labelsTe = {data.wordsTe(:).gttext};
data.labelsQu_subword = {data.words_subword(:).gttext};

data.wordClsTr = [data.wordsTr(:).class];
data.wordClsVa = [data.wordsVa(:).class];
data.wordClsTe = [data.wordsTe(:).class];

data.wordClsTr_subword = {data.wordsTr(:).class_subword};
data.wordClsVa_subword = {data.wordsVa(:).class_subword};
data.wordClsTe_subword = {data.wordsTe(:).class_subword};
data.wordClsQu_subword = {data.words_subword(:).class_subword};

data.phocsTr = data.phocs(:,data.idxTrain);
data.phocsVa = data.phocs(:,data.idxValidation);
data.phocsTe = data.phocs(:,data.idxTest);
data.phocsQu_subword = data.phocs_subword(:,:);

i=0
sumAll=0
sum20=0
sum80=0
for ngram = opts.ngrams
    ngramOccCount=0;
    r=regexpi(data.labelsTr,ngram);
    for rr = r
        if size(rr{1},1)>0
            ngramOccCount=ngramOccCount+1;
        end
    end
    disp([ngram ' (t): ' num2str(ngramOccCount)]);
    sumAll = sumAll+ngramOccCount;
    if i<20
        sum20 = sum20 +ngramOccCount;
    else
        sum80 = sum80 +ngramOccCount;
    end
    
    i=i+1;
end
disp(['avg all: ' num2str(sumAll/100.0)]);
disp(['avg 20: ' num2str(sum20/20.0)]);
disp(['avg 80: ' num2str(sum80/80.0)]);
end
