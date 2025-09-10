NAME = organizeSite

CXX = g++
CXXFLAGS = -O0 -Wall

all: $(NAME)

SOURCE = ../src/main.cpp

$(NAME): $(SOURCE)
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(NAME)

install: $(NAME)
	sudo cp $(NAME) /usr/local/bin/$(NAME)

clean:
	rm -f $(NAME)