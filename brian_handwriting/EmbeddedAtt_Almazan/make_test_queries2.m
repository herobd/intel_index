x = load('data/GW_words_indexes_sets_fold1.mat');
fid = fopen('datasets/GW/queries/queries.gtp');
out = fopen('queries_test.gtp','w');
out2 = fopen('queries_train.gtp','w');

idx=1;
tline = fgetl(fid);

while ischar(tline)
    if (x.idxTest(idx))
        fprintf(out,'%s\n',tline);
    else
        fprintf(out2,'%s\n',tline);
    end
    idx = idx+1;
    tline = fgetl(fid);
    
end

fclose(fid);
fclose(out);
fclose(out2);
