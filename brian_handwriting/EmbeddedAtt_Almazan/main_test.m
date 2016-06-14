%% Word Spotting and Recognition with Embedded Attributes
% Authors: Jon Almazan and Albert Gordo
% Contact: almazan@cvc.uab.es
%% Prepare options and read dataset
opts = prepare_opts_test2('GW');
%opts.KCCA.M=[200];
%opts.KCCA.Dims=[50];
data = load_dataset(opts);

%% Prepare images
prepare_images(opts,data);

%% Embed text labels with PHOC
data.phocs = embed_labels_PHOC(opts,data);

%% Extract features from images
data=extract_features_test(opts,data);

%% Split data into sets
data = prepare_data_learning_test(opts,data);


%% Learn PHOC attributes
[att_models,data] = learn_attributes_test(opts,data);
data.att_models=att_models;
%% Learn common subspaces and/or calibrations
%[embedding,mAPsval,data] = learn_common_subspace_test(opts,data);

%% Evaluate
%mAPstest = evaluate(opts,data,embedding);

%% Save model
% save_model(opts,data,embedding);
