CC = $(CROSS_COMPILE)gcc $(CROSS_COMPILE_ARG)
CFLAGS = -Wall -O2 -I./ -I../include -fPIC -I../threadpool -I../ 
LDFLAGS = -lrt -lpthread -L../threadpool -lthreadpool

TARGET = g_test
OBJS := $(TARGET).o g_tool.o g_schedule.o g_timer.o

all: $(TARGET) lib$(TARGET).so

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

