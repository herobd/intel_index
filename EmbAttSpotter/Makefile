CXX = g++-4.9 -g
LINK = g++-4.9 -g
CXX_FLAGS = -std=c++11 

SOURCES += main.cpp
INCLUDE += -I/urs/local/include/opencv -Ivlfeat-0.9.20
LIBS += -L/urs/local/include/opencv -lopencv_imgproc -lopencv_core -lopencv_highgui
LIBS += -L/home/brian/robert_stuff/documentproject/lib -ldocumentproject
LIBS += -Lvlfeat-0.9.20/bin/a64/ -lvl
LIBS += -lpng -ltiff -ljpeg -pthread
PROGRAM_NAME = embattspotter

OBJECTS = obj/main.o obj/embattspotter.o


bin: $(PROGRAM_NAME)
all: $(PROGRAM_NAME)


clean:
	- rm  $(PROGRAM_NAME)
	- rm  obj/*.o


$(PROGRAM_NAME):  $(OBJECTS)  
	$(LINK) $(CXX_FLAGS) -o $(PROGRAM_NAME) $(OBJECTS) $(LIBS)

	
obj/main.o: main.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/main.o main.cpp

obj/embattspotter.o: embattspotter.h embattspotter.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/embattspotter.o embattspotter.cpp
