all: a.out
CPPFLAGS :=-O3 -std=c++11 -Wall
LDFLAGS :=-lncursesw -lpthread
SRCS := SquareObject.cpp Block.cpp Player.cpp Bomb.cpp Fire.cpp ServerClient.cpp
OBJS := $(SRCS:%.cpp=%.o)
DEPS := $(SRCS:%.cpp=%.d)

a.out: server.o $(OBJS)
		g++ $(CPPFLAGS) server.o $(OBJS) $(LDFLAGS)

test: test.o $(OBJS)
		g++ $(CPPFLAGS)  test.o $(OBJS) $(LDFLAGS)

test1: test1.o $(OBJS)
		g++ $(CPPFLAGS)  test1.o $(OBJS) $(LDFLAGS)

test2: test2.o $(OBJS)
		g++ $(CPPFLAGS)  test2.o $(OBJS) $(LDFLAGS)

test3: test3.o $(OBJS)
		g++ $(CPPFLAGS)  test3.o $(OBJS) $(LDFLAGS)


.cpp.o:
	g++ -MMD -MP -c $<

run: a.out
		./a.out

clean:
		rm -f a.out $(OBJS) $(DEPS)

-include $(DEPS)
