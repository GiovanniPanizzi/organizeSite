NAME = organizeSite

CXX = g++ -std=c++17
CXXFLAGS = -O0 -Wall

all: $(NAME)

SOURCE = ../src/main.cpp ../src/Organizer.cpp ../src/FileManager.cpp ../src/DirectoryManager.cpp ../src/Parser.cpp

$(NAME): $(SOURCE)
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(NAME)

install: $(NAME)
	sudo cp $(NAME) /usr/local/bin/$(NAME)
	@echo "Installed $(NAME) in /usr/local/bin"
	@echo "Refreshing shell cache..."
	@hash -r

# Pulizia dei file generati
clean:
	rm -f $(NAME)
	sudo rm -f /usr/local/bin/$(NAME)