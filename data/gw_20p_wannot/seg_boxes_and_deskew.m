% seg_boxes_and_deskew (outdir)
%
% Segments the page images in the current working directory and
% writes word images to the folder outdir, which has to be created
% beforehand.

% Author: Toni M. Rath (trath@cs.umass.edu) 08/2004

function seg_boxes_and_deskew (outdir)
f=textread('file_order_TEST.txt','%s');

c=1;
for i=1:length(f)
  n=f{i};
  img=imread(n);
  slant=findSlant(n);
  [h,w]=size(img);
  bf=sprintf('%s_boxes.txt',n(1:end-4));
  b=textread(bf,'','headerlines',1);
  % scale relative coordinates
  b(:,1)=round(b(:,1)*(w-1)+1);
  b(:,2)=round(b(:,2)*(w-1)+1);
  b(:,3)=round(b(:,3)*(h-1)+1);
  b(:,4)=round(b(:,4)*(h-1)+1);
  for j=1:size(b,1)
    wimg=img(b(j,3):b(j,4),b(j,1):b(j,2));
    imwrite(wimg, sprintf('tmp/wordimg_%d.tif',c));
    clean_and_shear(sprintf('wordimg_%d',c),outdir,slant);
    c=c+1;
  end
end
