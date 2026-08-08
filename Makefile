CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread

SRCS = zayden_ai.cpp ollama_client.cpp consciousness_patch.cpp dna_snapshot.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = zayden_ai

.PHONY: all clean run stop

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) *.dna zayden_memory.txt

run: $(TARGET)
	./$(TARGET)

stop:
	-pkill -f $(TARGET)
