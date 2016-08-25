
CXX = g++-4.9 
LINK = g++-4.9
CXX_FLAGS =    -std=c++11 -march=native -g #-O3  #-g

SOURCES += main.cpp
INCLUDE +=  -I/home/brian/familysearch_documentproject_2013.08.30/src

LIBS += -L/usr/local/MATLAB/R2015a/bin/glnxa64/ -licuuc -lmx -lmex -lmat -lm #-lmx #-lmwi18n#-lmx -l:libicuio.so.52

LIBS += -L/usr/local/lib/ -lopencv_features2d -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_ml -fopenmp -lopencv_flann -lopencv_ocl -lopencv_nonfree 

LIBS += -L/home/brian/familysearch_documentproject_2013.08.30/lib/ -ldocumentproj_2013.08.30 -ljpeg -ltiff -lpng -pthread -lm
#LIBS += -L/home/brian/robert_stuff/documentproject/lib -ldocumentproject

all: morphtest

morphtest: MorphSpotter.o gwdataset.o
	$(CXX) $(CXX_FLAGS) main.cpp MorphSpotter.o gwdataset.o -o morphtest $(INCLUDE) $(LIBS)


MorphSpotter.o: MorphSpotter.cpp MorphSpotter.h dataset.h
	$(CXX) -c $(CXX_FLAGS) -c -rdynamic -fPIC MorphSpotter.cpp $(INCLUDE)

gwdataset.o: dataset.h gwdataset.h gwdataset.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o gwdataset.o gwdataset.cpp

clean:
	rm MorphSpotter.o
	rm gwdataset.o
	rm morphtest
