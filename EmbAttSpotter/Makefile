CXX = g++-4.9 -g
LINK = g++-4.9 -g
CXX_FLAGS = -std=c++11 

SOURCES += main.cpp
INCLUDE += -I/urs/local/include/opencv -Ivlfeat-0.9.20 -Imatio-1.5.6/src




LIBS += -L/home/brian/robert_stuff/documentproject/lib -ldocumentproject

LIBS += -L/home/brian/intel_index/EmbAttSpotter/vlfeat-0.9.20/bin/glnxa64/ -l:libvl.so
#LIBS += -L/home/brian/intel_index/EmbAttSpotter/matio-1.5.6/src/.libs/ -l:libmatio.a
LIBS += -L/usr/local/hdf5/lib/ -lhdf5
LIBS += -lpng -ltiff -ljpeg -pthread -lz 
LIBS += -lopencv_imgproc -lopencv_core -lopencv_highgui -lopencv_features2d -lopencv_nonfree
PROGRAM_NAME = embattspotter

OBJECTS = obj/main.o obj/embattspotter.o obj/gwdataset.o


bin: $(PROGRAM_NAME)
all: $(PROGRAM_NAME)


clean:
	- rm  $(PROGRAM_NAME)
	- rm  obj/*.o


$(PROGRAM_NAME):  $(OBJECTS)  
	$(LINK) $(CXX_FLAGS) -o $(PROGRAM_NAME) $(OBJECTS) $(LIBS)

	
obj/main.o: main.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/main.o main.cpp

obj/embattspotter.o: embattspotter.h embattspotter.cpp embattspotter_test.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/embattspotter.o embattspotter.cpp
	
obj/gwdataset.o: dataset.h gwdataset.h gwdataset.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/gwdataset.o gwdataset.cpp
	
	#export LD_LIBRARY_PATH=/home/brain/intel_index/EmbAttSpotter/vlfeat-0.9.20/bin/glnxa64/:$LD_LIBRARY_PATH -l:libmatio.so.4.0.2
	#-L/urs/local/include/opencv 
