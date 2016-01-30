CXX = g++-4.9 -g
LINK = g++-4.9 -g
CXX_FLAGS = -std=c++11 

SOURCES += main.cpp
INCLUDE += -I/urs/local/include/opencv -I/home/brian/robert_stuff/documentproject/src
LIBS += -L/urs/local/include/opencv -lopencv_imgproc -lopencv_core -lopencv_highgui
LIBS += -L/home/brian/robert_stuff/documentproject/lib -ldocumentproject
LIBS += -lpng -ltiff -ljpeg -pthread
PROGRAM_NAME = align

OBJECTS = obj/main.o obj/binarization.o


bin: $(PROGRAM_NAME)
all: $(PROGRAM_NAME)


clean:
	- rm  $(PROGRAM_NAME)
	- rm  obj/*.o


$(PROGRAM_NAME):  $(OBJECTS)  
	$(LINK) $(CXX_FLAGS) -o $(PROGRAM_NAME) $(OBJECTS) $(LIBS)

	
obj/main.o: main.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/main.o main.cpp

obj/binarization.o: binarization.h binarization.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/binarization.o binarization.cpp
