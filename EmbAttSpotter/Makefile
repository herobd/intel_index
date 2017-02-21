CXX = g++-4.9 
LINK = g++-4.9
CXX_FLAGS =  -fPIC  -std=c++11 -g #-O3  #-g

SOURCES += main.cpp
#INCLUDE +=  -Ivlfeat-0.9.20 #-Imatio-1.5.6/src -I/usr/local/MATLAB/R2015a/extern/include -I"/usr/local/MATLAB/R2015a/simulink/include"

#LIBS += -L/usr/local/MATLAB/R2015a/bin/glnxa64/ -licuuc -lmx -lmex -lmat -lm #-lmx #-lmwi18n#-lmx -l:libicuio.so.52

LIBS += -L/usr/local/lib/ -lopencv_features2d -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_ml -fopenmp -lopencv_flann -lopencv_ocl -lopencv_nonfree 

LIBS += -larmadillo
#LIBS += -L/home/brian/robert_stuff/documentproject/lib -ldocumentproject

LIBS += -L/home/brian/intel_index/EmbAttSpotter/vlfeat-0.9.20/bin/glnxa64/ -l:libvl.so -lstdc++
#LIBS += -L/home/brian/intel_index/EmbAttSpotter/matio-1.5.6/src/.libs/ -l:libmatio.a
#LIBS += -L/usr/local/hdf5/lib/ -l:libhdf5.a -lz -ldl
#LIBS += -lpng -ltiff -ljpeg -pthread -lz 


PROGRAM_NAME = embattspotter
LIB_NAME = libembattspotter.a

OBJECTS = obj/main.o obj/embattspotter.o obj/gwdataset.o obj/stridedataset.o

bin: $(PROGRAM_NAME)
all: $(PROGRAM_NAME) $(LIB_NAME)
lib: $(LIB_NAME)

clean:
	- rm  $(PROGRAM_NAME)
	- rm  $(LIB_NAME)
	- rm  obj/*.o

$(LIB_NAME): $(OBJECTS)
	- rm  $(LIB_NAME)
	ar -cvq $(LIB_NAME) $(OBJECTS)

$(PROGRAM_NAME):  $(OBJECTS)  
	$(LINK) $(CXX_FLAGS) -o $(PROGRAM_NAME) $(OBJECTS) $(LIBS) 

	
obj/main.o: main.cpp gwdataset.h stridedataset.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/main.o main.cpp $(LIBS)

obj/embattspotter.o: embattspotter.h embattspotter.cpp embattspotter_eval.cpp embattspotter_test.cpp gwdataset.h SubwordSpottingResult.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/embattspotter.o embattspotter.cpp $(LIBS)
	
obj/gwdataset.o: dataset.h gwdataset.h gwdataset.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/gwdataset.o gwdataset.cpp

obj/stridedataset.o: dataset.h stridedataset.h stridedataset.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/stridedataset.o stridedataset.cpp
	
	#export LD_LIBRARY_PATH=/home/brain/intel_index/EmbAttSpotter/vlfeat-0.9.20/bin/glnxa64/:$LD_LIBRARY_PATH -l:libmatio.so.4.0.2
	#-L/urs/local/include/opencv 
	#-I/urs/local/include/opencv

