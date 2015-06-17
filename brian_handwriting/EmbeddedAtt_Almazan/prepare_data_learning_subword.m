function data = prepare_data_learning_subword(opts,data)
% Splits the words in the different subsets (train, validation and test)

% opts.fileSets contains the indexes of the subset that each word belongs
load(opts.fileSets,'idxTrain','idxValidation','idxTest');
data.idxTrain = idxTrain;
data.idxValidation = idxValidation;
data.idxTest = idxTest;

% Words, labels, PHOCS and classes indexes are splitted in the different
% subsets according to the indexes
data.wordsTr = data.words(idxTrain);
data.numWTr = length(data.wordsTr);
data.wordsVa = data.words(idxValidation);
data.numWVa = length(data.wordsVa);
data.wordsTe = data.words(idxTest);
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

data.phocsTr = data.phocs(:,idxTrain);
data.phocsVa = data.phocs(:,idxValidation);
data.phocsTe = data.phocs(:,idxTest);
data.phocsQu_subword = data.phocs_subword(:,:);
end