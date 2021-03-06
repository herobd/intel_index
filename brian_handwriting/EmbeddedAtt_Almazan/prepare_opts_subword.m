function opts = prepare_opts_subword(dataset,typegrams)

%where typegrams is 'bigrams' or 'trigrams'

% Adjustable paths
% Select the disk location of your datasets
opts.path_datasets = 'datasets';
% Path where the generated files will be saved
opts.pathData = '~/intel_index/brian_handwriting/EmbeddedAtt_Almazan/data/watts';
% Select the dataset
if nargin < 1
    dataset = 'IAM';
end
opts.dataset = dataset;
opts.saveLineSpottings = false;
if strcmp(dataset, 'IAM_lines')
    dataset='IAM'
    opts.saveLineSpottings = true;
end
opts.precForThresh = 0.5;
opts.recallForThresh = 0.5;
opts.threshold = 0.52284769;%??0.4504;
opts.saveSpottings = true;


% Adding all the necessary libraries and paths
addpath('util/');
if ~exist('util/bin','dir')
    mkdir('util/bin');
end
addpath('util/bin');
addpath('util/io');
if ~exist('calib_c','file')
    mex -output util/bin/calib_c -O -largeArrayDims util/calib_c.c
end
if ~exist('computeStats_c_subword','file')
    mex -output util/bin/computeStats_c_subword -O -largeArrayDims  CFLAGS="\$CFLAGS -std=c99" util/computeStats_c_subword.c
%     mex -output util/bin/computeStats_c_subword -g -largeArrayDims  CFLAGS="\$CFLAGS -std=c99" util/computeStats_c_subword.c
end
if ~exist('computeStats_c','file')
    mex -output util/bin/computeStats_c -O -largeArrayDims  CFLAGS="\$CFLAGS -std=c99" util/computeStats_c.c
end
if ~exist('phoc_mex','file')
    mex -output util/bin/phoc_mex -O -largeArrayDims util/phoc_mex.cpp
end
if ~exist('levenshtein_c','file')
    mex -output util/bin/levenshtein_c -O -largeArrayDims util/levenshtein_c.c
end
if ~exist('util/vlfeat-0.9.20/toolbox/mex','dir')
    if isunix
        cd 'util/vlfeat-0.9.20/';
        mexloc = fullfile(matlabroot,'bin/mex');
        % This is necessary to include support to OpenMP in Mavericks+XCode5
        % gcc4.2 can be installed from MacPorts
        %if strcmpi(computer,'MACI64')
        %   system(sprintf('make MEX=%s CC=/opt/local/bin/gcc-apple-4.2',mexloc));
        %else
        system(sprintf('make MEX=%s',mexloc));
        %end
        cd ../..;
    else
        run('util/vlfeat-0.9.20/toolbox/vl_compile');
    end
end


run('util/vlfeat-0.9.20/toolbox/vl_setup')

% Set random seed to default
rng('default');

opts.pathDataset = sprintf('%s/%s/',opts.path_datasets,dataset);
opts.pathImages = sprintf('%s/%s/images/',opts.path_datasets,dataset);
opts.pathDocuments = sprintf('%s/%s/documents/',opts.path_datasets,dataset);
opts.pathQueries = sprintf('%s/%s/queries/',opts.path_datasets,opts.dataset);

opts.pathNgrams = sprintf('%s/%s/%s/',opts.path_datasets,dataset,typegrams);

if strcmp(typegrams,'bigrams')
    opts.pathNgramLocationsTop = sprintf('%s/%s/bigramLocationsTop10.csv',opts.path_datasets,dataset);    
else
    opts.pathNgramLocationsTop = sprintf('%s/%s/trigramLocationsTop5.csv',opts.path_datasets,dataset);
end

% Options FV features
opts.numWordsTrainGMM = 500;
opts.SIFTDIM = 128;
opts.PCADIM = 62;
opts.numSpatialX = 6;
opts.numSpatialY = 2;
opts.G = 16;
opts.phowOpts = {'Verbose', false, 'Step', 3, 'FloatDescriptors', true, 'sizes',[2,4,6,8,10,12]} ;
opts.doMinibox = 1;
opts.minH = -1;
opts.maxH = 99999;
opts.fold = -1;

% Options PHOC attributes
opts.levels = [2 3 4 5];
opts.levelsB = [2];
opts.numBigrams = 50;
fid = fopen('data/bigrams.txt','r');
bgrams = textscan(fid,'%s');
fclose(fid);
opts.bgrams = bgrams{1}(1:opts.numBigrams);
opts.unigrams = 'abcdefghijklmnopqrstuvwxyz';
opts.digits='0123456789';
opts.considerDigits = 1;

