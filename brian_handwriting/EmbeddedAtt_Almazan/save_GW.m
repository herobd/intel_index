%function save_GW(opts)
disp('* Reading GW info and saving images *');
opts=struct;
opts.dataset='GW';
opts.path_datasets = 'datasets';
opts.pathDataset = sprintf('%s/%s/',opts.path_datasets,opts.dataset);
opts.pathImages = sprintf('%s/%s/images/',opts.path_datasets,opts.dataset);
opts.pathDocuments = sprintf('%s/%s/documents/',opts.path_datasets,opts.dataset);
opts.pathQueries = sprintf('%s/%s/queries/',opts.path_datasets,opts.dataset);


%%%%%%%%
%% Reading words information
fileQueries=[opts.pathQueries 'test_queries.gtp'];
fid = fopen(fileQueries, 'r');
input = textscan(fid, '%s %d %d %d %d %s');
nWords = length(input{1});

margin=16;
pathIm = '';

for j=1:nWords
    thispathIm = [opts.pathImages input{1}{j}];
    loc = [input{2}(j) input{4}(j) input{3}(j) input{5}(j)];
    locOrig = loc;
    origLoc = loc;
    
    loc = [loc(1)-margin loc(2)+margin loc(3)-margin loc(4)+margin];
    if ~strcmp(thispathIm,pathIm)
        imDoc = imread(thispathIm);
        if ndims(imDoc)>2
            imDoc = rgb2gray(imDoc);
        end
        pathIm = thispathIm;
    end
    [H,W] = size(imDoc);
    x1 = max(loc(1),1); x2 = min(loc(2),W);
    y1 = max(loc(3),1); y2 = min(loc(4),H);
    im = imDoc(y1:y2,x1:x2);
    
    imwrite(im,['~/intel_index/data/gw_20p_wannot/Almazan_words/wordimg_' num2str(j) '.png']);
    
end

