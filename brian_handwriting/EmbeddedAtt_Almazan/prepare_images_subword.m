function  prepare_images_subword(opts,data)
disp('* Preparing images subword*');

if ~exist(opts.fileImages_subword, 'file')
    % Prepare header
    % Get bytes per image after resizing
    bytesPerImage = int64(zeros(1,length(data.words_subword)));
    for i=1:length(data.words_subword)
        H = data.words_subword(i).H;
        W = data.words_subword(i).W;
        if (opts.minH > data.words_subword(i).H)
            W = ceil((opts.minH * data.words_subword(i).W)/data.words_subword(i).H-1e-5);
            H = opts.minH;
        end
        if (opts.maxH < data.words_subword(i).H)
            W = ceil((opts.maxH * data.words_subword(i).W)/data.words_subword(i).H-1e-5);
            H = opts.maxH;
        end        
        bytesPerImage(i) = 8 + W*H;% Two bytes for image header (rows, cols) plus k elements as uint8
    end
    
    posAtI = 4 + length(bytesPerImage)* 8 + [ 0 cumsum(double(bytesPerImage))]; % Numer of images + lookup (8 byte integers) + accumulated sizes
    posAtI = int64(posAtI(1:end-1));
    
    fid = fopen(opts.fileImages_subword, 'w');
    fwrite(fid, int32(length(bytesPerImage)), 'int32');
    fwrite(fid, posAtI, 'int64');
    
    previousPath = '';
    
    for i=1:length(data.words_subword)
        if ~strcmp(previousPath,data.words_subword(i).pathIm)
            previousPath = data.words_subword(i).pathIm;
            im = imread(data.words_subword(i).pathIm);
            [Him,Wim,numC] = size(im);
            if numC > 1
                im = rgb2gray(im);
            end
        end
        
        patch = im(data.words_subword(i).loc(3):data.words_subword(i).loc(4),data.words_subword(i).loc(1):data.words_subword(i).loc(2));
        
        % Move to single and equalize if necessary
        patch = im2single(patch);
        m = max(max(patch));
        if m < 0.2
            patch = patch*0.2/m;
        end
                             
        [H,W] = size(patch);
        if (data.words_subword(i).H~=H || data.words_subword(i).W~=W)
            error('something wrong happened!');
        end
        
        if  (opts.minH > data.words_subword(i).H)
            patch = imresize(patch, [opts.minH,nan]);
        end
        if  (opts.maxH < data.words_subword(i).H)
            patch = imresize(patch, [opts.maxH,nan]);
        end
        
        [H,W,numC] = size(patch);
        % Save as uint8
        fwrite(fid, int32(W),'int32');
        fwrite(fid, int32(H), 'int32');
        fwrite(fid, im2uint8(patch), 'uint8');
    end
    fclose(fid);
end

end