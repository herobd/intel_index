function  [embedding,mAP] = learnKCCA_test(opts,DATA)
% Evaluate KCCA.

%% Part 1: Crosvalidate to find the best parameters in the config range
fprintf('\n');
disp('**************************************');
disp('*************   CV KCSR   ************');
disp('**************************************');


% For each config, learn on Tr (small), validate on val, and keep the
% best according to QBE+QBS map. Other criterions (eg, QBE map or QBS p1) are
% possible.

bestScore = -1;
bestReg = opts.KCCA.Reg(end);
bestK = opts.KCCA.Dims(end);
bestG = opts.KCCA.G(end);
bestM = opts.KCCA.M(end);

Dx = size(DATA.attReprTr,1);
Dy = size(DATA.phocsTr,1);


% Get random matrix
%RandStream.setGlobalStream(RandStream('mt19937ar','seed',0));
rndmatx = normrnd(0,1/bestG, bestM,Dx);
matx = rndmatx(1:bestM,:);
%matx(1:bestM,1:Dx)=1/(2*bestG)
%matx(round(bestM/2):bestM,1:Dx)=-1/(1.9*bestG)
%maty(1:bestM,1:Dy)=1/(2*bestG)
%maty(round(bestM/2):bestM,1:Dy)=-1/(1.9*bestG)
rndmaty = normrnd(0,1/bestG, bestM,Dy);
maty = rndmaty(1:bestM,:);
% Embed train (full) and test
% Project attributes and phocs using the explicit exponential
% embedding. Project train, val, and test.
% train
tmp = matx*DATA.attReprTrFull;
attReprTrFull_emb = 1/sqrt(bestM) * [ cos(tmp); sin(tmp)];
tmp = maty*DATA.phocsTrFull;
phocsTrFull_emb = 1/sqrt(bestM) * [ cos(tmp); sin(tmp)];

% Mean center
ma = mean(attReprTrFull_emb,2);
attReprTrFull_emb=bsxfun(@minus, attReprTrFull_emb, ma);

mh = mean(phocsTrFull_emb,2);
phocsTrFull_emb=bsxfun(@minus, phocsTrFull_emb, mh);

% Learn CCA
[Wx,Wy,r] = cca2(attReprTrFull_emb', phocsTrFull_emb',bestReg,bestK);

mAP = 100*bestScore/2;
embedding.Wx = Wx;
embedding.Wy = Wy;
embedding.K = bestK;
embedding.M = bestM;
embedding.reg = bestReg;
embedding.rndmatx = rndmatx;
embedding.rndmaty = rndmaty;
embedding.matts = ma;
embedding.mphocs = mh;


dlmwrite('embedding_rndmatx_test.csv',rndmatx,'precision',8);
dlmwrite('embedding_rndmaty_test.csv',rndmaty,'precision',8);
dlmwrite('embedding_M_test.csv',bestM,'precision',8);
dlmwrite('embedding_matt_test.csv',ma,'precision',8);
dlmwrite('embedding_mphoc_test.csv',mh,'precision',8);
dlmwrite('embedding_Wx_test.csv',Wx,'precision',8);
dlmwrite('embedding_Wy_test.csv',Wy,'precision',8);
end
