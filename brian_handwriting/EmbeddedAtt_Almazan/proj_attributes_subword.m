function proj_attributes_subword(opts,data,attModels)
% projects the dataset to the new attribute
% space
    %if ~exist(opts.fileAttRepresTe_slidingwindow_meta,'file')
    if ~exist(opts.fileAttRepresTe_slidingwindow,'file')
        if strcmp(class(attModels),'single')
            W = attModels(1:end-1,:);
        else
            W = [attModels(:).W];
        end
        
        attReprTe_slidingwindow = {};
        
        if ~exist('features_slidingwindow_meta','var');
            meta = load(opts.fileFeatures_slidingwindow_meta,'meta');
            meta=meta.meta;
            nInBatch=[];
            curPos=1;
            %features_te_slidingwindow = [];
            for cb = 1:meta.nBatches
                idxTest_batch=data.idxTest(curPos:curPos+meta.nInBatch(cb)-1);
                curPos=curPos+meta.nInBatch(cb);
                disp(['loading features_slidingwindow batch ' num2str(cb) ' of ' num2str(meta.nBatches)]);
                load(strcat(opts.fileFeatures_slidingwindow,num2str(cb)),'featsBatch');
                %features_te_slidingwindow = [features_te_slidingwindow features_slidingwindow_X.featsBatch(idxTest_batch)];
                feats_te_slidingwindow_batch = featsBatch(idxTest_batch);
                
                attReprTe_slidingwindow_batch={};
                disp('   projecting features');
                for i  =1:size(feats_te_slidingwindow_batch,2)
                    attReprTe_slidingwindow_batch{i} = W'*feats_te_slidingwindow_batch{i};
                end
                attReprTe_slidingwindow = [attReprTe_slidingwindow attReprTe_slidingwindow_batch];
                %save(strcat(opts.fileAttRepresTe_slidingwindow,num2str(cb)),'attReprTe_slidingwindow_batch'); 
                %nInBatch(cb)=size(feats_te_slidingwindow_batch,2);
                disp(['size of batch ' num2str(cb) ' is ' num2str(size(feats_te_slidingwindow_batch,2))]);
                clear feats_batch
             end
            
            %meta.nInBatch=nInBatch;
            %save(opts.fileAttRepresTe_slidingwindow_meta,'meta');
            
            %features_slidingwindowX=load(opts.fileFeatures_slidingwindow,'featsBatch');
            %features_slidingwindow=features_slidingwindowX.featsBatch;
        end
        
        %already selecting
        %feats_te_slidingwindow = features_slidingwindow(data.idxTest);
        
        
        %attReprTe_slidingwindow = {};
        %for i  =1:size(feats_te_slidingwindow,2)
        %    attReprTe_slidingwindow{i} = W'*feats_te_slidingwindow{i};
        %end

        save(opts.fileAttRepresTe_slidingwindow,'attReprTe_slidingwindow'); 
    end
end
