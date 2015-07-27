load('/home/brian/intel_index/brian_handwriting/PS_inkball_models_Howe/results_howe_ver.mat35','fullResults');

locationCSVPath='~/intel_index/data/gw_20p_wannot/wordLocations.csv';
imageNameNumExtract='\d+';
file = fopen(locationCSVPath);
ngram = fgetl(file);
fileList = fgetl(file);
%files = regexp(fileList,'\w+,?','match');
files = regexp(fileList,imageNameNumExtract,'match');
locations = containers.Map;
locations(ngram) = files;
while ischar(ngram)
    
    ngram = fgetl(file);
    if (ngram==-1) break; end;
    fileList = fgetl(file);
    files = regexp(fileList,imageNameNumExtract,'match');
    locations(ngram) = files;
end

fclose(file);

ap=[];
map=0;
mapCount=0;

for exCell = fullResults.keys()
    ex = exCell{1};
    scores = fullResults(ex);
    
    [s,e]=regexp(ex,'[a-zA-Z]*');
    ngram = ex(s:e);
    ngram_locations = locations(ngram);
    
    foundRelevent = 0;
    avgPrecision = 0.0;
%         pastRecall=0;
    
    totalRelevent=size(ngram_locations,2);
    for top = 1:size(scores,1)
        ii = scores(top,1);
        if (ismember([num2str(ii)],ngram_locations))
            foundRelevent = foundRelevent+1;
            precision  = foundRelevent/top;
            avgPrecision = avgPrecision+ precision;
        end
%             precision  = foundRelevent/top;
%             recall = foundRelevent/totalRelevent;
%             avgPrecision = avgPrecision+precision*(recall-pastRecall);
%             pastRecall = recall;
    end
%         avgPrecision = avgPrecision/size(scores,1);
    avgPrecision = avgPrecision/totalRelevent;
    map = map+avgPrecision;
        mapCount=mapCount+1;
    
    disp(['average precision for ' ex ': ' num2str(avgPrecision)]);
end

map = map/mapCount