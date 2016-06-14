function [ mat ] = readMat(f,idxs)
[fid,msg] = fopen(f, 'r');
N=fread(fid, 1, '*int32');
D=fread(fid, 1,'*int32');
curIdx=1;
idxs = sort(idxs.');
disp([num2str(size(idxs,2)) ' columns to read'])
mat = zeros(D,size(idxs,2));
count=0;
fileCount=0;
for idx = idxs
    count = count+1;
    %fread(fid, [D,idx-curIdx], '*single');
    while fileCount ~= idx
	    tmp = fread(fid, [D,1], '*single');
	    fileCount = fileCount+1;
    end
    
    %curIdx=idx;
    %tmp = fread(fid, [D,1], '*single');
    mat(:,count) =tmp;
end
assert(count==size(idxs,2));
fclose(fid);
end
