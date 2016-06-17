function extract_features_subword(opts)
disp('* Extracting FV features *');
% Extracts the FV representation for every image in the dataset

if  ~exist(opts.fileFeatures,'file')
      
    if ~exist(opts.fileGMM,'file')
        toc = readImagesToc(opts.fileImages);
        % Computes GMM and PCA models
        idxTrainGMM = sort(randperm(length(toc),opts.numWordsTrainGMM));
        [fid,msg] = fopen(opts.fileImages, 'r');
        getImage = @(x) readImage(fid,toc,x);
        images = arrayfun(getImage, idxTrainGMM', 'uniformoutput', false);
        fclose(fid);
        [GMM,PCA] = compute_GMM_PCA_models(opts,images);
        writeGMM(GMM,opts.fileGMM);
        writePCA(PCA, opts.filePCA); 
        clear images;
    end
    
    extract_FV_features_fast(opts);
    
    
end

if  ~exist(opts.fileFeatures_slidingwindow_meta,'file')
    extract_FV_features_fast_slidingwindow(opts);
end

if  ~exist(opts.fileFeatures_subword,'file')
    extract_FV_features_fast_subword(opts);
end

end
