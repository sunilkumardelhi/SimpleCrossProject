CXX = g++ --std=gnu++11
	
# Remember to tweak this if you move this file.
GTEST_DIR = googletest

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread

# All Google Test headers.  
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# House-keeping build targets.	
# Builds gtest.a and gtest_main.a.

GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

# A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.
# simple_cross is depended on OrderBook
all: simple_cross simple_cross_unittest

simple_cross: simple_cross.o OrderBook.o
	$(CXX) simple_cross.o OrderBook.o -o simple_cross

simple_cross.o: simple_cross.cpp
	$(CXX) -c simple_cross.cpp

OrderBook.o: OrderBook.cpp OrderBook.h
	$(CXX) -c OrderBook.cpp
	

simple_cross_unittest.o : simple_cross_unittest.cpp \
                     $(GTEST_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c simple_cross_unittest.cpp

simple_cross_unittest : OrderBook.o simple_cross_unittest.o gtest_main.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@

clean: 
	rm *.o *.exe
