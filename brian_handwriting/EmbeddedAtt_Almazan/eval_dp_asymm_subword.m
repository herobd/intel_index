function [p1, mAP, threshPrec,threshRecall,precAtRecall] = eval_dp_asymm_subword(alpha, DATA, opts, queries, dataset_slidingwindow, allClasses, queriesClasses, queriesIdx, doqbs)
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
%NRelevantsPerQuery


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
    
    [spottingsFID,msg] = fopen(sprintf(opts.spottingsFile,alpha),'w');
    disp(msg);
    [agSpottingsFID,msg] = fopen(sprintf(opts.agSpottingsFile,alpha),'w');
    disp(msg);
end

if opts.saveLineSpottings
    [lineSpottingsFID,msg] = fopen(sprintf(opts.lineSpottingsFile,alpha),'w');
    disp(msg);
end

S=[];
for i = 1:size(dataset_slidingwindow,2)
    tmp = queries*dataset_slidingwindow{i};
    if opts.saveSpottings
        wordInfo = DATA.words(mapImages(i));       
        ii=1;
        for ngram = opts.ngrams
	    maxScoreForNgram=-999;
            location=-1;
            for exN = 1:opts.ngramCountPer
	        maxScoreForNgramL=-999;
                locationL=-1;
                for loc = 1:size(tmp,2)
                    scoreHere = tmp(ii,loc);
                    if scoreHere > maxScoreForNgram
                        maxScoreForNgram=scoreHere;
                        location=loc;
                    end
                    if scoreHere > maxScoreForNgramL
                        maxScoreForNgramL=scoreHere;
                        locationL=loc;
                    end
                end
                ii = ii+1;
           
                if locationL ~= -1
	          img = readImage(fid, imagesTOC, mapImages(i));
                  locationStart = 1 + (locationL-1)*windowStride;
                  locationEnd = locationStart+windowWidth;
                  if locationEnd>size(img,2)
                    locationEnd=size(img,2);
                  end
                  
                  tlx = locationStart+wordInfo.loc(1)-1;
                  tly = wordInfo.loc(3);
                  brx = locationEnd+wordInfo.loc(1)-1;
                  bry = DATA.words(mapImages(i)).loc(4);
                  doc = DATA.words(mapImages(i)).docId;
                  truth='false';
                  res = findstr(ngram{1},lower(wordInfo.gttext));
                  if (length(res)>0)
                    %truth='true';

                    %check if its close
                    gtLen=length(wordInfo.gttext);
                    relL=floor(gtLen*locationStart/size(img,2));
                    relR=ceil(gtLen*locationEnd/size(img,2));
                    nLen=length(ngram{1});
                    for pos = res
                      if pos>=relL-nLen-1 && pos<=relR+1
                        truth='true';
                      end
                      if (mod(i,150)==0)
                        disp(sprintf('pos=%d, relL-nLen-1=%d, relR+1=%d, truth=%s\n',pos,relL-nLen-1,relR+1,truth));
                      end
                     end
                  end
                  fprintf(spottingsFID,'%s, %d, %s, %d, %d, %d, %d, %f, %s, %s, %d\n', ngram{1}, (exN-1), doc, tlx-1, tly-1, brx-1, bry-1, maxScoreForNgramL,truth,wordInfo.gttext,mapImages(i));
                end
            end
            if location ~= -1
	        img_gray = readImage(fid, imagesTOC, mapImages(i));
                img = cat(3, img_gray, img_gray, img_gray);
                locationStart = 1 + (location-1)*windowStride;
                locationEnd = locationStart+windowWidth;
                if locationEnd>size(img,2)
                    locationEnd=size(img,2);
                end
                if maxScoreForNgram>opts.threshold
                  %highlight spotted window
                  %img_gray = imread(DATA.wordsTe(i).pathIm);
                  for c = locationStart:locationEnd
                    for r = 1:size(img,1)
                        img(r,c,2)=0.4*img(r,c,2);
                        img(r,c,3)=0.4*img(r,c,3);
                    end
                  end
                
                  img= uint8(img);
                  mkdir(pathResults,ngram{1});
                  imwrite(img,sprintf('%s/%s/wordimg_%d.png',pathResults,ngram{1},i));
                end
                tlx = locationStart+DATA.words(mapImages(i)).loc(1)-1;
                tly = DATA.words(mapImages(i)).loc(3);
                brx = locationEnd+DATA.words(mapImages(i)).loc(1)-1;
                bry = DATA.words(mapImages(i)).loc(4);
                doc = DATA.words(mapImages(i)).docId;
                  truth='false';
                  res = findstr(ngram{1},lower(wordInfo.gttext));
                  if (length(res)>0)
                    %check if its close
                    gtLen=length(wordInfo.gttext);
                    relL=floor(gtLen*locationStart/size(img,2));
                    relR=ceil(gtLen*locationEnd/size(img,2));
                    nLen=length(ngram{1});
                    for pos = res
                      if pos>=relL-nLen-1 && pos<=relR+1
                        truth='true';
                      end
                    end
                  end
                fprintf(agSpottingsFID,'%s, %d, %s, %d, %d, %d, %d, %f, %s, %s, %d\n', ngram{1}, 0, doc, tlx-1, tly-1, brx-1, bry-1, maxScoreForNgram, truth,wordInfo.gttext,mapImages(i));
            end
        end

        
    end
    if opts.saveLineSpottings
        lineInfo = DATA.words(mapImages(i));
        ii=1;
        fprintf(lineSpottingsFID,'%s; ', lineInfo.wordId);
        for loc = 1:size(tmp,2)
            for ngram = opts.ngrams
                maxForExs=-9999;
                for exN = 1:opts.ngramCountPer
                    scoreHere = tmp(ii,loc);
                    if scoreHere > maxForEx
                        maxForExs=scoreHere;
                    end
                    ii = ii+1;
                end
                fprintf(lineSpottingsFID,'%f, ', maxForExs);
            end
            fprintf(lineSpottingsFID,'; ');
        end
    end
    S(:,i) = max(tmp,[],2);
end

if opts.saveSpottings
    warning(w)
    fclose(fid);
    fclose(spottingsFID);
    fclose(agSpottingsFID);
end

if opts.saveLineSpottings
    fclose(lineSpottingsFID);
    disp(['Done for ' num2str(alpha)]);
else
% The last part automatically removes the query from the dataset when
% computing stats by providing the indexes of the queries. If the queries
% are not included in the dataset, set the last parameter to a vector of
% [-1,-1,...,-1];
disp('Running MEX');
if doqbs==0
[p1, mAP,bestIdx,threshPrec,threshRecall,precAtRecall] = computeStats_c_subword(single(S'),int32([queriesClasses{:}]'), allClasses, int32(NRelevantsPerQuery'), int32(queriesIdx'-1), opts.precForThresh,opts.recallForThresh);
else
    v = ones(N)*-1;
[p1, mAP,bestIdx,threshPrec,threshRecall,precAtRecall] = computeStats_c_subword(single(S'),int32([queriesClasses{:}]'), allClasses, int32(NRelevantsPerQuery'), int32(v), opts.precForThresh,opts.recallForThresh);
end
disp('Done with MEX');
disp(mean(mAP));
% qidx=keepQueries;
end

end
