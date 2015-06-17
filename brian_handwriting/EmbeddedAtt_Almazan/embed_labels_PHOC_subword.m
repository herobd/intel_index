function phocs_subword = embed_labels_PHOC_subword(opts,data)
% Computes the PHOC embedding for every word label in the dataset
disp('* Computing PHOC histograms *');
if  ~exist(opts.filePHOCs_subword,'file')
    voc = opts.unigrams;
    if opts.considerDigits
        voc = [voc opts.digits];
    end
    str2cell = @(x) {char(x)};
    voc = arrayfun(str2cell, voc);
    
    lf = @(x) lower(x.gttext);
    W = arrayfun(lf, data.words_subword,'UniformOutput', false);
        
    phocsuni = phoc_mex(W, voc, int32(opts.levels));
    if opts.numBigrams>0
        phocsbi = phoc_mex(W, opts.bgrams, int32(opts.levelsB));
    else
        phocsbi = [];
    end
    phocs_subword = [phocsuni;phocsbi];   
    writeMat(single(phocs_subword), opts.filePHOCs_subword);    
else
    phocs_subword = readMat(opts.filePHOCs_subword);
end

end
