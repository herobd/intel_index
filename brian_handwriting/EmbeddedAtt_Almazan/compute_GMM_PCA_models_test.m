function [GMM,PCA] = compute_GMM_PCA_models_test(opts,data,images)

descrs = {};
for i=1:length(images)
    fprintf('Word %d\n', i);
    im = images{i};
    
    %disp(['canary ' num2str(im(1,1))]);    
    % Resizes the image to a minimum height without modifying the aspect
    % ratio
    [height,width] = size(im);
    if height<opts.minH
        disp(['this shouldn;t be called, right? ' num2str(height)]);
        ar = height/width;
        height = opts.minH;
        width = round(height/ar);
        im = imresize(im, [height,width]);
    end
    
    im = im2single(im);
    %disp(['canary ' num2str(im(1,1))]);    
    % Densely extracts SIFTs at different levels
    [f,d] = vl_phow(im, opts.phowOpts{:});
    d = d/255;
    
    if i==1 && 0
     disp(['image ' num2str(i) ' size [' num2str(size(im,1)) ' ' num2str(size(im,2))]); 
     disp(data.words(i).gttext)
     disp(['image ' num2str(i+1) ' size [' num2str(size(images{i+1},1)) ' ' num2str(size(images{i+1},2))]); 
     disp(data.words(i+1).gttext)
     size(d)
     disp([num2str(f(1,1)) ', ' num2str(f(2,1)) ' : ' num2str(f(4,1)) ' = ' num2str(f(3,1))]);
     lasttt=2;
     for ii = 1:size(f,2)
       %disp(num2str(d(ii)))    
       %d(ii)    
       if (f(4,ii) ~= lasttt)
         lasttt=f(4,ii);
	 disp([num2str(f(1,ii-1)) ', ' num2str(f(2,ii-1)) ' : ' num2str(f(4,ii-1)) ' = ' num2str(f(3,ii-1))]);
         disp([num2str(f(1,ii)) ', ' num2str(f(2,ii)) ' : ' num2str(f(4,ii)) ' = ' num2str(f(3,ii))]);
       end
     end
     %disp([num2str(f(1,1)) ', ' num2str(f(2,1)) ' : ' num2str(f(4,1))]);
     disp([num2str(f(1,size(f,2))) ', ' num2str(f(2,size(f,2))) ' : ' num2str(f(4,size(f,2))) ' = ' num2str(f(3,size(f,2)))]);
    end

    if opts.doMinibox == 0
        % XY at GT coordinate space
        fx = single(f(1,:)/width-0.5);
        fy = single(f(2,:)/height-0.5);
    else
        % XY at word coordinate space
        bb = DoBB(im);
        w = bb(2)-bb(1)+1;
        h = bb(4)-bb(3)+1;
        cx = round(bb(1)+w/2);
        cy = round(bb(3)+h/2);
        fx = single((f(1,:)-cx)/w);
        fy = single((f(2,:)-cy)/h);
    end
    xy = [fx; fy];
    d = [d; xy];
    
    
    [d_norm,drop] = normalizeSift(opts,d);
    size(d)
    %saved_desc
    dlmwrite(['descs_test/GMM_PCA_desc_' num2str(i-1) '_test.csv'],d_norm,'precision',10);
    %stop
    % Assings each SIFT to a region of the spatial pyramid
    for s = 1:length(opts.numSpatialX)
        ax = linspace(-0.5,0.5,opts.numSpatialX(s)+1);
        ax(1) = -Inf; ax(end) = Inf;
        ay = [-Inf 0 Inf];
        binsx = vl_binsearch(ax,double(fx));
        binsy = vl_binsearch(ay,double(fy));
        
        for j=1:opts.numSpatialX(s)
            for k=1:opts.numSpatialY(s)
                idx = (binsx==j) & (binsy==k);
                descrs{s}{j,k}{i} = d(:,idx);
            end
        end
        
    end
end


