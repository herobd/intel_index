function attModels = learn_attributes_subword(opts,data)
% Learns the attribute models and projects the dataset to the new attribute
% space

if ~exist(opts.fileAttModels,'file') || ~exist(opts.fileAttRepresQu_subword,'file')
    
    if opts.bagging
        if opts.cluster
            % When running the code in a cluster
        else
            % When running the code in a single machine
            
            
            %features = readMat(opts.fileFeatures);            
            % Training and validation sets are concatenated
            %data.feats_training = features(:, [find(data.idxTrain); find(data.idxValidation)]);
            
            %data.feats_training = readMatBig(opts.fileFeatures,[find(data.idxTrain); find(data.idxValidation)]);
            %data.phocs_training = [data.phocsTr data.phocsVa];
            disp('reading fileFeatures');
            data.feats_training = readMatBig(opts.fileFeatures,[find(data.idxTrain)]);
            data.phocs_training = [data.phocsTr];
            
            %features_subword = readMat(opts.fileFeatures_subword);
            
            
            %clear features;
            disp('bagging');
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
    disp('projecting va and subwords');
    W = [attModels(:).W];
    if ~exist('features','var');
        features = readMat(opts.fileFeatures);
    end
    feats_va = features(:,data.idxValidation);
    if ~isempty(feats_va)
        attReprVa = W'*feats_va;
    else
        attReprVa = [];
    end
    feats_qu_subword = features_subword(:,:);
    attReprQu_subword = W'*feats_qu_subword;
    
    disp('writing attModels');
    writeMat(single([[attModels.W];[attModels.B]]), opts.fileAttModels);
    writeMat(single(attReprTr), opts.fileAttRepresTr);
    writeMat(single(attReprVa), opts.fileAttRepresVal);
    writeMat(single(attReprQu_subword), opts.fileAttRepresQu_subword); 
else
   disp('reading attModels');
   attModels = readMat(opts.fileAttModels);    
end
end
