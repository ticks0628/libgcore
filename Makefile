CC = $(CROSS_COMPILE)gcc $(CROSS_COMPILE_ARG)
CFLAGS = -Wall -O2 -I./ -I../include -fPIC -I./threadpool -I./g_timer
LDFLAGS = -L./threadpool -lthreadpool -L./g_timer -lg_timer -lrt -lpthread 

TARGET = g_test
OBJS := $(TARGET).o 

all: $(TARGET) 

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

prebuild: lib$(TARGET).o

distclean: clean

clean:
	rm -rf $(OBJS) $(TARGET) lib$(TARGET).o

%.o: %.c %.h
	$(CC) -c $(CFLAGS) $< -o $@

lib$(TARGET).so: $(TARGET).c $(TARGET).h
	$(CC) -c -fPIC $(CFLAGS) -o $@ $<

