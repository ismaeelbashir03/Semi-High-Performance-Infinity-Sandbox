CXX ?= c++
CXXFLAGS = -O3 -march=native -ffast-math -std=c++17 -DNDEBUG -MMD -MP
LDFLAGS =

BREW ?= brew
LIBOMP_PREFIX := $(shell $(BREW) --prefix libomp 2>/dev/null)
CXX_VERSION := $(shell $(CXX) --version 2>/dev/null)

# need to get libomp from brew, as im on mac
ifneq (,$(findstring clang,$(CXX_VERSION)))
  CXXFLAGS += -Xpreprocessor -fopenmp
  ifneq (,$(LIBOMP_PREFIX))
    CXXFLAGS += -I$(LIBOMP_PREFIX)/include
    LDFLAGS += -L$(LIBOMP_PREFIX)/lib
  endif
  LDFLAGS += -lomp
else
  CXXFLAGS += -fopenmp
endif

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

all: limitless_server

limitless_server: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f limitless_server $(OBJS)

-include $(DEPS)
