# specifying g++ as the compiler
CC=g++
# defining C++ standard, and demanding compiler to generate all warnings possible
CFLAGS=-std=c++17 -Wall -Wextra -Wpedantic
# when generating a profile build, i want to include debug info in the build
DEBUG_FLAGS=-g2
# less optimization for profile builds
OPTIMIZATION_FLAGS_PROFILE=-O2 -march=native
# more optimization for release builds
OPTIMIZATION_FLAGS =-O3 -march=native -flto=auto

# directories for building
OBJDIR=object_files
PROFILE_OBJDIR=profile_object_files
OUTDIR=Versions

# get date
date = $(shell date '+%d%m%Y--%H%M')
# naming conventions
NAME="Leandor dev 2.6.1 $(date).exe"
NAME_PROFILE="Leandor dev 2.6.1 profile.exe"


# all source files
SRCS = $(wildcard *.cpp)
# generate object files from source files
OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
PROFILE_OBJS = $(patsubst %.cpp,$(PROFILE_OBJDIR)/%.o,$(SRCS))
# compile engine from .o files
engine: $(OBJS)
	$(CC) $(CFLAGS) $(OPTIMIZATION_FLAGS) $(OBJS) -o $(OUTDIR)/$(NAME)
complete: 
	$(CC) $(CFLAGS) $(OPTIMIZATION_FLAGS) $(SRCS) -o $(OUTDIR)/$(NAME)
profile: $(PROFILE_OBJS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS_PROFILE) $(PROFILE_OBJS) -o $(OUTDIR)/$(NAME_PROFILE)
# recepy to compile a .cpp file into an .o file
$(OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) $(OPTIMIZATION_FLAGS) -c $< -o $@

$(PROFILE_OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS_PROFILE) -c $< -o $@

clean:
	rm -rf $(OBJDIR)/* $(PROFILE_OBJDIR)/*

echo-date: 
	$(info date="$(date)")