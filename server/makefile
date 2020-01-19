TARGET := server
OBJECTS := server.o encode_decode.o

CC := g++
CFLAGS = -std=c++11 -g -Wall
LDLIBS := -lpthread -lcrypto


default: $(TARGET)
all: default

%.o: %.cpp
	@echo compiling $< '--->'
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): server.o encode_decode.o
	@echo Linking $< '--->'
	$(CC) $(OBJECTS) -Wall $(LDLIBS) -o $@


.PHONY: default all clean

clean:
	-rm -f *.o
