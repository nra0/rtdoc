CC        = gcc
CFLAGS    = -Wall
DFLAGS    = -DDEBUG -g

TARGET    = rtdoc
TTARGET   = test

SRCDIR    = src
TESTDIR   = tests tests/unit
BUILDDIR  = build
MAIN      = main

SRC       = $(filter-out %$(MAIN).c,$(wildcard $(SRCDIR)/*.c))
OBJ       = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRC))
TEST      = $(foreach dir,$(TESTDIR),$(wildcard $(dir)/*.c))
TOBJ      = $(patsubst $(TESTDIR)/%.c,$(BUILDDIR)/%.o,$(TEST))
MOBJ      = $(BUILDDIR)/$(MAIN).o


all: checkdir $(TARGET)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(TARGET): $(OBJ) $(MOBJ)
	$(CC) $^ -o $@

check: CFLAGS += $(DFLAGS)
check: checkdir $(TTARGET)

debug: CFLAGS += $(DFLAGS)
debug: checkdir $(TARGET)

$(TTARGET): $(OBJ) $(TOBJ)
	$(CC) $(CFLAGS) $^ -o $@

checkdir:
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR) $(TARGET) $(TTARGET)

.PHONY: checkdir clean
