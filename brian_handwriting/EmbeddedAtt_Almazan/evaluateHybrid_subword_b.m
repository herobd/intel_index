function hybrid_map = evaluateHybrid_subword(opts,DATA,embedding)

fprintf('\n');
disp('**************************************');
disp('*******  Hybrid KCSR _subword  ******');
disp('**************************************');

%data.attReprTe_slidingwindow

matx = embedding.rndmatx(1:embedding.M,:);
maty = embedding.rndmaty(1:embedding.M,:);


tmp_subword = matx*DATA.attReprQu_subword;
attReprQu_emb_subword = 1/sqrt(embedding.M) * [ cos(tmp_subword); sin(tmp_subword)];
tmp_subword = maty*DATA.phocsQu_subword;
phocsQu_emb_subword = 1/sqrt(embedding.M) * [ cos(tmp_subword); sin(tmp_subword)];

%phocsTe_emb=bsxfun(@minus, phocsTe_emb, embedding.mphocs);
attReprQu_emb_subword=bsxfun(@minus, attReprQu_emb_subword, embedding.matts);
phocsQu_emb_subword=bsxfun(@minus, phocsQu_emb_subword, embedding.mphocs);

%phocsTe_cca = embedding.Wy(:,1:embedding.K)' * phocsTe_emb;
attReprQu_cca_subword = embedding.Wx(:,1:embedding.K)' * attReprQu_emb_subword;
phocsQu_cca_subword = embedding.Wy(:,1:embedding.K)' * phocsQu_emb_subword;

%phocsTe_cca = (bsxfun(@rdivide, phocsTe_cca, sqrt(sum(phocsTe_cca.*phocsTe_cca))));
attReprQu_cca_subword = (bsxfun(@rdivide, attReprQu_cca_subword, sqrt(sum(attReprQu_cca_subword.*attReprQu_cca_subword))));
phocsQu_cca_subword = (bsxfun(@rdivide, phocsQu_cca_subword, sqrt(sum(phocsQu_cca_subword.*phocsQu_cca_subword))));

if ~exist('meta','var');
    load(opts.fileAttRepresTe_slidingwindow_meta,'meta');
end    
for cb = 1:meta.nBatches
    disp(['loading  AttRepresTe for batch ' num2str(cb)]);
    load(strcat(opts.fileAttRepresTe_slidingwindow,num2str(cb)),'attReprTe_slidingwindow_batch');
    %tmp = matx*DATA.attReprTe;
    tmp={};
    for i = 1:size(attReprTe_slidingwindow_batch,2)
        tmp{i}=matx*attReprTe_slidingwindow_batch{i};
    end

    %attReprTe_emb = 1/sqrt(embedding.M) * [ cos(tmp); sin(tmp)];
    attReprTe_emb_slidingwindow={};
    for i = 1:size(tmp,2)
        attReprTe_emb_slidingwindow{i} = 1/sqrt(embedding.M) * [ cos(tmp{i}); sin(tmp{i})];
    end

    %tmp = maty*DATA.phocsTe;
    %phocsTe_emb = 1/sqrt(embedding.M) * [ cos(tmp); sin(tmp)];



    % Mean center
    %attReprTe_emb=bsxfun(@minus, attReprTe_emb, embedding.matts);
    for i = 1:size(attReprTe_emb_slidingwindow,2)
        attReprTe_emb_slidingwindow{i} = bsxfun(@minus, attReprTe_emb_slidingwindow{i}, embedding.matts);
    end



    % Embed test
    %attReprTe_cca = embedding.Wx(:,1:embedding.K)' * attReprTe_emb;
    for i = 1:size(attReprTe_emb_slidingwindow,2)
        attReprTe_cca_slidingwindow{i} = embedding.Wx(:,1:embedding.K)' * attReprTe_emb_slidingwindow{i};
    end



    % L2 normalize (critical)
    %attReprTe_cca = (bsxfun(@rdivide, attReprTe_cca, sqrt(sum(attReprTe_cca.*attReprTe_cca))));
    for i = 1:size(attReprTe_cca_slidingwindow,2)
        attReprTe_cca_slidingwindow{i} = (bsxfun(@rdivide, attReprTe_cca_slidingwindow{i}, sqrt(sum(attReprTe_cca_slidingwindow{i}.*attReprTe_cca_slidingwindow{i}))));
    end
    
    save(strcat(opts.fileAttRepresTe_cca_slidingwindow,num2str(cb)),'attReprTe_cca_slidingwindow');
end
clear('attReprTe_cca_slidingwindow');
clear('attReprTe_emb_slidingwindow');
clear('tmp');


% Evaluate
% disp('evaluating...');
alpha = 0:0.1:1;
hybrid_map = zeros(length(alpha),1);
hybrid_p1 = zeros(length(alpha),1);
hybrid_thresh = zeros(length(alpha),1);
for i=1:length(alpha)
% 	disp('looping s');
    attReprQu_hybrid_subword = attReprQu_cca_subword*alpha(i) + phocsQu_cca_subword*(1-alpha(i));
%     disp(['eval ', num2str(i)]);
    [p1,mAPEucl,thresh] = eval_dp_asymm_subword(alpha(i),DATA, opts, attReprQu_hybrid_subword, DATA.wordClsTe_subword, DATA.wordClsQu_subword, DATA.ngramIdx);
%     disp('exit eval');
    hybrid_map(i) = mean(mAPEucl)*100;
    hybrid_p1(i) = mean(p1)*100;
    hybrid_thresh(i) = thresh;
%     disp('looping e');
end
% disp('finished alpha eval');
[best_map,idx] = max(hybrid_map);
best_p1 = hybrid_p1(idx);
best_alpha = alpha(idx);
best_thresh = hybrid_thresh(idx);

% Display info
disp('------------------------------------');
fprintf('alpha: %.2f reg: %.8f. k: %d\n', best_alpha, embedding.reg, embedding.K);
fprintf('hybrid --   test: (map: %.2f. p@1: %.2f) thresh=%.8f\n',  best_map, best_p1,best_thresh);
disp('------------------------------------');

plot(alpha,hybrid_map,'.-','MarkerSize',16)
title(opts.dataset)
xlabel('alpha')
ylabel('Mean Average Precision (%)')
grid on

end