% Options learning models
opts.bagging = 1;
opts.cluster = 0;
opts.sgdparams.eta0s = single([1]);
opts.sgdparams.lbds = single([1e-3,1e-4,1e-5]);
opts.sgdparams.betas = int32([32,64,80]);
opts.sgdparams.bias_multipliers = single([1]);
opts.sgdparams.epochs = 75;
opts.sgdparams.eval_freq = 2;
opts.sgdparams.t = 0;
opts.sgdparams.weightPos = 1;
opts.sgdparams.weightNeg = 1;

% Options embedding
opts.RemoveStopWords = 0;
opts.TestFV = 0;
opts.TestDirect = 0;

opts.TestPlatts = 0;
opts.Platts.verbose = 0;

opts.TestRegress = 0;
opts.Reg.Reg = [1e-1,1e-2,1e-3,1e-4];
opts.Reg.verbose = 1;

opts.TestCCA = 1;
opts.CCA.Dims = [96];
opts.CCA.Reg = [1e-4,1e-5,1e-6];
opts.CCA.verbose = 0;

opts.TestKCCA = 1;
opts.KCCA.M = [2500];
opts.KCCA.G = [40];
opts.KCCA.Dims = [160];
opts.KCCA.Reg = [1e-5];
opts.KCCA.verbose = 1;

opts.evalRecog = 0;
opts.TestHybrid = 1;

% Specific dataset options
if strcmp(opts.dataset,'GW')
    opts.fold = 1;
    opts.minH = 80;
    opts.maxH = 80;
    opts.windowStride=15;
    if strcmp(typegrams, 'bigrams')
        opts.ngrams = {'th' 'he', 'er', 'an', 're', 'on', 'at', 'or', 'es', 'en', 'te', 'nd', 'ed', 'ar', 'to', 'ti', 'st', 'ng', 'nt', 'it'};
        opts.ngramCountPer=10;
        %opts.windowWidth=135;%should be wide
        opts.windowWidth=75;%modified 66?
    elseif strcmp(typegrams, 'bigrams_100')
        opts.TestHybrid = 0;
        opts.ngrams = {'th', 'he', 'in', 'er', 'an', 're', 'on', 'at', 'en', 'nd', 'ti', 'es', 'or', 'te', 'of', 'ed', 'is', 'it', 'al', 'ar', 'st', 'to', 'nt', 'ng', 'se', 'ha', 'as', 'ou', 'io', 'le', 've', 'co', 'me', 'de', 'hi', 'ri', 'ro', 'ic', 'ne', 'ea', 'ra', 'ce', 'li', 'ch', 'll', 'be', 'ma', 'si', 'om', 'ur', 'ca', 'el', 'ta', 'la', 'ns', 'di', 'fo', 'ho', 'pe', 'ec', 'pr', 'no', 'ct', 'us', 'ac', 'ot', 'il', 'tr', 'ly', 'nc', 'et', 'ut', 'ss', 'so', 'rs', 'un', 'lo', 'wa', 'ge', 'ie', 'wh', 'ee', 'wi', 'em', 'ad', 'ol', 'rt', 'po', 'we', 'na', 'ul', 'ni', 'ts', 'mo', 'ow', 'pa', 'im', 'mi', 'ai', 'sh'};
        
        opts.ngramCountPer=1;
        opts.windowWidth=75;%modified 66?
    elseif strcmp(typegrams, 'unigrams_26')
        opts.TestHybrid = 0;
        opts.ngrams = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
        
        opts.ngramCountPer=1;
        opts.windowWidth=35;
    elseif strcmp(typegrams, 'bigrams_20')
        opts.TestHybrid = 0;
        opts.ngrams = {'th' 'he', 'er', 'an', 're', 'on', 'at', 'or', 'es', 'en', 'te', 'nd', 'ed', 'ar', 'to', 'ti', 'st', 'ng', 'nt', 'it'};
        opts.ngramCountPer=1;
        opts.windowWidth=75;%modified 66?
    else
        opts.ngrams = {'the', 'and', 'ing', 'ion', 'ent', 'her', 'for', 'hat', 'his', 'tha'};
        opts.ngramCountPer=5;
        %opts.windowWidth=187;
        opts.windowWidth=120;
    end
