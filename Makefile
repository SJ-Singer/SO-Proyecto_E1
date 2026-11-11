CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
TARGET = simulador.exe

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

# Detectar el sistema operativo para la regla 'clean'
ifeq ($(OS),Windows_NT)
    CLEAN_CMD = del /f /q *.o $(TARGET) 2>NUL || exit 0
else
    CLEAN_CMD = rm -f *.o $(TARGET)
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(CLEAN_CMD)

.PHONY: all clean