function proj_attributes_subword(opts,data,W)
% projects the dataset to the new attribute
% space

    features_slidingwindowX=load(opts.fileFeatures_slidingwindow,'featsBatch');%,'-v7.3');
            features_slidingwindow=features_slidingwindowX.featsBatch;

    if ~exist('features_slidingwindow','var');
        features_slidingwindowX=load(opts.fileFeatures_slidingwindow,'featsBatch');
        features_slidingwindow=features_slidingwindowX.featsBatch;
    end
    
    feats_te_slidingwindow = features_slidingwindow(data.idxTest);
    
    
    attReprTe_slidingwindow = {};
    for i  =1:size(feats_te_slidingwindow,2)
        attReprTe_slidingwindow{i} = W'*feats_te_slidingwindow{i};
    end

    save(opts.fileAttRepresTe_slidingwindow,'attReprTe_slidingwindow'); 

end
