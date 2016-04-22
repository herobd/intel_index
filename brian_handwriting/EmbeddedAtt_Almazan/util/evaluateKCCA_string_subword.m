function  mAP = evaluateKCCA(opts,DATA,embedding)
% Evaluate KCCA.

fprintf('\n');
disp('**************************************');
disp('***by string***  KCSR   **************');
disp('**************************************');

matx = embedding.rndmatx(1:embedding.M,:);
maty = embedding.rndmaty(1:embedding.M,:);

%%%%%%%%%%%%%%%%
%tmp = matx*DATA.attReprTe;
tmp={};
for i = 1:size(DATA.attReprTe_slidingwindow,2)
    tmp{i}=matx*DATA.attReprTe_slidingwindow{i};
end

%attReprTe_emb = 1/sqrt(embedding.M) * [ cos(tmp); sin(tmp)];
attReprTe_emb_slidingwindow={};
for i = 1:size(tmp,2)
    attReprTe_emb_slidingwindow{i} = 1/sqrt(embedding.M) * [ cos(tmp{i}); sin(tmp{i})];
end

%tmp = maty*DATA.phocsTe;
%phocsTe_emb = 1/sqrt(embedding.M) * [ cos(tmp); sin(tmp)];

tmp_subword = maty*DATA.phocsQu_subword;
phocsQu_emb_subword = 1/sqrt(embedding.M) * [ cos(tmp_subword); sin(tmp_subword)];

% Mean center
%attReprTe_emb=bsxfun(@minus, attReprTe_emb, embedding.matts);
for i = 1:size(attReprTe_emb_slidingwindow,2)
    attReprTe_emb_slidingwindow{i} = bsxfun(@minus, attReprTe_emb_slidingwindow{i}, embedding.matts);
end

%phocsTe_emb=bsxfun(@minus, phocsTe_emb, embedding.mphocs);
phocsQu_emb_subword=bsxfun(@minus, phocsQu_emb_subword, embedding.mphocs);

% Embed test
%attReprTe_cca = embedding.Wx(:,1:embedding.K)' * attReprTe_emb;
for i = 1:size(attReprTe_emb_slidingwindow,2)
    attReprTe_cca_slidingwindow{i} = embedding.Wx(:,1:embedding.K)' * attReprTe_emb_slidingwindow{i};
end

%phocsTe_cca = embedding.Wy(:,1:embedding.K)' * phocsTe_emb;
phocsQu_cca_subword = embedding.Wy(:,1:embedding.K)' * phocsQu_emb_subword;

% L2 normalize (critical)
%attReprTe_cca = (bsxfun(@rdivide, attReprTe_cca, sqrt(sum(attReprTe_cca.*attReprTe_cca))));
for i = 1:size(attReprTe_cca_slidingwindow,2)
    attReprTe_cca_slidingwindow{i} = (bsxfun(@rdivide, attReprTe_cca_slidingwindow{i}, sqrt(sum(attReprTe_cca_slidingwindow{i}.*attReprTe_cca_slidingwindow{i}))));
end

%phocsTe_cca = (bsxfun(@rdivide, phocsTe_cca, sqrt(sum(phocsTe_cca.*phocsTe_cca))));
phocsQu_cca_subword = (bsxfun(@rdivide, phocsQu_cca_subword, sqrt(sum(phocsQu_cca_subword.*phocsQu_cca_subword))));

%%%%%%%%%%%%%%%%%%%%%%





% QBS
[p1,mAPEucl, threshPrec,threshRecall,precAtRecall] = eval_dp_asymm_subword(0,DATA,opts,phocsQu_cca_subword,attReprTe_cca_slidingwindow, DATA.wordClsTe_subword, DATA.wordClsQu_subword, DATA.ngramIdx);
qbs_test_map = mean(mAPEucl);
qbs_test_p1 = mean(p1);


% Display info
disp('------------------------------------');
fprintf('reg: %.8f. k: %d\n',  embedding.reg, embedding.K);
fprintf('QbS --   test: (map: %.2f. p@1: %.2f)\n',  100*qbs_test_map, 100*qbs_test_p1);
fprintf('thresh at %.3f prec is %.8f\n', opts.precForThresh, threshPrec);
fprintf('thresh at %.3f recall is %.8f\n', opts.recallForThresh, threshRecall);
fprintf('prec at %.3f recall is %.8f\n', opts.recallForThresh, precAtRecall);
disp('------------------------------------');

mAP.qbe = -1 %100*qbe_test_map;
mAP.qbs = 100*qbs_test_map;
end
