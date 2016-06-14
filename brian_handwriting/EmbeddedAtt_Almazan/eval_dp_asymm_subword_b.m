function [p1, mAP, thresh] = eval_dp_asymm_subword(alpha, DATA, opts, queries, allClasses, queriesClasses, queriesIdx, doqbs)
if nargin < 9
    doqbs = 0;
end

% Get size and prepare stuff
[d,N] = size(queries);
queries = single(queries');
%dataset = single(dataset');

%For each element, find the number of relevants
% H = hist(single(allClasses), single(1:max(max(allClasses))));
% nRel = H(allClasses)';
% 
% % Keep as queries only words that appear at least 1 more time
% % besides being the query. If necessary, remove stopwords)
% keep = find(nRel >=0);
% if opts.RemoveStopWords
%     sw = textread(opts.swFile,'%s','delimiter',',');
%     sw = unique(lower(sw));
%     keep = find(~ismember(lower(queriesWords), lower(sw)));
% end
% if doqbs == 0;
%     keep = intersect(keep, find(nRel >=2));
% else
%     keep = intersect(keep, find(nRel >=1));
% end
% 
% % If QBS, keep only one instance per query
% if doqbs
%     set = containers.Map('keytype','int32','ValueType','int32');
%     % If qbe, keep only one instance of each class as query
%     keep2=[];
%     for i=1:length(keep)
%         if ~set.isKey(allClasses(keep(i)))
%             set(allClasses(keep(i)))=1;
%             keep2 = [keep2 keep(i)];
%         end
%     end
%     keep = keep2;
% end
% keepQueries = keep;
% 
% % Get updated queries
% queries = queries(keepQueries,:);
% queriesClases = allClasses(keepQueries);

% Keep as queries only words also occurring in training
% load(opts.Data,'wordsTr');
% keep = ismember(lower(queriesWords), lower(unique(wordsTr)));
% queries = queries(keep,:);
% queriesClases = queriesClases(keep);

N = size(queries,1);

% Compute the number of relevants for each query:
% NRelevantsPerQuery = H(queriesClases)';
for i=1:size(queriesClasses,2)
    count=0;
    for list = allClasses
        %if ismember(queriesClasses(i),list)
        qClass=queriesClasses(i);
        if any(qClass{:}==list{:})
            count=count+1;
        end
    end
    NRelevantsPerQuery(i)=count;
end


if doqbs
    s = sum(queries,2);
    idxNan = find(isnan(s));
end

queries(isnan(queries)) = 0;

% Compute scores
%S=queries*dataset';
if opts.saveSpottings
    
    [imagesTOC,countImages] = readImagesTocCount(opts.fileImages);
    [fid,msg] = fopen(opts.fileImages, 'r');
    nums = [1:size(DATA.idxTest,1)];
    mapImages = nums(DATA.idxTest);
    assert(size(mapImages,2) == size(dataset_slidingwindow,2));
    spottings=[];
    windowWidth=opts.windowWidth;
    windowStride=opts.windowStride;
    pathResults = sprintf('%s_%f',opts.pathResults,alpha);
    w = warning ('off','all');
    mkdir(pathResults);
end

S=[];

if ~exist('meta','var')
    load(opts.fileAttRepresTe_slidingwindow_meta,'meta');
end
curPos=0;

for cb = 1:meta.nBatches
    disp(['loading  attReprTe_cca for batch ' num2str(cb)]);
    load(strcat(opts.fileAttRepresTe_cca_slidingwindow,num2str(cb)),'attReprTe_cca_slidingwindow');
    
    
    for i = 1:size(attReprTe_cca_slidingwindow,2)
        tmp = queries*attReprTe_cca_slidingwindow{i};
        if opts.saveSpottings
            
            ii=1;
            for ngram = opts.ngrams
            maxScoreForNgram=0;
                location=-1;
                for exN = 1:opts.ngramCountPer
                    for loc = 1:size(tmp,2)
                        scoreHere = tmp(ii,loc);
                        if scoreHere > maxScoreForNgram
                            maxScoreForNgram=scoreHere;
                            location=loc;
                        end
                    end
                    ii = ii+1;
                end
                if location ~= -1 && maxScoreForNgram>opts.threshold
                    %highlight spotted window
                    %img_gray = imread(DATA.wordsTe(i).pathIm);
                    img_gray = readImage(fid, imagesTOC, mapImages(i+curPos));
                    img = cat(3, img_gray, img_gray, img_gray);
                    locationStart = 1 + (location-1)*windowStride;
                    locationEnd = locationStart+windowWidth;
                    if locationEnd>size(img,2)
                        locationEnd=size(img,2);
                    end
                    for c = locationStart:locationEnd
                        for r = 1:size(img,1)
                            img(r,c,2)=0.4*img(r,c,2);
                            img(r,c,3)=0.4*img(r,c,3);
                        end
                    end
                    img= uint8(img);
                    mkdir(pathResults,ngram{1});
                    imwrite(img,sprintf('%s/%s/wordimg_%d.png',pathResults,ngram{1},i+curPos));
                end
            end
        end
        S(:,i+curPos) = max(tmp,[],2);
        curPos=curPos+meta.nInBatch(cb);
    end

if opts.saveSpottings
    warning(w)
    fclose(fid);
    
end

% The last part automatically removes the query from the dataset when
% computing stats by providing the indexes of the queries. If the queries
% are not included in the dataset, set the last parameter to a vector of
% [-1,-1,...,-1];
disp('Running MEX');
if doqbs==0
[p1, mAP,bestIdx,thresh] = computeStats_c_subword(single(S'),int32([queriesClasses{:}]'), allClasses, int32(NRelevantsPerQuery'), int32(queriesIdx'-1), opts.precForThresh);
else
    v = ones(N)*-1;
[p1, mAP,bestIdx,thresh] = computeStats_c_subword(single(S'),int32([queriesClasses{:}]'), allClasses, int32(NRelevantsPerQuery'), int32(v), opts.precForThresh);
end
disp('Done with MEX');
disp(mean(mAP));
% qidx=keepQueries;


end
