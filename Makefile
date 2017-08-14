#Compiler and Linker
CC			:= g++

#The Target Binary Program
TARGET					:= LEngine

#The Directories, Source, Includes, Objects, Binary and Resources
SRCDIR		:= Source/Engine
INCDIR		:= -I./LinuxDependencies/rapidxml-1.13 -I./LinuxDependencies/LuaBridge-master/Source/LuaBridge
BUILDDIR		:= obj
TARGETDIR	:= bin
RESDIR		:= Data
SRCEXT		:= cpp
DEPEXT		:= d
OBJEXT		:= o

#Flags, Libraries and Includes
CFLAGS		:= -Wall -g -DGLEW_STATIC -std=c++11 -ggdb3
LIB				:= -lSOIL -llua5.2 -lSDL2 -lSDL2main -lSDL2_ttf -lSDL2_mixer -lphysfs -lGL -lGLU -lglut -lGLEW -lfreetype
INC			:= $(INCDIR) -I/usr/local/include
INCDEP		:= $(INCDIR)

#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------

#Exclude main.cpp from the source directory
SOURCES			:= $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
#SOURCES			:= $(filter-out $(SRCDIR)/Engine/main.cpp, $(SOURCES))

OBJECTS			:= $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

#Defauilt Make
all: resources $(TARGET)

#Remake
remake: cleaner all

#Copy Resources from Resources Directory to Target Directory
resources: directories
	@cp -r $(RESDIR)/* $(TARGETDIR)/$(RESDIR)

#Make the Directories
directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(TARGETDIR)/$(RESDIR)
	@mkdir -p $(BUILDDIR)

#Clean only Objecst
clean:
	@$(RM) -rf $(BUILDDIR)

#Full Clean, Objects and Binaries
cleaner: clean
	@$(RM) -rf $(TARGETDIR)

#Pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

#Link
$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGETDIR)/$(TARGET) $^ $(LIB)

#Compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
	@$(CC) $(CFLAGS) $(INCDEP) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
	@cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	@rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

#Non-File Targets
.PHONY: all remake clean cleaner resources
