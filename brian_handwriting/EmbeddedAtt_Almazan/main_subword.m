%% Word Spotting and Recognition with Embedded Attributes
% Authors: Jon Almazan and Albert Gordo
% Contact: almazan@cvc.uab.es

%TODO: load_{dataset}.m files are where the images should be read in (to
%data.words_subword?)
%extract_lexicon.m?

%%VLFeat stuff
% vl_setup('verbose');

%% Prepare options and read dataset
opts = prepare_opts_subword('IAM','trigrams');
data = load_dataset(opts);
data = load_subword(opts,data);

%% Prepare images
prepare_images(opts,data);
prepare_images_subword(opts,data);

%% Embed text labels with PHOC
data.phocs = embed_labels_PHOC(opts,data);
data.phocs_subword = embed_labels_PHOC_subword(opts,data);

%% Extract features from images
extract_features_subword(opts);

%% Split data into sets
data = prepare_data_learning_subword(opts,data);

%% Learn PHOC attributes
data.att_models = learn_attributes_subword(opts,data);

%% Learn common subspaces and/or calibrations
[embedding,mAPsval] = learn_common_subspace(opts,data);

%% Evaluate
mAPstest = evaluate_subword(opts,data,embedding);

%% Save model
%save_model(opts,data,embedding);