elseif strcmp(dataset,'IAM')
    opts.PCADIM = 30;
    opts.RemoveStopWords = 0;
    opts.swFile = 'data/swIAM.txt';
    opts.minH = 80;
    opts.maxH = 80;
    opts.windowStride=5;
    if strcmp(typegrams, 'bigrams')
        opts.ngrams = {'TH', 'HE', 'ER', 'AN', 'RE', 'ON', 'AT', 'OR', 'ES', 'EN', 'TE', 'ND', 'ED', 'AR', 'TO', 'TI', 'ST', 'NG', 'NT', 'IT'};
        opts.ngramCountPer=10;
        opts.windowWidth=60;%90;
    elseif strcmp(typegrams, 'bigrams_100')
        opts.TestHybrid = 0;
        opts.ngrams = {'TH', 'HE', 'IN', 'ER', 'AN', 'RE', 'ON', 'AT', 'EN', 'ND', 'TI', 'ES', 'OR', 'TE', 'OF', 'ED', 'IS', 'IT', 'AL', 'AR', 'ST', 'TO', 'NT', 'NG', 'SE', 'HA', 'AS', 'OU', 'IO', 'LE', 'VE', 'CO', 'ME', 'DE', 'HI', 'RI', 'RO', 'IC', 'NE', 'EA', 'RA', 'CE', 'LI', 'CH', 'LL', 'BE', 'MA', 'SI', 'OM', 'UR', 'CA', 'EL', 'TA', 'LA', 'NS', 'DI', 'FO', 'HO', 'PE', 'EC', 'PR', 'NO', 'CT', 'US', 'AC', 'OT', 'IL', 'TR', 'LY', 'NC', 'ET', 'UT', 'SS', 'SO', 'RS', 'UN', 'LO', 'WA', 'GE', 'IE', 'WH', 'EE', 'WI', 'EM', 'AD', 'OL', 'RT', 'PO', 'WE', 'NA', 'UL', 'NI', 'TS', 'MO', 'OW', 'PA', 'IM', 'MI', 'AI', 'SH'};
        opts.ngramCountPer=1;
        opts.windowWidth=60;%90;
    else
        opts.ngrams = {'THE', 'AND', 'ING', 'ION', 'ENT', 'HER', 'FOR', 'HAT', 'HIS', 'THA'};
        opts.ngramCountPer=5;
        opts.windowWidth=115;%170;
    end
elseif strcmp(opts.dataset,'IIIT5K')
    opts.minH = 80;
    opts.maxH = 80;
    opts.doMinibox = 0;
elseif strcmp(opts.dataset,'SVT')
    opts.minH = 80;
    opts.maxH = 80;
    opts.doMinibox = 0;
elseif strcmp(opts.dataset,'ICDAR11')
    opts.minH = 80;
    opts.maxH = 80;
    opts.doMinibox = 0;
elseif strcmp(opts.dataset,'ICDAR03')
    opts.minH = 80;
    opts.maxH = 80;
    opts.doMinibox = 0;
elseif strcmp(opts.dataset,'LP')
    opts.minH = 80;
    opts.maxH = 80;
    opts.doMinibox = 0;
end

if opts.saveSpottings %colors!
    opts.pathResults = 'results';
end

opts.FVdim = (opts.PCADIM+2)*opts.numSpatialX*opts.numSpatialY*opts.G*2;

if opts.evalRecog || opts.TestHybrid
    opts.TestKCCA = 1;
end

% Tags
tagminH = '';
if opts.minH > -1 || opts.maxH < 99999
    tagminH = sprintf('_minH%d_maxH%d',opts.minH, opts.maxH);
end
tagFold = '';
if opts.fold > -1
    tagFold = sprintf('_fold%d',opts.fold);
end
tagPCA = sprintf('_PCA%d',opts.PCADIM);
tagGMM = sprintf('_GMM%dx%dx%dx%d%s',opts.G,opts.numSpatialY,opts.numSpatialX,opts.PCADIM+2,tagminH);
tagLevels = ['_l' strrep(strrep(strrep(mat2str(opts.levels),' ',''),'[',''),']','')];
tagLevelsB = ['_lb' strrep(strrep(strrep(mat2str(opts.levelsB),' ',''),'[',''),']','')];
tagNumB = sprintf('_nb%d',opts.numBigrams);
if opts.bagging
    tagBagging = '_bagging';
else
    tagBagging = '_noBagging';
end
tagFeats = '_FV';

opts.tagPHOC = sprintf('_PHOCs%s%s%s',tagLevels,tagLevelsB,tagNumB);
opts.tagFeatures = sprintf('%s%s%s%s_%d',tagFeats,tagPCA,tagGMM,tagFold,opts.windowWidth);

% Paths and files
opts.pathFiles = sprintf('%s/files',opts.pathData);
if ~exist(opts.pathData,'dir')
    mkdir(opts.pathData);
