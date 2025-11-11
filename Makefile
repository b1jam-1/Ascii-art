CXX ?= g++

ifeq ($(DEBUG),1)
CXXFLAGS += -g -O0 -D_DEBUG
endif

TARGET := ascii_art
SRCS := main.cpp bmp_reader.cpp

.PHONY: all clean debug

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

debug:
	@$(MAKE) DEBUG=1

clean:
	-rm -f $(TARGET) *.o
