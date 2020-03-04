
CC = gcc
INCLUDE = -I/home/ubuntu/Documents/flycapture.2.7.3.19_armhf/include -I/usr/include/flycapture
LIBS = -L/home/ubuntu/Documents/flycapture.2.7.3.19_armhf/libs -lm -lflycapture-c
CFLAGS = -O2 -W -Wall 
TARGET = capfotos

OBJ = capfotos.o geoCam.o getData.o  txVideo.o
# all: programa

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE)

capfotos: $(OBJ)
	$(CC) -o $@ $^  $(INCLUDE) $(LIBS)
	@rm -r $(OBJ)
	mkdir Resultados

clean:
	rm -rf $(TARGET) *~ *.o 
