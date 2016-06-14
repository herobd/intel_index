x = load('data/GW_words_indexes_sets_fold1.mat');
fid = fopen('../../data/gw_20p_wannot/annotations.txt');
out = fopen('../../data/gw_20p_wannot/Almazan_annotations.txt','w');

idx=1;
tline = fgetl(fid);

while ischar(tline)
    if (x.idxTest(idx))
        fprintf(out,'%s\n',tline);
    end
    idx = idx+1;
    tline = fgetl(fid);
    
end

fclose(fid);
fclose(out);