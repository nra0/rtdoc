CC				= gcc
CFLAGS		= -Wall

TARGET		= rtdoc

SRCDIR 		= src
BUILDDIR	= build

SRC				= $(wildcard $(SRCDIR)/*.c)
OBJ 			= $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRC))

all: checkdir $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

checkdir:
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)

.PHONY: checkdir clean

