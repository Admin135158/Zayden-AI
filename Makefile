CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread

.PHONY: all clean run stop

all: zayden_ai

zayden_ai: zayden_ai.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f zayden_ai *.o *.dna zayden_memory.txt
	@echo "Clean complete"

run: zayden_ai
	./zayden_ai

stop:
	-pkill -f zayden_ai
	@echo "Zayden-AI stopped"
