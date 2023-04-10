CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g

TARGET = server
OBJS = log/*.cpp threadpool/*.cpp heaptimer/*.cpp epoller/*.cpp sqlconn/*.cpp \
       http/*.cpp server/*.cpp main.cpp


all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o bin/$(TARGET)  -pthread -lmysqlclient

clean:
	rm -rf bin/$(OBJS) $(TARGET)




