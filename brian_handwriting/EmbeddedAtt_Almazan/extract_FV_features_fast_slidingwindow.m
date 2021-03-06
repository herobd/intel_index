function extract_FV_features_fast_slidingwindow(opts,DATA)

GMM = readGMM(opts.fileGMM);
PCA = readPCA(opts.filePCA);
imagesTOC = readImagesToc(opts.fileImages);
meta={};
meta.nWords = length(imagesTOC);

%imagesPerBatch = 256;
%imagesPerBatch = nWords;
meta.imagesPerBatch = 128;
meta.nBatches = int32(ceil(meta.nWords/meta.imagesPerBatch));
meta.nInBatch=[];
%featsBatch = zeros(opts.FVdim,imagesPerBatch,'single');
featsBatch = {};

% Write output header
% fid = fopen(opts.fileFeatures_slidingwindow, 'w');
% fwrite(fid, nWords, 'int32');
% fwrite(fid, opts.FVdim, 'int32');
% fclose(fid);
tic;
for cb=1:meta.nBatches    
    sp = (cb-1)*meta.imagesPerBatch + 1;
    ep = sp + meta.imagesPerBatch -1;
    if ep > meta.nWords
        ep = meta.nWords;
    end
    meta.nInBatch(cb) = ep-sp+1;
    fprintf('Extracting FV batch %d/%d (%d images)\n',cb,meta.nBatches,meta.nInBatch(cb));        
    % Read image batch
    [fid,msg] = fopen(opts.fileImages, 'r');
    readIm = @(x) readImage(fid, imagesTOC, x);
    imagesBatch = arrayfun(readIm, [sp:ep], 'uniformOutput', false);
    fclose(fid);
    
    windowWidth=opts.windowWidth;
    windowStride=opts.windowStride;
    
    for i=1:length(imagesBatch)        
        full_im = imagesBatch{i};       
        featsBatch{i}=[];
        %%%Do sliding window
        %We will assume we can just take a horizontal sliding window
%         window=1;
        %gt = DATA.words(i).gttext;
        %low_gt=lower(gt);
        if size(full_im,2)>windowWidth
            for window_lx = 1:windowStride:size(full_im,2)-windowWidth
                %extract sub-image
                endIdx=(window_lx+windowWidth);
                if (endIdx > size(full_im,2))
                    endIdx=size(full_im,2);
                end

                im = full_im(:,(window_lx:endIdx));


                fv=computeFV(opts, im, GMM, PCA);
                featsBatch{i} = [featsBatch{i} fv]; 
    %             window=window+1;
                
                %check what ngrams this window overlaps
            end
        else
            fv=computeFV(opts, full_im, GMM, PCA);
            featsBatch{i} = [featsBatch{i} fv]; 
        end
        %%%
    end
    %featsBatch(isnan(featsBatch)) = 0;
    % Write the batch
%     fid = fopen(opts.fileFeatures_slidingwindow, 'r+');    
%     fseek(fid, 2*4  + (int64(cb)-1)*imagesPerBatch*opts.FVdim * 4, 'bof');
%     fwrite(fid, featsBatch(:,1:nInBatch,:), 'single');        
    %if (size(featsBatch,2) ~= meta.nInBatch(cb))
    %    disp(['ERROR, batch size dif: ' num2str(size(featsBatch,2))]);
    %end
    save(strcat(opts.fileFeatures_slidingwindow,num2str(cb)),'featsBatch','-v7.3');
end
save(opts.fileFeatures_slidingwindow_meta,'meta','-v7.3');
disp(toc);

end

% -------------------------------------------------------------------------
function fv = getImageDescriptorFV(opts, GMM, PCA, descrs)
% -------------------------------------------------------------------------

% Project into PCA space
xy = descrs(opts.SIFTDIM+1:end,:);
descrs=bsxfun(@minus, descrs(1:opts.SIFTDIM,:), PCA.mean);
descrs=PCA.eigvec'*descrs;

descrs = [descrs; xy];

% Extracts FV representation using the GMM
fv  =  vl_fisher(descrs, GMM.mu, GMM.sigma, GMM.we, 'Improved');
end

function X = normFV(X)
% -------------------------------------------------------------------------
X = sign(X).*sqrt(abs(X));
X = bsxfun(@rdivide, X, sqrt(sum(X.*X)));
X(isnan(X)) = 0;
end

function [descrs_normalized,frames_normalized] = normalizeSift(opts,descrs,frames)
% -------------------------------------------------------------------------
descrs_normalized = descrs;

xy = descrs_normalized(opts.SIFTDIM+1:end,:);
descrs_normalized = descrs_normalized(1:opts.SIFTDIM,:);

% Remove empty ones
idx = find(sum(descrs_normalized)==0);
descrs_normalized(:,idx)=[];
if nargin < 3
    frames_normalized = [];
else
    frames_normalized = frames;
    frames_normalized(:,idx) = [];
end

% Square root:
descrs_normalized = sqrt(descrs_normalized);

% 1/4 norm
X = sum(descrs_normalized.*descrs_normalized).^-0.25;
descrs_normalized = bsxfun(@times, descrs_normalized,X);

xy(:,idx) = [];
descrs_normalized = [descrs_normalized; xy];

descrs_normalized(isnan(descrs_normalized))=0;
end

function im = adjustImage(im)
imOrig = im;
im = im2bw(im);
[h,w] = size(im);
x = find(im==0);
w1 = ceil(min(x)/h);
w2 = floor(max(x)/h);
h1 = min(mod(x,h))+1;
h2 = max(mod(x,h))-1;
im = imOrig(h1:h2,w1:w2);
end

function fv = computeFV(opts, im, GMM, PCA)
    [height,width] = size(im);
    im = im2single(im);

    % get PHOW features
    [frames, descrs] = vl_phow(im, opts.phowOpts{:}) ;
    descrs = descrs / 255;



    if opts.doMinibox == 0
        % XY at GT coordinate space
        fx = single(frames(1,:)/width-0.5);
        fy = single(frames(2,:)/height-0.5);
    else
        % XY at word coordinate space
        bb = DoBB(im);
        w = bb(2)-bb(1)+1;
        h = bb(4)-bb(3)+1;
        cx = round(bb(1)+w/2);
        cy = round(bb(3)+h/2);
        fx = single((frames(1,:)-cx)/w);
        fy = single((frames(2,:)-cy)/h);
    end
    xy = [fx; fy];
    descrs = [descrs; xy];


    [descrs,frames] = normalizeSift(opts,descrs,frames);

    fv = single(getImageDescriptorFV(opts, GMM, PCA, descrs));
    fv(isnan(fv))=0;
end
