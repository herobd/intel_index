%  localMins2D  Get the significant local minimum elements and indices from a 2d matrix
%
%  function [ms,is,js] = localMins2d(M)

function [ms,is,js] = localMins2d(M)

[m1,i1] = min(M,[],1);
[m,j] = min(m1,[],2);
% i = i1(j);

% minFilter = [1 1 1 1 1
%              1 1 1 1 1
%              1 1 -24 1 1
%              1 1 1 1 1
%              1 1 1 1 1];
% minFilter = [1 1 1 1 1
%              1 0 0 0 1
%              1 0 -16 0 1
%              1 0 0 0 1
%              1 1 1 1 1];

% minFilter = [1 1 1
%              1 -8 1
%              1 1 1];
ms=[];
js=[];
is=[];
% R = conv2(M,minFilter);
% for i = 3:(size(R,1)-3)
%     for j = 3:(size(R,2)-3)
%         if (R(i,j) >0 && M(i,j)<m*2)
% %             disp('min at')
% %             [i,j]
% %             R(i,j)
%             ms = [ms,M(i,j)];
%             js = [js,j];
%             is = [is,i];
%         end
%     end
% end


for i = 3:(size(M,1)-3)
    for j = 3:(size(M,2)-3)
        if (M(i,j)<m*1.5 && M(i,j)<=M(i-1,j) && M(i,j)<=M(i+1,j) && M(i,j)<=M(i,j-1) && M(i,j)<=M(i,j+1) )
%             disp('min at')
%             [i,j]
%             R(i,j)
            ms = [ms,M(i,j)];
            js = [js,j];
            is = [is,i];
            
        end
    end
end