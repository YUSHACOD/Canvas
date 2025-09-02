SHELL = pwsh.exe
.SHELLFLAGS = -NoProfile -Command

.PHONY: all clean build debug release run

CC      = cl
CFLAGS  = -EHsc
DEBUG_FLAGS   = -Zi -Od
RELEASE_FLAGS = -O2

SRC_DIR = src
SRC     = $(wildcard $(SRC_DIR)/windows.cpp)
OUTDIR  = build
DBGDIR  = $(OUTDIR)/debug
RELDIR  = $(OUTDIR)/release
DEBUGGER = raddbg

NAME = Canvas

all: debug

debug: $(DBGDIR) $(RELDIR)
	$(CC) -DDEBUG=1 $(CFLAGS) $(DEBUG_FLAGS) /Fe$(DBGDIR)/$(NAME).exe $(SRC)

release: $(DBGDIR) $(RELDIR)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) /Fe$(DBGDIR)/$(NAME).exe $(SRC)

run: debug
	$(DBGDIR)/$(NAME).exe

debugger: debug
	$(DEBUGGER) $(DBGDIR)/$(NAME).exe

FORCE:


$(DBGDIR) $(RELDIR):
	mkdir -Force $@
