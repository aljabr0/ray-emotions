CC=gcc
LD=ld
CFLAGS=-Wall -Werror $(OPT_CFLAGS)
LDFLAGS=-lm -lpthread
LDFLAGS+=$(shell pkg-config --libs "glib-2.0" "gobject-2.0" "gthread-2.0")
CFLAGS+=$(shell pkg-config --cflags "glib-2.0" "gobject-2.0" "gthread-2.0")
INC=-I../include/ -I./
MYLIB=../renderer.o
TEST_EXE=
OBJS=vectors.o light.o object3d.o raytrace.o renderer.o color.o trivialobjs3d.o image.o \
	photon_map.o qmc.o patch_rendering.o kdtree.o utils.o triangle.o \
	procobj.o shader.o basic_shaders.o texture.o pnm.o matrix.o \
	global_illumination.o amb_occl_gi.o init.o samplers.o mathout.o \
	solvers.o rayem-math.o polygon.o supersampling.o mosaic.o raw_faces.o \
	tracing_accelerator.o grid_tracing_accelerator.o transforms.o varray.o \
	tr_vertex_normal.o tone_mapping.o nonl_tone_mapping.o bloom-filter.o \
	filter.o mitchell.o parser-utils.o mesh.o mywavefront.o octree.o \
	irradiance-cache.o param_set.o timer.o infinite_light.o camera.o \
	triangle-mesh.o kdtree-accelerator.o raytrace-ser.o sys.o plugin.o \
	imgpipeline.o filter-kernel.o

all: $(OBJS) $(TEST_EXE) $(MYLIB)
clean:
	-rm *.o $(TEST_EXE) $(MYLIB)

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $< -o $@
	
$(MYLIB): $(OBJS)
	$(LD) -r -o $(MYLIB) $(OBJS)
	
$(TEST_EXE): $(OBJS)
	$(CC) -Wall -Werror $(INC) $(LDFLAGS) $(CFLAGS) $@.c -o $@ $(OBJS)
