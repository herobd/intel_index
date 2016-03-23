function data = load_subword(opts,data)
%to be called after loading the main dataset
disp('* Reading character n-gram queries info *');

ngramIdx =[];
if opts.TestHybrid
    lines = strsplit(fileread(opts.pathNgramLocationsTop),'\n');
    for i = 2:2:length(lines)
        ngram=lines{i-1};
        line = textscan(lines{i},'wordimg_%d,');
        for j = 1:opts.ngramCountPer
            ngramIdx = [ngramIdx line{1}(j)];
        end;
    end
else
    for bi = opts.ngrams
        ngramIdx = [ngramIdx -1];
    end
end
data.ngramIdx = ngramIdx;

for j=1:length(data.words)
    data.words(j).class_subword = [];
        
end



newClass = 1;
words(1).class = [];
classes = containers.Map();
idxClasses = {};

ngramCount=0;
for ngramC = opts.ngrams
    
    ngram = ngramC{1};
    class = int32(newClass);
    newClass = newClass+1;
    classes(ngram) = class;
    idxClasses{class} = int32([]);
    
    dir = [opts.pathNgrams lower(ngram)];
    dir = [dir '/'];
    for j = 1:opts.ngramCountPer
        if opts.TestHybrid
            words(j+ngramCount*opts.ngramCountPer).pathIm = sprintf('%s/%03d.png',dir,j);
        
            im = imread(words(j+ngramCount*opts.ngramCountPer).pathIm);
            if ndims(im)>2
                im = rgb2gray(im);
            end
            
            [H,W] = size(im);
            x1 = 1; x2 = W;
            y1 = 1; y2 = H;
            words(j+ngramCount*opts.ngramCountPer).loc = [x1 x2 y1 y2];
            words(j+ngramCount*opts.ngramCountPer).locOrig = [x1 x2 y1 y2];
            [words(j+ngramCount*opts.ngramCountPer).H,words(j+ngramCount*opts.ngramCountPer).W] = size(im);
        else
            words(j+ngramCount*opts.ngramCountPer).pathIm = 'No image';
            words(j+ngramCount*opts.ngramCountPer).loc = [-1 -1 -1 -1];
            words(j+ngramCount*opts.ngramCountPer).locOrig = [-1 -1 -1 -1];
            words(j+ngramCount*opts.ngramCountPer).H=-1;
            words(j+ngramCount*opts.ngramCountPer).W=-1;
        end
        words(j+ngramCount*opts.ngramCountPer).gttext = ngram;
        
        wordId = sprintf('%s_%03d',ngram,j);
        words(j+ngramCount*opts.ngramCountPer).docId = wordId;
        
        words(j+ngramCount*opts.ngramCountPer).wordId = wordId;
        words(j+ngramCount*opts.ngramCountPer).lineId = 0; 
        words(j+ngramCount*opts.ngramCountPer).class_subword = class;
    end
    
    
    
    %exp = ['.*' ngram];
    %exp = [exp '.*'];
    ngramOccCount=0;
    for j=1:length(data.words)
        %if ~isempty(strfind(opts.words(j).gttext, ngram))
        %if ~isempty(regexp(opts.words(j).gttext,exp))
        if ~isempty(regexpi(data.words(j).gttext,ngram))
            idxClasses{class} = [idxClasses{class} j];
            data.words(j).class_subword = [data.words(j).class_subword class];
            ngramOccCount=ngramOccCount+1;
        end
    end
    %disp([ngram ': ' num2str(ngramOccCount)]);
    ngramCount = ngramCount+1;
end




%%The end result, so far
data.words_subword = words; %same as other words
data.classes_subword = classes;
data.idxClasses_subword = idxClasses;
data.queriesIdx = ngramIdx;

end