%% Computing global PCA
% Selects a subset of normalized SIFTs
disp('* Computing PCA model *');
d = [descrs{:}];
d = [d{:}];
d = [d{:}];
[d,drop] = normalizeSift(opts,d);
numSamp = min(5000,size(d,2));
d = d(:,1:numSamp);%vl_colsubset(d, 20e5);
dlmwrite(['GMM_PCA_descs_all_test.csv'],d(1:opts.SIFTDIM,:),'precision',8);
[eigvec, m] = compute_PCA(d(1:opts.SIFTDIM,:),opts.PCADIM);
PCA.eigvec = eigvec;
PCA.mean = m;


%% Computing GMM
% Computing a GMM for every region of the spatial pyramid and concatenate
disp('* Computing GMM model *');
GMM.we =[];
GMM.mu = [];
GMM.sigma = [];
for s = 1:length(opts.numSpatialX)
    for j=1:opts.numSpatialX(s)
        for k=1:opts.numSpatialY(s)
            d = cat(2, descrs{s}{j,k}{:});
            hasNeg=0;
            for ii=1:opts.SIFTDIM
                for jj=1:size(d,2)
                    if d(ii,jj)<0
                        hasNeg=1;
                        disp(['neg at:',num2str(ii),' ',num2str(jj)])
                    end
                end
            end
            [d,drop] = normalizeSift(opts,d);
            dlmwrite(['GMM_vecs/GMM_descs_' num2str((j-1)*opts.numSpatialY(s)+(k-1)) '.csv'],d,'precision',15);
            xy = d(opts.SIFTDIM+1:end,:);
            d=bsxfun(@minus, d(1:opts.SIFTDIM,:), PCA.mean);
            d=PCA.eigvec'*d;
            
            d = [d; xy];
            dlmwrite(['GMM_vecs/GMM_vec_' num2str((j-1)*opts.numSpatialY(s)+(k-1)) '.csv'],d,'precision',15);
            vl_twister('state',0);
            if (j-1)*opts.numSpatialY(s)+(k-1)==7 || (j-1)*opts.numSpatialY(s)+(k-1)==6
                [mu,sigma,we] = vl_gmm(d, opts.G, 'MaxNumIterations', 30, 'NumRepetitions', 2, 'verbose'); 
            else
                [mu,sigma,we] = vl_gmm(d, opts.G, 'MaxNumIterations', 30, 'NumRepetitions', 2);
            end 
            if (hasNeg)
                crash
            end
            we = we'; 
            GMM.we = [GMM.we we];
            GMM.mu = [GMM.mu mu];
            GMM.sigma = [GMM.sigma sigma];
        end
    end
end
GMM.we = GMM.we/sum(GMM.we);

%crash

function [descrs_normalized,frames_normalized] = normalizeSift(opts,descrs,frames)
% -------------------------------------------------------------------------
descrs_normalized = descrs;

xy = descrs_normalized(opts.SIFTDIM+1:end,:);
descrs_normalized = descrs_normalized(1:opts.SIFTDIM,:);

% Remove empty ones
idx = find(sum(descrs_normalized)==0);
descrs_normalized(:,idx)=[];
if nargin < 3
    frames_normalized = [];
else
    frames_normalized = frames;
    frames_normalized(:,idx) = [];
end

% Square root:
descrs_normalized = sqrt(descrs_normalized);

% 1/4 norm
X = sum(descrs_normalized.*descrs_normalized).^-0.25;
descrs_normalized = bsxfun(@times, descrs_normalized,X);

xy(:,idx) = [];
descrs_normalized = [descrs_normalized; xy];

descrs_normalized(isnan(descrs_normalized))=0;

function [eigvec, m] = compute_PCA(X,PCADIM)
m = mean(X,2);
[eigvec,eigval]=eig(cov(X'));
[a,I] =  sort(diag(eigval), 'descend');
eigvec=eigvec(:,I(1:PCADIM));
