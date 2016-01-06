function [varargout] = shear_fill(varargin)

% psmFit:  Fits a part-structured model to a binary image.
%
% Uses bicubic translation for subpixel accuracy on translation.
%
% [dtsq,loc] = psmFit(model,img)
% [dtsq,loc] = psmFit(model,img,mdsq)
% Note:  mdsq = max_x^2+max_y^2 for equivalence with psmFit_gpu
%
% See also psmFit_gpu.

fprintf('Mex file not found -- attempting to compile...\n');
try
    mex -I/home/brian/robert_stuff/documentproject/src -L/home/brian/robert_stuff/documentproject/lib -ldocumentproject -Ilpng120/ -lpng -ltiff -ljpeg -I/urs/local/include/opencv -lopencv_imgproc -lopencv_core -lopencv_highgui shear_fill.cpp binarization.cpp
    fprintf('Compilation complete.  Continuing...\n');
    [varargout{1:nargout}] = shear_fill(varargin{:});
catch
    error('Unable to run mex file.');
end
