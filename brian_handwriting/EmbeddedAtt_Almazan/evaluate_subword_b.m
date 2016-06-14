function results = evaluate_subword(opts,data,embedding)

%% Load attribute representations
%attReprTe = readMat(opts.fileAttRepresTe);
%load(opts.fileAttRepresTe_slidingwindow,'attReprTe_slidingwindow');
attReprQu_subword = readMat(opts.fileAttRepresQu_subword);

%data.attReprTe = single(attReprTe);
%data.attReprTe_slidingwindow=attReprTe_slidingwindow;
data.attReprQu_subword = single(attReprQu_subword);
data.phocsTe = single(data.phocsTe);
data.phocsQu_subword = single(data.phocsQu_subword);
%data.wordClsTe = single(data.wordClsTe);

% Augment phocs with length?
%W={data.wordsTe.gttext};
%data.phocsTe = [data.phocsTe;encodeWordsLength(W,10)];

% Evaluate the retrieval task
results.retrieval = evaluate_retrieval_subword(opts,data,embedding);

% Evaluate the recognition task
% if opts.evalRecog
%     results.recognition = evaluate_recognition(opts,data,embedding);
% end

end


