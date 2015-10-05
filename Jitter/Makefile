CXX = g++-4.9
CXX_FLAGS = -std=c++11

SOURCES += main.cpp

LIBS += -L/urs/local/include/opencv2 -lopencv_features2d -lopencv_core -lopencv_highgui
PROGRAM_NAME = Jitter

bin: $(PROGRAM_NAME)

clean:
	- rm -f $(PROGRAM_NAME)
	
	
Jitter: main.cpp
	$(CXX) $(CXX_FLAGS) main.cpp -o $(PROGRAM_NAME) $(LIBS)
