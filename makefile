CXX ?= g++
CXXFLAGS := $(CXXFLAGS) -Wall -Werror -pedantic -std=c++11
ifeq ($(BUILD),release)
    CXXFLAGS := $(CXXFLAGS) -DNDEBUG -O2
else
    CXXFLAGS := $(CXXFLAGS) -D_DEBUG -g
endif

rshell.TARGET := bin/rshell
rshell.SOURCE := \
    src/AppendRedirectionCommand.cpp \
    src/ArgVector.cpp \
    src/Command.cpp \
    src/ConjunctiveCommand.cpp \
    src/DisjunctiveCommand.cpp \
    src/ExecutableCommand.cpp \
    src/Executor.cpp \
    src/ExecutorPipe.cpp \
    src/ExecutorStream.cpp \
    src/ExecutorStreamSet.cpp \
    src/ExitBuiltinCommand.cpp \
    src/ExitException.cpp \
    src/InputRedirectionCommand.cpp \
    src/OutputRedirectionCommand.cpp \
    src/Parser.cpp \
    src/PipeCommand.cpp \
    src/PosixExecutor.cpp \
    src/PosixExecutorAppendFileStream.cpp \
    src/PosixExecutorInputFileStream.cpp \
    src/PosixExecutorOutputFileStream.cpp \
    src/PosixExecutorPipe.cpp \
    src/PosixExecutorPipeStream.cpp \
    src/SequentialCommand.cpp \
    src/Shell.cpp \
    src/TestBuiltinCommand.cpp \
    src/Tokenizer.cpp \
    src/main.cpp
rshell.OBJECT := $(patsubst %.cpp,%.o,$(rshell.SOURCE))
rshell.DEPEND := $(patsubst %.cpp,%.d,$(rshell.SOURCE))

.PHONY: all clean distclean
all: all-rshell
clean: clean-rshell clean-doc
distclean: distclean-rshell distclean-doc
	$(RM) -r bin

bin:
	mkdir -p bin

.PHONY: doc all-doc clean-doc distclean-doc
doc:
	doxygen doc/Doxyfile
all-doc: doc
clean-doc:
	$(RM) -r doc/output
distclean-doc: doc

.PHONY: test all-test clean-test distclean-test
test:
	sh tests/all.sh
all-test: test
clean-test:
distclean-test:

.PHONY: rshell all-rshell clean-rshell distclean-rshell
rshell: all-rshell
all-rshell: $(rshell.TARGET)
clean-rshell:
	$(RM) $(rshell.OBJECT)
	$(RM) $(rshell.DEPEND)
distclean-rshell: clean-rshell
	$(RM) $(rshell.TARGET)
$(rshell.TARGET): bin $(rshell.OBJECT)
	$(CXX) $(CXXFLAGS) -o $@ $(rshell.OBJECT)
-include $(rshell.DEPEND)

%.o: %.cpp makefile
	$(CXX) $(CXXFLAGS) -MMD -MP -MT $@ -o $@ -c $<
