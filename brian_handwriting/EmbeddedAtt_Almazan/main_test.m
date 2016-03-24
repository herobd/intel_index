%% Word Spotting and Recognition with Embedded Attributes
% Authors: Jon Almazan and Albert Gordo
% Contact: almazan@cvc.uab.es
%% Prepare options and read dataset
opts = prepare_opts('GW');
data = load_dataset(opts);

%% Prepare images
prepare_images(opts,data);

%% Embed text labels with PHOC
data.phocs = embed_labels_PHOC(opts,data);

%% Extract features from images
extract_features_test(opts,data);

%% Split data into sets
data = prepare_data_learning_test(opts,data);

%% Learn PHOC attributes
data.att_models = learn_attributes_test(opts,data);

%% Learn common subspaces and/or calibrations
[embedding,mAPsval] = learn_common_subspace_test(opts,data);

%% Evaluate
%mAPstest = evaluate(opts,data,embedding);

%% Save model
% save_model(opts,data,embedding);
