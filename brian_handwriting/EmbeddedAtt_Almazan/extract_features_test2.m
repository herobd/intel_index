function extract_features_test2(opts,data)
disp('* Extracting FV features *');
% Extracts the FV representation for every image in the dataset

if  ~exist(opts.fileFeatures,'file')
      
    if ~exist(opts.fileGMM,'file')
        toc = readImagesToc(opts.fileImages);
        % Computes GMM and PCA models
        %idxTrainGMM = sort(randperm(length(toc),opts.numWordsTrainGMM));
        idxTrainGMM = [1:opts.numWordsTrainGMM];
        [fid,msg] = fopen(opts.fileImages, 'r');
        getImage = @(x) readImage(fid,toc,x);
        images = arrayfun(getImage, idxTrainGMM', 'uniformoutput', false);
        fclose(fid);
        [GMM,PCA] = compute_GMM_PCA_models_test(opts,data,images);
        writeGMM(GMM,opts.fileGMM);
        writePCA(PCA, opts.filePCA); 
        clear images;
    end
        
    extract_FV_features_fast(opts);
    
end

end
