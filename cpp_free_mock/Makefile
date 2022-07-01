TMPDIR=.
PROJECT_ROOT=.
GMOCK_LIB_DIR = ../thirdpkg/lib/gmock
GTEST_LIB_DIR = ../thirdpkg/lib/gtest

GMOCK_LIB := $(GMOCK_LIB_DIR)/libgmock_main.a $(GMOCK_LIB_DIR)/libgmock.a $(GTEST_LIB_DIR)/libgtest.a -lpthread
GMOCK_INCLUDE = -I ../thirdpkg/include/gmock -I ../thirdpkg/include/gtest -I ../thirdpkg/include -I .

CPP11_FLAGS=-g -O0 -std=c++11 -fno-inline -falign-functions=32
SRC=$(wildcard *.cpp)
TARGETS=$(patsubst %.cpp, %, $(SRC))
TARGETS_PATH=$(patsubst %.cpp, $(TMPDIR)/%.out, $(SRC))
RECENT=hello_world
RECENT_PATH=$(patsubst %, $(TMPDIR)/%.out, $(RECENT))

%: %.cpp
	g++ $(CPP11_FLAGS) $^ $(GMOCK_INCLUDE) $(GMOCK_LIB) -o $(TMPDIR)/$@.out

all: $(TARGETS)

recent: $(RECENT)

test: all
	echo $(TARGETS_PATH) | tr " " "\n" | while read test; do echo "Test: $$test"; $$test; done

recent-test: recent
	$(RECENT_PATH)
