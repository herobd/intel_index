CXX = g++-4.9 
LINK = g++-4.9
CXX_FLAGS =  -g  -std=c++11 #-ansi -fexceptions -fPIC -fno-omit-frame-pointer -pthread

SOURCES += main.cpp
INCLUDE +=  -Ivlfeat-0.9.20 -Imatio-1.5.6/src -I/usr/local/MATLAB/R2015a/extern/include -I"/usr/local/MATLAB/R2015a/simulink/include"

LIBS += -L/usr/local/MATLAB/R2015a/bin/glnxa64/ -licuuc -lmx -lmex -lmat -lm #-lmx #-lmwi18n#-lmx -l:libicuio.so.52

LIBS += -L/usr/local/lib/ -lopencv_features2d -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_ml -fopenmp -lopencv_flann -lopencv_ocl -lopencv_nonfree 
#LIBS += -L/home/brian/robert_stuff/documentproject/lib -ldocumentproject

LIBS += -L/home/brian/intel_index/EmbAttSpotter/vlfeat-0.9.20/bin/glnxa64/ -l:libvl.so -lstdc++
#LIBS += -L/home/brian/intel_index/EmbAttSpotter/matio-1.5.6/src/.libs/ -l:libmatio.a
#LIBS += -L/usr/local/hdf5/lib/ -l:libhdf5.a -lz -ldl
#LIBS += -lpng -ltiff -ljpeg -pthread -lz 


PROGRAM_NAME = embattspotter

OBJECTS = obj/main.o obj/embattspotter.o obj/gwdataset.o

bin: $(PROGRAM_NAME)
all: $(PROGRAM_NAME)


clean:
	- rm  $(PROGRAM_NAME)
	- rm  obj/*.o


$(PROGRAM_NAME):  $(OBJECTS)  
	$(LINK) $(CXX_FLAGS) -o $(PROGRAM_NAME) $(OBJECTS) $(LIBS) 

	
obj/main.o: main.cpp gwdataset.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/main.o main.cpp $(LIBS)

obj/embattspotter.o: embattspotter.h embattspotter.cpp embattspotter_eval.cpp embattspotter_test.cpp gwdataset.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/embattspotter.o embattspotter.cpp $(LIBS)
	
obj/gwdataset.o: dataset.h gwdataset.h gwdataset.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/gwdataset.o gwdataset.cpp
	
	#export LD_LIBRARY_PATH=/home/brain/intel_index/EmbAttSpotter/vlfeat-0.9.20/bin/glnxa64/:$LD_LIBRARY_PATH -l:libmatio.so.4.0.2
	#-L/urs/local/include/opencv 
	#-I/urs/local/include/opencv

