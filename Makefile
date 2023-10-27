CC=gcc
LDIRS = -L/usr/local/lib 

# Use script to get SDL2 flags (recommended for portability)
SDL2_CONFIG = sdl2-config
CFLAGS = $(shell $(SDL2_CONFIG) --cflags) -g
LIBS = $(shell $(SDL2_CONFIG) --libs)

# Add other compilation and linking flags if necessary
CFLAGS += -I/usr/local/include
LIBS += -lavformat -lavcodec -lavutil -lavdevice -lswscale

OBJFILES = cab.o imageViewer.o landmarkDetector.o

all: webCamCapture cab imageViewer landmarkDetector main 
.PHONY: all

# Generate application
webCamCapture: webCamCapture.c  
	$(CC) $(CFLAGS) $(LDIRS) $< -o $@ $(LIBS)

main: main.c cab.o imageViewer.o landmarkDetector.o
	$(CC) $(CFLAGS) $(LDIRS) $< -o $@ $(OBJFILES) $(LIBS) -pthread

cab: src/cab.c include/cab.h
	$(CC) $(CFLAGS) $(LDIRS) $< -c $(LIBS) -pthread

imageViewer: src/imageViewer.c include/imageViewer.h
	$(CC) $(CFLAGS) $(LDIRS) $< -c $(LIBS) -pthread


landmarkDetector: src/landmarkDetector.c include/landmarkDetector.h
	$(CC) $(CFLAGS) $(LDIRS) $< -c $(LIBS) -pthread

.PHONY: clean 

clean:
	rm -f *.o 
	rm webCamCapture
	rm main
