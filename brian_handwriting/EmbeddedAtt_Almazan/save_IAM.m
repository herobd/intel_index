%function save_IAM(opts)
disp('* Reading IAM info and saving images *');
opts=struct;
opts.dataset='IAM';
opts.path_datasets = 'datasets';
opts.pathDataset = sprintf('%s/%s/',opts.path_datasets,opts.dataset);
opts.pathImages = sprintf('%s/%s/images/',opts.path_datasets,opts.dataset);
opts.pathDocuments = sprintf('%s/%s/documents/',opts.path_datasets,opts.dataset);
opts.pathQueries = sprintf('%s/%s/queries/',opts.path_datasets,opts.dataset);

%% Reading writers information
file=[opts.pathDataset 'forms.gtp'];
fid = fopen(file, 'r');

input = textscan(fid, '%s %d %d %s %d %d %d %d');
nDocs = length(input{1});
numWriters = max(input{2}(:))+1;

docs = containers.Map();
list = cell(numWriters,1);
for i=1:nDocs
    % Determine the class of the query given the GT text
    idW = input{2}(i) + 1;
    docs(input{1}{i}) = idW;
    list{idW}{length(list{idW})+1} = input{1}{i};
end
writers.list = list;
writers.docs = docs;
writers.num = length(list);
fclose(fid);
data.writers = writers;

%% Reading words information
fileQueries=[opts.pathQueries 'test_queries.gtp'];
fid = fopen(fileQueries, 'r');
input = textscan(fid, '%s %d %d %d %d %s %s');
nWords = length(input{1});

margin=16;
pathIm = '';

for j=1:nWords
    %id=input{7}{j};
    %id = [id '.png'];
    pathIm = [opts.pathImages id];
    
    
    %words(j).pathIm = [opts.pathImages input{1}{j}];
    %words(j).pathIm = [words(j).pathIm '.png'];
    loc = [input{2}(j) input{4}(j) input{3}(j) input{5}(j)];
    locOrig = loc;
    %words(j).origLoc = loc;
    
    loc = [loc(1)-margin loc(2)+margin loc(3)-margin loc(4)+margin];
    tmp=input{1}(j);
    thisPathIm = [opts.pathImages tmp{1} '.png'];
    if ~strcmp(words(j).pathIm,pathIm)
        %imDoc = imread(words(j).pathIm);
        imDoc = imread(thisPathIm);
        if ndims(imDoc)>2
            imDoc = rgb2gray(imDoc);
        end
        %pathIm = words(j).pathIm;
        pathIm = thisPathIm;
    end
    [H,W] = size(imDoc);
    x1 = max(loc(1),1); x2 = min(loc(2),W);
    y1 = max(loc(3),1); y2 = min(loc(4),H);
    im = imDoc(y1:y2,x1:x2);
    %mkdir('~/intel_index/data/IAM/Almazan_words',id(1:3));
    imwrite(im,['~/intel_index/data/IAM/Almazan_words/wordimg_' num2str(j) '.png']);
    
       
end


