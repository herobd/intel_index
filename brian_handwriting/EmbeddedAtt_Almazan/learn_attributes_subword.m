function attModels = learn_attributes_subword(opts,data)
% Learns the attribute models and projects the dataset to the new attribute
% space

if ~exist(opts.fileAttModels,'file') || ~exist(opts.fileAttRepresQu_subword,'file')
    
    if opts.bagging
        if opts.cluster
            % When running the code in a cluster
        else
            % When running the code in a single machine
            data.feats_training = readMatBig(opts.fileFeatures,[find(data.idxTrain); find(data.idxValidation)]);
            features_slidingwindowX=load(opts.fileFeatures_slidingwindow,'featsBatch');%,'-v7.3');
            features_slidingwindow=features_slidingwindowX.featsBatch;
            features_subword = readMat(opts.fileFeatures_subword);
            % Training and validation sets are concatenated
            %data.feats_training = features(:, [find(data.idxTrain); find(data.idxValidation)]);
            data.phocs_training = [data.phocsTr data.phocsVa];
            %clear features;
            [attModels,attReprTr] = learn_attributes_bagging(opts,data);
        end
    else
        % Regular training (without bagging) should only use the training
        % set. In case that the dataset does not contain a validation set,
        % it is recommended to randomly split the training set in training
        % and validation sets. This will avoid overfitting when learning
        % the common subspace
    end
    
    % FV representations of the images are projected into the attribute
    % space
    W = [attModels(:).W];
    if ~exist('features','var');
        features = readMat(opts.fileFeatures);
    end
    if ~exist('features_slidingwindow','var');
        features_slidingwindowX=load(opts.fileFeatures_slidingwindow,'featsBatch');
        features_slidingwindow=features_slidingwindowX.featsBatch;
    end
    feats_va = features(:,data.idxValidation);
    if ~isempty(feats_va)
        attReprVa = W'*feats_va;
    else
        attReprVa = [];
    end
    feats_te_slidingwindow = features_slidingwindow(data.idxTest);
    feats_qu_subword = features_subword(:,:);
    
    %attReprTe_slidingwindow = W'*feats_te_slidingwindow;%%eehhh
    attReprTe_slidingwindow = {};
    for i  =1:size(feats_te_slidingwindow,2)
%         attReprTe_slidingwindow{i}=[];
%         for j = 1:size(feats_te_slidingwindow{i},2)
%             attReprTe_slidingwindow{i} = [attReprTe_slidingwindow{i} W'*feats_te_slidingwindow{i}(j)];
%         end
        attReprTe_slidingwindow{i} = W'*feats_te_slidingwindow{i};
    end
    
    attReprQu_subword = W'*feats_qu_subword;
    
    writeMat(single([[attModels.W];[attModels.B]]), opts.fileAttModels);
    writeMat(single(attReprTr), opts.fileAttRepresTr);
    writeMat(single(attReprVa), opts.fileAttRepresVal);
    save(opts.fileAttRepresTe_slidingwindow,'attReprTe_slidingwindow'); 
    writeMat(single(attReprQu_subword), opts.fileAttRepresQu_subword); 
else
   attModels = readMat(opts.fileAttModels);    
end
end
