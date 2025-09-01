# specifying g++ as the compiler
CC=g++
# defining C++ standard, and demanding compiler to generate all warnings possible
CFLAGS=-std=c++17 -Wall -Wextra -Wpedantic
# when generating a profile build, include debug info
DEBUG_FLAGS=-g2
# less optimization for profile builds
OPTIMIZATION_FLAGS_PROFILE=-O0 -march=native
# more optimization for release builds
OPTIMIZATION_FLAGS =-O3 -march=native -flto=auto

# directories for building
OBJDIR=object_files
PROFILE_OBJDIR=profile_object_files
OUTDIR=Versions

# get date
date = $(shell date '+%d%m%Y--%H%M')
# naming conventions
NAME="Leandor dev 2.6.2 $(date).exe"
NAME_PROFILE="Leandor dev 2.6.2 profile.exe"

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
debug: $(PROFILE_OBJS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS_PROFILE) $(PROFILE_OBJS) -o $(OUTDIR)/$(NAME_PROFILE)
# recipe to compile a .cpp file into an .o file; -MMD -MP generate dependency files
$(OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) $(OPTIMIZATION_FLAGS) -MMD -MP -c $< -o $@

$(PROFILE_OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS_PROFILE) -MMD -MP -c $< -o $@

clean:
	rm -rf $(OBJDIR)/* $(PROFILE_OBJDIR)/*

echo-date:
	$(info date="$(date)")

# include dependency files
-include $(OBJS:.o=.d)
-include $(PROFILE_OBJS:.o=.d)