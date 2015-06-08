fullResults = load('fullResults.mat','fullResults');

locationCSVPath='~/intel_index/data/gw_20p_wannot/bigramLocations.csv';
imageNameNumExtract='(?<=wordimg_)\d+';
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

for ngramCell = fullResults.keys()
    ngram = ngramCell{1};
    
end