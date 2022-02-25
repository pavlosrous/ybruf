TARGET = ybruf
INCLUDE = ../include
OBJS = ybruf.o yb_requests.o
CC = gcc
# Uncomment the end of the rule for memory debugging
LIBS = #-lefence

CFLAGS = -I$(INCLUDE) -Wall -pedantic -Wextra -Wcast-align -Wcast-qual \
	-Winit-self \
	-Wmissing-include-dirs -Wredundant-decls -Wshadow \
	-Wstrict-overflow=5 -Wundef -Werror -Wno-unused \
	-g -Wno-unused-result -O3 -Wno-strict-overflow -Wno-sign-compare

all: $(TARGET)

$(TARGET): $(OBJS) 
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

ybruf.o: ybruf.c $(INCLUDE)/ybruf.h

yb_requests.o: yb_requests.c $(INCLUDE)/ybruf.h

# Remove everything that can be rebuilt
clean:
	rm -f $(OBJS) $(TARGET) 
