% seg_boxes_and_deskew (outdir)
%
% Segments the page images in the current working directory and
% writes word images to the folder outdir, which has to be created
% beforehand.

% Author: Toni M. Rath (trath@cs.umass.edu) 08/2004

function deslant_ngrams (indir,outdir)
f=textread('file_order.txt','%s');
sumSlant=0;
num=1;
c=1;
for i=1:num
  n=f{i};
  img=imread(n);
  sumSlant= sumSlant+findSlant(n);
  
end

slant = sumSlant/num;

%imgs=[];
cd(indir)
files = dir();
for file = files'
    if ~strcmp(file.name,'.') && ~strcmp(file.name,'..') && file.isdir==1
        cd(file.name)
        exs = dir('*.png');
        for ex = exs'
            imgPath = [indir '/' file.name '/'];
            thisOutDir=[outdir '/' file.name '/'];
            cd ../..
            mkdir(thisOutDir);
            shear_fill(imgPath,ex.name,thisOutDir,slant);
            cd ([indir '/' file.name]);
            %imgs = [imgs {imgPath}];
        end
        cd ..
    end
end
cd ..

%for i=1:length(imgs)
%    img = imgs{i};
%    
%end

