# Makefile for Basler Pylon sample program
.PHONY			: all clean

# the program to build
NAME			:= AcquireSingleFrame

# Build tools and flags
CXX			:= /usr/bin/g++
LD			:= /usr/bin/g++
CPPFLAGS		:= -I$(GENICAM_ROOT_V2_1)/library/CPP/include \
			   -I$(PYLON_ROOT)/include -DUSE_GIGE
CXXFLAGS		:=    			#e.g., CXXFLAGS=-g -O0 for debugging

# To keep the makefile for 32 and 64 in sync we add 64 and 32-bit library paths
# If you are targeting only 32 bit for you can remove the entries containing "64"
LDFLAGS			:= -L$(PYLON_ROOT)/lib64 \
			   -L$(PYLON_ROOT)/lib \
			   -L$(GENICAM_ROOT_V2_1)/bin/Linux64_x64 \
			   -L$(GENICAM_ROOT_V2_1)/bin/Linux64_x64/GenApi/Generic \
			   -L$(GENICAM_ROOT_V2_1)/bin/Linux32_i86 \
			   -L$(GENICAM_ROOT_V2_1)/bin/Linux32_i86/GenApi/Generic \
			   -Wl,-E
LIBS			:= -lpylonbase

all			: $(NAME)

$(NAME)			: $(NAME).o
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

$(NAME).o		: $(NAME).cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean			:
	$(RM) $(NAME).o $(NAME)
