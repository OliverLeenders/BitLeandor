# specifying g++ as the compiler
CXX=g++
# defining C++ standard, and demanding compiler to generate all warnings possible
CXXFLAGS=-std=c++17 -Wall -Wextra -Wpedantic -g -fanalyzer
# when generating a profile build, include debug info
DEBUG_FLAGS=-g2 -fsanitize=address
# less optimization for profile builds

all: avx2 avx512 x86-64

avx2: CXXFLAGS += -mavx2
avx512: CXXFLAGS += -mavx512
x86-64: CXXFLAGS += -m64

avx2: engine
avx512: engine
x86-64: engine

OPTIMIZATION_FLAGS_PROFILE=-O0
# more optimization for release builds
OPTIMIZATION_FLAGS =-O3 -flto=auto

# directories for building
OBJDIR=object_files
PROFILE_OBJDIR=profile_object_files
OUTDIR=Versions

# create object_files directory
$(shell mkdir -p $(OBJDIR) $(PROFILE_OBJDIR) $(OUTDIR))

# get date
date = $(shell date '+%d%m%Y--%H%M')
# naming conventions
NAME="Leandor dev 2.0.0 $(date).exe"
NAME_PROFILE="Leandor dev 2.0.0 profile.exe"

avx2: NAME="Leandor dev 2.0.0 avx2 $(date).exe"
avx512: NAME="Leandor dev 2.0.0 avx512 $(date).exe"
x86-64: NAME="Leandor dev 2.0.0 x86-64 $(date).exe"

# all source files
SRCS = $(wildcard *.cpp)
# generate object files from source files
OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
PROFILE_OBJS = $(patsubst %.cpp,$(PROFILE_OBJDIR)/%.o,$(SRCS))
# compile engine from .o files
engine: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OPTIMIZATION_FLAGS) $(OBJS) -o $(OUTDIR)/$(NAME)
complete:
	$(CXX) $(CXXFLAGS) $(OPTIMIZATION_FLAGS) $(SRCS) -o $(OUTDIR)/$(NAME)
debug: $(PROFILE_OBJS)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS_PROFILE) $(PROFILE_OBJS) -o $(OUTDIR)/$(NAME_PROFILE)
# recipe to compile a .cpp file into an .o file; -MMD -MP generate dependency files
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(OPTIMIZATION_FLAGS) -MMD -MP -c $< -o $@

$(PROFILE_OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS_PROFILE) -MMD -MP -c $< -o $@

clean:
	rm -rf $(OBJDIR)/* $(PROFILE_OBJDIR)/*

echo-date:
	$(info date="$(date)")

# include dependency files
-include $(OBJS:.o=.d)
-include $(PROFILE_OBJS:.o=.d)