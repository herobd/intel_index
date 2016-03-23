function extract_features_test(opts)
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
        [GMM,PCA] = compute_GMM_PCA_models(opts,images);
        %writeGMM(GMM,opts.fileGMM);
        dlmwrite('GMM_mean_test.csv',GMM.mu,'precision',8);
        dlmwrite('GMM_covariances_test.csv',GMM.sigma,'precision',8);
        dlmwrite('GMM_priors_test.csv',GMM.we,'precision',8);
        %writePCA(PCA, opts.filePCA); 
        dlmwrite('PCA_mean_test.csv',PCA.mean,'precision',8);
        dlmwrite('PCA_eigvec_test.csv',PCA.eigvec,'precision',8);
        clear images;
    end
        
    extract_FV_features_fast_test(opts);
    
end

end
