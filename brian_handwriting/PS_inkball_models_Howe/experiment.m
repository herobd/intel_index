%read in locations
%for each image in ngram directories, score against every image in word directory
%(exepct those that are in the top10 file?)
%rank by score, compute statistics
locationCSVPath='~/intel_index/data/gw_20p_wannot/bigramLocations.csv';
exemplarDirPath='~/intel_index/data/gw_20p_wannot/bigrams/';
imageNameNumExtract='(?<=wordimg_)\d+';
imageNameFormat='%swordimg_%d.tif';
exemplarNameFormat='%s%03d.png';
dataDirPath = '~/intel_index/data/gw_20p_wannot/words/';
dataSize=4860;


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


scores = [dataSize,2];
fullResults = containers.Map;

map=0;
mapCount=0;

for ngramCell = locations.keys()
    ngram = ngramCell{1};
    dirPath = strcat(exemplarDirPath,ngram,'/');
    ngram_locations = locations(ngram);
    
    for exemplarIdx = 1:10
        imagePath = sprintf(exemplarNameFormat, dirPath, exemplarIdx);
        exemplar = (imread(imagePath))<128;
        exemplar_m = autoPsm(exemplar);
        
        
        root = find([exemplar_m.parent]==0);
        for imageIdx = 1:dataSize
            %ignore exemplar image
            temp3=num2str(imageIdx);
            temp4=ngram_locations(exemplarIdx);
            if (strcmp(temp3,temp4{1}))
                temp3
                continue;
            end
            
            imagePath = sprintf(imageNameFormat, dataDirPath, imageIdx);
            word = (imread(imagePath))<128;
            sktext = bwmorph(word,'thin',inf);
            
            % Fit model forwards (exemplar to word)
%             try
%                 % attempt gpu version if available
%                 [dtsq,loc] = psmFit_gpu(exemplar_m,sktext,[8 8]);  % Third argument should be greater than maximum node displacement
%             catch
                % else fall back to cpu code
                [dtsq,loc] = psmFit(exemplar_m,sktext);
%             end;

            % get best location
            %             [min,y,x] = min2d(dtsq);
            [mins,ys,xs] = localMins2d(dtsq);




            skfox = bwmorph(exemplar,'thin',inf);
            newMins=zeros(size(mins));
%             newMinsx=cell(size(mins));
%             newMinsy=cell(size(mins));
%             locs2=cell(size(mins));
            for i = 1:size(mins,2) %for all the local minima
                minx=99999;
                maxx=0;
                miny=99999;
                maxy=0;

                for x = loc{ys(i),xs(i)}(1,:)
                    if (x<minx) minx=x; end;
                    if (x>maxx) maxx=x; end;
                end
                for y = loc{ys(i),xs(i)}(2,:)
                    if (y<miny) miny=y; end;
                    if (y>maxy) maxy=y; end;
                end
                %put some padding around the area
                miny=max(miny-5,1);
                maxy=min(maxy+5,size(word,1));
                minx=max(minx-5,1);
                maxx=min(maxx+5,size(word,2));

                subIm = word(miny:maxy,minx:maxx,:);
            %     figure
            %     imshow(subIm);
            %     hold on
                
                subIm_m = autoPsm(subIm);
                % Fit model backwards (sub image of word to exemplar)
            %                 try
                    % attempt gpu version if available
            %                     [dtsq2,loc2] = psmFit_gpu(subIm_m,skfox,[8 8]);  % Third argument should be greater than maximum node displacement
            %                 catch
                    % else fall back to cpu code
                    if (size(skfox,1)>=size(subIm,1) && size(skfox,2)>=size(subIm,2))
                        [dtsq2,loc2] = psmFit(subIm_m,skfox);
                    else
                        difW=max(0,size(subIm,2)-size(skfox,2));
                        difH=max(0,size(subIm,1)-size(skfox,1));
                        skpadded = [skfox zeros(size(skfox,1), difW); 
                                    zeros(difH, size(skfox,2)) zeros(difH,difW)];
                        [dtsq2,loc2] = psmFit(subIm_m,skpadded);
                    end
            %                 end;
                [min2,min2y,min2x] = min2d(dtsq2);
                newMins(1,i)=mins(i)+min2; %combine forward and backward scores
%                 newMinsx{i}=min2x;
%                 newMinsy{i}=min2y;
%                 locs2{i}=loc2;
            end

            [bestMin,bestI] = min(newMins); %find smallest score combo

            scores(imageIdx,1)=imageIdx;
            scores(imageIdx,2)=bestMin;
            
%             if ii==100 break; end;
        end
        
        scores = sortrows(scores,2);
        fullResults(strcat(ngram,num2str(exemplarIdx)))=scores;
        %compute average precision
        foundRelevent = 0;
        avgPrecision = 0.0;
%         pastRecall=0;
        totalRelevent=size(ngram_locations,2);
        for top = 1:size(scores,1)
            if (ismember([num2str(top)],ngram_locations))
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
    end
    
end
map = map/mapCount