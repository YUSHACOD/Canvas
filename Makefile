SHELL = pwsh.exe
.SHELLFLAGS = -NoProfile -Command

.PHONY: all clean build debug release run

CC      = cl
CFLAGS  = -nologo -EHsc
DEBUG_FLAGS   = -Zi -Od
RELEASE_FLAGS = -O2

SRC_DIR = src
SRC     = $(wildcard $(SRC_DIR)/main.cpp)
OUTDIR  = build
DBGDIR  = $(OUTDIR)/debug
RELDIR  = $(OUTDIR)/release
DEBUGGER = devenv

NAME = Canvas

all: debug

debug: $(DBGDIR) $(RELDIR)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) /Fe$(DBGDIR)/$(NAME).exe $(SRC)

release: $(DBGDIR) $(RELDIR)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) /Fe$(DBGDIR)/$(NAME).exe $(SRC)

run: debug
	$(DBGDIR)/$(NAME).exe

debugger: debug
	$(DEBUGGER) $(DBGDIR)/$(NAME).exe

FORCE:


$(DBGDIR) $(RELDIR):
	mkdir -Force $@

clean:
	Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $(DBGDIR),$(RELDIR)
