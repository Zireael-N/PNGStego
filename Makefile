CXX ?= g++
CXXFLAGS = --std=c++11 -O3 -DNDEBUG -Wall -Wextra
LIBS = -lpng -lz
LDFLAGS =
RM = rm -f
DEVNULL = /dev/null
VERSIONRES =
MKDIR = mkdir -p

SRCDIR = src/
HEADERS = include/

TEMPDIR = temp/
DEPENDS = $(TEMPDIR).depend

CXXFLAGS += -I"$(HEADERS)"

SRCS = $(wildcard $(SRCDIR)*.cpp)
OBJS = $(addprefix $(TEMPDIR), $(filter-out main-stego.o main-destego.o, $(notdir $(SRCS:.cpp=.o))))

ENCODER = PNGStego
DECODER = PNGDeStego
TESTEXECUTABLE = test

ISCYGWIN = 0
ISMINGW = 0
ISCMDEXE = 0

ifeq ($(OS),Windows_NT)
	ENCODER := $(ENCODER).exe
	DECODER := $(DECODER).exe
	TESTEXECUTABLE := $(TESTEXECUTABLE).exe
	VERSIONRES = $(TEMPDIR)version.res
ifeq ($(PWD),)
	# ! cmd.exe !
	ISCMDEXE = 1
	# Check if your boost libraries have the same name
	LIBS += -lboost_nowide-mgw52-mt-1_59 -lboost_iostreams-mgw52-mt-1_59 -lbz2
	LIBS += -static-libstdc++ -static-libgcc
	# You might need to replace -lpthread with
	# -lws2_32, depends on what MinGW you have
	LIBS += -lcryptopp -static -lpthread
	RM = DEL /F /Q
	MKDIR = mkdir
	DEVNULL = NUL
else
	# ! MSYS2 / Cygwin shell !
	ISCYGWIN = $(shell uname | egrep -c "CYGWIN")
	ISMINGW = $(shell uname | egrep -c "MINGW")
ifeq ($(ISCYGWIN),1)
	LIBS += -lboost_iostreams -lbz2
	LIBS += -lcryptopp -lpthread
endif # ISCYGWIN
ifeq ($(ISMINGW),1)
	LIBS += -lboost_nowide-mt -lboost_iostreams-mt -lbz2
	LIBS += -static-libstdc++ -static-libgcc
	# You might need to replace -lpthread with
	# -lws2_32, depends on what MinGW you have
	LIBS += -lcryptopp -static -lpthread
endif # ISMINGW
endif # PWD
else
	LIBS += -lboost_iostreams -lbz2
	LIBS += -lcryptopp -lpthread
	UNAME := $(shell uname -s)
	ifneq ($(UNAME),Darwin)
		# convert to lowercase
		ENCODER := $(shell echo $(ENCODER) | tr A-Z a-z)
		DECODER := $(shell echo $(DECODER) | tr A-Z a-z)
	endif
	ifneq ($(UNAME),Linux)
		# OS X's compiler doesn't check these directories by default
		LDFLAGS = -L"/usr/local/lib"
		CXXFLAGS += -isystem "/usr/local/include"
    endif
endif

default: all

# find dependencies automatically
$(DEPENDS): $(SRCS)
ifeq ($(ISCMDEXE),1)
	@if not exist $(TEMPDIR) $(MKDIR) $(subst /,,$(TEMPDIR))
	@-$(RM) $(subst /,\,$(DEPENDS)) 2>$(DEVNULL)
	$(CXX) $(CXXFLAGS) -MM $^ >> $(subst /,\,$(DEPENDS))
else
	@$(MKDIR) $(subst /,,$(TEMPDIR))
	@-$(RM) $(DEPENDS) 2>$(DEVNULL)
	$(CXX) $(CXXFLAGS) -MM $^ >> $(DEPENDS)
endif

-include $(DEPENDS)
# ^ this include automatically invokes $(DEPENDS) :(

all: $(ENCODER) $(DECODER)

$(ENCODER): $(OBJS) $(VERSIONRES)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCDIR)main-stego.cpp $^ $(LDFLAGS) $(LIBS)

$(DECODER): $(OBJS) $(VERSIONRES)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCDIR)main-destego.cpp $^ $(LDFLAGS) $(LIBS)

$(TEMPDIR)%.o: $(SRCDIR)%.cpp $(DEPENDS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

ifeq ($(OS),Windows_NT)
$(VERSIONRES): ./msvc/version.rc
	windres ./msvc/version.rc -O coff -o $@
endif

# do not invoke these on Windows
ifneq ($(ISCMDEXE),1)
install: $(ENCODER) $(DECODER)
	cp $(ENCODER) /usr/bin/$(ENCODER)
	cp $(DECODER) /usr/bin/$(DECODER)

uninstall:
	if [ -a /usr/bin/$(ENCODER) ] ; \
	then \
		$(RM) /usr/bin/$(ENCODER) ; \
	fi;
	if [ -a /usr/bin/$(DECODER) ] ; \
	then \
		$(RM) /usr/bin/$(DECODER) ; \
	fi;
endif

.PHONY: clean

# convert forward slashes
# to backslashes if needed
clean:
ifeq ($(ISCMDEXE),1)
	$(RM) $(subst /,\,$(OBJS) $(ENCODER) $(DECODER) $(DEPENDS) $(VERSIONRES))
else
	$(RM) $(OBJS) $(ENCODER) $(DECODER) $(DEPENDS) $(VERSIONRES)
endif


TESTSDIR = tests/

# tests are not for distribution
# no reason to statically link on any OS
TLIBS = $(filter-out -static -static-libstdc++ -static-libgcc, $(LIBS))

.PHONY: clean-test

# convert forward slashes
# to backslashes if needed
clean-test:
ifeq ($(ISCMDEXE),1)
	$(RM) $(subst /,\,$(TESTSDIR)$(TESTEXECUTABLE))
else
	$(RM) $(TESTSDIR)$(TESTEXECUTABLE)
endif

$(TESTSDIR)$(TESTEXECUTABLE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(TESTSDIR)tests.cpp $^ $(LDFLAGS) $(TLIBS)

test: $(TESTSDIR)$(TESTEXECUTABLE)
	$(TESTSDIR)$(TESTEXECUTABLE)