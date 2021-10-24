all: a.out
CPPFLAGS :=-O3 -std=c++11 -Wall
LDFLAGS :=-lncursesw -lpthread
SRCS := test.cpp SquareObject.cpp Block.cpp Player.cpp Bomb.cpp Fire.cpp
OBJS := $(SRCS:%.cpp=%.o)
DEPS := $(SRCS:%.cpp=%.d)

a.out: $(OBJS)
		g++ $(CPPFLAGS) $(OBJS) $(LDFLAGS)

.cpp.o:
	g++ -MMD -MP -c $<

run: a.out
		./a.out

clean:
		rm -f a.out $(OBJS) $(DEPS)

-include $(DEPS)
