PWD:=$(shell pwd)
SOURCE=$(shell find src/ -type f -name "*.cc")
INCLUDE:=$(shell find include/ -type f -name "*.h")
OBJECT=${SOURCE:.cc=.o}
TEST:=$(shell find unittest/ -type f -name "*-test.cc")
TESTOBJECT:=${TEST:.cc=.t}
CXX = g++
SANITIZER=-fsanitize=address,undefined

CXXFLAGS += -Iinclude/dinject -std=c++17

src/%.o : src/%.cc include/%.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(LDFLAGS)

all : release

unittest/%.t : unittest/%.cc  $(OBJECT) $(INCLUDE) $(SOURCE)
	$(CXX) $(OBJECT) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

test: CXXFLAGS += -g3 $(SANITIZER)
test: $(TESTOBJECT)

release: CXXFLAGS += -O3
release: $(OBJECT)
	ar crf libdinject.a $(OBJECT)

clean :
	rm -rf $(OBJECT)
	rm -rf libdinject.a

.PHONY: clean

