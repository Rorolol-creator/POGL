#************************************************
#*                                              *
#*             (c) 2017 J. FABRIZIO             *
#*                                              *
#*                               LRDE EPITA     *
#*                                              *
#************************************************

CC = g++

CC = g++
CPP_FILES = fileloader.cpp matrix4.cpp program.cpp particle.cpp material.cpp perlin3D.cpp
HXX_FILES = fileloader.hh matrix4.hh program.hh particle.hh material.hh perlin3D.hh
HXX_FILES += object_vbo.hh
OBJ_FILES = $(CPP_FILES:.cpp=.o) main.o       
CXX_FLAGS += -Wall -Wextra -O3 -g -std=c++11
CXX_FLAGS += -m64 -march=x86-64
CXX_FLAGS +=
LDXX_FLAGS = -lGL -lGLEW -lglut -lpthread -lX11 
LIBS=glfw3 gl glu glut glew
CXX_FLAGS += $(shell pkg-config --cflags $(LIBS))
LDXX_FLAGS += $(shell pkg-config --libs $(LIBS))
MAIN_FILE = main.cpp
DIST = main
SKEL_DIST_DIR = pogl_skel_tp
SKEL_FILES = $(CPP_FILES) $(HXX_FILES) $(MAIN_FILE) Makefile vertex.shd fragment.shd texture.tga lighting.tga normalmap.tga


#For gcc 4.9
#CXXFLAGS+=-fdiagnostics-color=auto
export GCC_COLORS=1

define color
    if test -n "${TERM}" ; then\
	if test `tput colors` -gt 0 ; then \
	    tput setaf $(1); \
        fi;\
    fi
endef

define default_color
    if test -n "${TERM}" ; then\
	if test `tput colors` -gt 0 ; then  tput sgr0 ; fi; \
    fi
endef


all: post-build

pre-build:
	@$(call color,4)
	@echo "******** Starting Compilation ************"
	@$(call default_color)

post-build:
	@make --no-print-directory main-build ; \
	sta=$$?;	  \
	$(call color,4); \
	echo "*********** End Compilation **************"; \
	$(call default_color); \
	exit $$sta;

main-build: pre-build build

build: $(OBJ_FILES)
	$(CC) -o $(DIST) $(OBJ_FILES) $(CXX_FLAGS) $(LDXX_FLAGS)

CIMG_FLAGS = $(filter-out -Wall -Wextra, $(CXX_FLAGS)) -Wno-class-memaccess

fileloader.o: fileloader.cpp fileloader.hh
	@$(call color ,2)
	@echo "[$@] (CImg warnings suppressed)"
	@$(call default_color)
	@$(CC) -c -o $@ $< $(CIMG_FLAGS) ; \
	sta=$$?;      \
	if [ $$sta -eq 0 ]; then  \
		$(call color,2) ; \
		echo "[$@ succes]" ; \
		$(call default_color) ; \
	else  \
		$(call color,1) ; \
      	echo "[$@ failure]" ; \
      	$(call default_color) ; \
	fi ;\
	exit $$sta


material.o: material.cpp material.hh
	@$(call color ,2)
	@echo "[$@] (CImg warnings suppressed)"
	@$(call default_color)
	@$(CC) -c -o $@ $< $(CIMG_FLAGS) ; \
	sta=$$?;      \
	if [ $$sta -eq 0 ]; then  \
		$(call color,2) ; \
		echo "[$@ succes]" ; \
		$(call default_color) ; \
	else  \
		$(call color,1) ; \
      	echo "[$@ failure]" ; \
      	$(call default_color) ; \
	fi ;\
	exit $$sta

main.o: main.cpp
	$(CC) -c -o $@ $< $(CIMG_FLAGS)

%.o: %.cpp %.hh
	@$(call color,2)
	@echo "[$@] $(CXX_FLAGS)"
	@$(call default_color)
	@$(CC) -c -o $@ $< $(CXX_FLAGS) ; \
	sta=$$?;	  \
	if [ $$sta -eq 0 ]; then  \
	  $(call color,2) ; \
	  echo "[$@ succes]" ; \
	  $(call default_color) ; \
	else  \
	  $(call color,1) ; \
	  echo "[$@ failure]" ; \
	  $(call default_color) ; \
	fi ;\
	exit $$sta

.PHONY: all clean pre-build post-build main-build build skel

clean:
	rm -f $(OBJ_FILES)
	rm -f $(DIST)
	rm -rf $(SKEL_DIST_DIR).tar.bz2


skel:
	rm -rf $(SKEL_DIST_DIR)
	mkdir $(SKEL_DIST_DIR)
	cp $(SKEL_FILES) $(SKEL_DIST_DIR)
	tar -cjvf $(SKEL_DIST_DIR).tar.bz2 $(SKEL_DIST_DIR)
	rm -rf $(SKEL_DIST_DIR)
