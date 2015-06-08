% Demo of Inkblot Part-Structured Models
%
% Reference code for this paper:
% N. Howe, Part-Structured Inkball Models for One-Shot Handwritten Word Spotting.
% International Conference on Document Analysis and Recognition, August 2013.

textimgname='../../data/gw_20p_wannot/words/wordimg_25.tif';

% Read in images
% fox = rgb2gray(imread('gw_an.png'))<128;
% text = rgb2gray(imread('gw_and.tif'))<128;
exemplar = (imread('../../data/gw_20p_wannot/bigrams/to/001.png'))<128;
word = (imread(textimgname))<128;


% Build model & skeletonize text
exemplar_m = autoPsm(exemplar);
root = find([exemplar_m.parent]==0);
sktext = bwmorph(word,'thin',inf);
% figure
% imshow(fox);
% hold on;
% plot([fox_m.absx]+fox_rp(1)-fox_m(root).x,[fox_m.absy]+fox_rp(2)-fox_m(root).y,'r*');

% Fit model
% try
%     % attempt gpu version if available
%     [dtsq,loc] = psmFit_gpu(exemplar_m,sktext,[8 8]);  % Third argument should be greater than maximum node displacement
% catch
%     % else fall back to cpu code
%     [dtsq,loc] = psmFit(exemplar_m,sktext);
% end;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
[dtsq,loc] = psmFit(exemplar_m,sktext);


%             end;

% get best location
% [minV,y,x] = min2d(dtsq);
% figure
% imshow(word);
% hold on
% plot(loc{y,x}(1,:),loc{y,x}(2,:),'r*')

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
[mins,ys,xs] = localMins2d(dtsq);




skfox = bwmorph(exemplar,'thin',inf);
newMins=zeros(size(mins));
newMinsx=cell(size(mins));
newMinsy=cell(size(mins));
locs2=cell(size(mins));
for i = 1:size(mins,2)
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
    miny=max(miny-5,1);
    maxy=min(maxy+5,size(word,1));
    minx=max(minx-5,1);
    maxx=min(maxx+5,size(word,2));

    subIm = word(miny:maxy,minx:maxx,:);
%     figure
%     imshow(subIm);
%     hold on

    subIm_m = autoPsm(subIm);
    % Fit model
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
    newMins(1,i)=mins(i)+min2;
    newMinsx{i}=min2x;
    newMinsy{i}=min2y;
    locs2{i}=loc2;
end

[bestMin,bestI] = min(newMins);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
bestMin


x=newMinsx{bestI};
y=newMinsy{bestI};
loc2=locs2{bestI};
figure('Name','backwards')
imshow(exemplar);
hold on
plot(loc2{y,x}(1,:),loc2{y,x}(2,:),'r*')

% [minV,y,x] = min2d(dtsq);
x=xs(bestI);
y=ys(bestI);
figure('Name','forewards')
imshow(word);
hold on
plot(loc{y,x}(1,:),loc{y,x}(2,:),'r*')

    minx=99999;
    maxx=0;
    miny=99999;
    maxy=0;

    for x = loc{ys(bestI),xs(bestI)}(1,:)
        if (x<minx) minx=x; end;
        if (x>maxx) maxx=x; end;
    end
    for y = loc{ys(bestI),xs(bestI)}(2,:)
        if (y<miny) miny=y; end;
        if (y>maxy) maxy=y; end;
    end
    miny=max(miny-5,1);
    maxy=min(maxy+5,size(word,1));
    minx=max(minx-5,1);
    maxx=min(maxx+5,size(word,2));

    subIm = word(miny:maxy,minx:maxx,:);
    figure('Name','subImage')
    imshow(subIm);
    hold on

% maxV=60;
% heatmap = imread(textimgname);
% for i = 1:size(heatmap,1)
%     for j = 1:size(heatmap,2)
%         score=dtsq(i,j);
%         ratio=(score-minV)/maxV;
%         color=[heatmap(i,j,1), heatmap(i,j,1)*ratio, heatmap(i,j,1)*ratio];
%         %color=cast(color,'like',heatmap(i,j));
%         
%         heatmap(i,j,1) = color(1);
%         heatmap(i,j,2) = color(2);
%         heatmap(i,j,3) = color(3);
%     end
% end
% 
% figure
% imshow(heatmap)
% hold on
