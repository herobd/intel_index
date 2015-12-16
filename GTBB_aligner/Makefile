CXX = g++-4.9 -g
CXX_FLAGS = -std=c++11 -pthread

SOURCES += main.cpp
INCLUDE += -I/urs/local/include/opencv -I/home/brian/robert_stuff/documentproject/src
LIBS += -L/urs/local/include/opencv -lopencv_imgproc -lopencv_core -lopencv_highgui
LIBS += -L/home/brian/robert_stuff/documentproject/lib -ldocumentproject
LIBS += -lpng -ltiff -ljpeg
PROGRAM_NAME = align

bin: $(PROGRAM_NAME)

clean:
	- rm  $(PROGRAM_NAME)
	
	
align: main.cpp
	$(CXX) $(CXX_FLAGS) main.cpp -o $(PROGRAM_NAME) $(INCLUDE) $(LIBS)