end
opts.dataFolder = sprintf('%s/%s%s%s',opts.pathFiles,opts.dataset,opts.tagPHOC,opts.tagFeatures);
if ~exist(opts.dataFolder,'dir')
    mkdir(opts.dataFolder);
end
opts.fileData = sprintf('%s/%s_data.mat',opts.pathFiles,opts.dataset);
opts.fileImages = sprintf('%s/%s_images%s.bin',opts.pathFiles,opts.dataset,tagminH);
opts.fileImages_subword = sprintf('%s/%s_images_subword%s_%s.bin',opts.pathFiles,opts.dataset,tagminH,typegrams);
opts.fileWriters = sprintf('%s/%s_writers.mat',opts.pathFiles,opts.dataset);
opts.fileGMM = sprintf('%s/%s%s.bin',opts.dataFolder,opts.dataset,tagGMM);
opts.filePCA = sprintf('%s/%s%s.bin',opts.dataFolder,opts.dataset,tagPCA);
opts.filePHOCs = sprintf('%s/%s%s.bin',opts.dataFolder,opts.dataset,opts.tagPHOC);
opts.filePHOCs_subword = sprintf('%s/%s%s_subword_%s.bin',opts.dataFolder,opts.dataset,opts.tagPHOC,typegrams);
opts.fileFeatures = sprintf('%s/%s%s.bin',opts.dataFolder,opts.dataset,opts.tagFeatures);
opts.fileFeatures_slidingwindow_meta = sprintf('%s/%s%s_slidingwindow_meta.mat',opts.dataFolder,opts.dataset,opts.tagFeatures);
opts.fileFeatures_slidingwindow = sprintf('%s/%s%s_slidingwindow_',opts.dataFolder,opts.dataset,opts.tagFeatures);
opts.fileFeatures_subword = sprintf('%s/%s%s_subword.bin',opts.dataFolder,opts.dataset,opts.tagFeatures);
opts.fileAttModels = sprintf('%s/%s_attModels%s%s%s.bin',opts.dataFolder,opts.dataset,opts.tagPHOC,opts.tagFeatures,tagBagging);
opts.fileAttRepresTr = sprintf('%s/%s_attRepresTr%s%s%s.bin',opts.dataFolder,opts.dataset,opts.tagPHOC,opts.tagFeatures,tagBagging);
opts.fileAttRepresVal = sprintf('%s/%s_attRepresVal%s%s%s.bin',opts.dataFolder,opts.dataset,opts.tagPHOC,opts.tagFeatures,tagBagging);
opts.fileAttRepresTe = sprintf('%s/%s_attRepresTe%s%s%s.bin',opts.dataFolder,opts.dataset,opts.tagPHOC,opts.tagFeatures,tagBagging);
opts.fileAttRepresTe_slidingwindow = sprintf('%s/%s_attRepresTe_slidingwindow%s%s%s_',opts.dataFolder,opts.dataset,opts.tagPHOC,opts.tagFeatures,tagBagging);
opts.fileAttRepresTe_slidingwindow_meta = sprintf('%s/%s_attRepresTe_slidingwindow%s%s%s_meta.mat',opts.dataFolder,opts.dataset,opts.tagPHOC,opts.tagFeatures,tagBagging);
opts.fileAttRepresTe_cca_slidingwindow = sprintf('%s/%s_attRepresTe_cca_slidingwindow%s%s%s_',opts.dataFolder,opts.dataset,opts.tagPHOC,opts.tagFeatures,tagBagging);
opts.fileAttRepresQu_subword = sprintf('%s/%s_attRepresQu_subword%s%s%s.bin',opts.dataFolder,opts.dataset,opts.tagPHOC,opts.tagFeatures,tagBagging);
opts.folderModels = sprintf('%s/models%s/',opts.dataFolder,tagBagging);
opts.modelsLog = sprintf('%s/learning.log',opts.folderModels);

opts.spottingsFile = [sprintf('data/%s_spottings%s_',opts.dataset,tagFold) '%f.csv'];
opts.agSpottingsFile = [sprintf('data/%s_agSpottings%s_',opts.dataset,tagFold) '%f.csv'];
opts.lineSpottingsFile = [sprintf('data/%s_lineSpottings%s_',opts.dataset,tagFold) '%f.csv'];

if ~exist(opts.folderModels,'dir')
    mkdir(opts.folderModels);
end
opts.fileSets = sprintf('data/%s_words_indexes_sets%s.mat',opts.dataset,tagFold);
opts.fileLexicon = sprintf('%s/%s_lexicon%s.mat',opts.pathFiles,opts.dataset,opts.tagPHOC);
end
