CXX := g++

CXXFLAGS := -g -std=c++11 -fPIC -Wall
CXXFLAGS += -I ../Engine/Engine/include/
CXXFLAGS += -I ../Engine/dependencies/SFML-2.0/include/
CXXFLAGS += -DGLEW_STATIC -DGLEW_NO_GLU

OBJDIR := build

#deps
LDFLAGS := -lGL -lGLEW
LDFLAGS += ../Engine/Engine/lib_Engine.a
LDFLAGS += -lfreeimage

CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(patsubst %.cpp, %.o, $(CPP_FILES))

Minicraft : $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	
$(OBJDIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm src/*.o

.PHONY: clean
