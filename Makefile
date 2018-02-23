PWD:=$(shell pwd)
SOURCE=$(shell find src/ -type f -name "*.cc")
INCLUDE:=$(shell find include/ -type f -name "*.h")
OBJECT=${SOURCE:.cc=.o}
TEST:=$(shell find unittest/ -type f -name "*-test.cc")
TESTOBJECT:=${TEST:.cc=.t}
CXX = g++
SANITIZER=-fsanitize=address,undefined

CXXFLAGS += -Iinclude/ -std=c++17

src/%.o : src/%.cc include/%.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(LDFLAGS)

all : $(OBJECT)

unittest/%.t : unittest/%.cc  $(OBJECT) $(INCLUDE) $(SOURCE)
	$(CXX) $(OBJECT) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

test: CXXFLAGS += -g3 $(SANITIZER)
test: LDFLAGS  += $(TEST_LIBS)
test: $(TESTOBJECT)


clean :
	rm -rf $(OBJECT)

.PHONY: clean

