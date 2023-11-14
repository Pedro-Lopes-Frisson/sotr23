CC=gcc
LDIRS = -L/usr/local/lib 

# Use script to get SDL2 flags (recommended for portability)
SDL2_CONFIG = sdl2-config
CFLAGS = $(shell $(SDL2_CONFIG) --cflags) -g
LIBS = $(shell $(SDL2_CONFIG) --libs)

# Add other compilation and linking flags if necessary
CFLAGS += -I/usr/local/include
LIBS += -lavformat -lavcodec -lavutil -lavdevice -lswscale

OBJFILES = cab.o imageViewer.o objDetector.o landmarkDetector.o detectRedSquare.o
HDRFILES = include/cab.h include/imageViewer.h include/objDetector.h include/landmarkDetector.h include/detectRedSquare.h include/varsDisplayer.h

all: webCamCapture cab imageViewer objDetector landmarkDetector detectRedSquare varsDisplayer main test_cab
.PHONY: all

# Generate application
webCamCapture: webCamCapture.c  
	$(CC) $(CFLAGS) $(LDIRS) $< -o $@ $(LIBS)

main: main.c $(OBJFILES)
	$(CC) $(CFLAGS) $(LDIRS) $< -o $@ $(OBJFILES) $(LIBS) -pthread

cab: src/cab.c include/cab.h
	$(CC) $(CFLAGS) $(LDIRS) $< -c $(LIBS) -pthread

imageViewer: src/imageViewer.c include/imageViewer.h
	$(CC) $(CFLAGS) $(LDIRS) $< -c $(LIBS) -pthread

objDetector: src/objDetector.c include/objDetector.h
	$(CC) $(CFLAGS) $(LDIRS) $< -c $(LIBS) -pthread

landmarkDetector: src/landmarkDetector.c include/landmarkDetector.h include/point.h
	$(CC) $(CFLAGS) $(LDIRS) $< -c $(LIBS) -pthread

detectRedSquare: src/detectRedSquare.c include/detectRedSquare.h include/point.h
	$(CC) $(CFLAGS) $(LDIRS) $< -c $(LIBS) -pthread

varsDisplayer: src/varsDisplayer.c include/varsDisplayer.h
	$(CC) $(CFLAGS) $(LDIRS) $< -o $@ $(LIBS)

test_cab: test_cab.c cab.o
	$(CC) $(CFLAGS) $(LDIRS) $< -o $@ cab.o $(LIBS)

.PHONY: clean 

clean:
	rm -f *.o 
	rm -f webCamCapture
	rm -f main
	rm -f varsDisplayer
