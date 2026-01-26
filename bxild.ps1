param(
	[string]$Target = "debug"
)

# -------------------------------
# Global settings
# -------------------------------
$CC = "cl"
$CFLAGS = @("-nologo", "-GR-", "-Gm-", "-MT", "-Zi", "-EHsc", "-EHa-", "-W4", "-WX", "-Fm", "-DCANVXS=1")
$LINKER_FLAGS = @("/link", "-incremental:no","-opt:ref")

$DISABLED_WARNINGS = @("-wd4100", "-wd4201", "-wd4996")

$DEBUG_FLAGS   = @("-Oi", "-DDEBUG=1")
$RELEASE_FLAGS = @("-Oi", "-O2")

$SRC_DIR = "src"
$EXE_SRC = Get-ChildItem "$SRC_DIR/windows.cpp"
$DLL_SRC = Get-ChildItem "$SRC_DIR/canvas.cpp"

$OUTDIR  = "build"
$DBGDIR  = Join-Path $OUTDIR "debug"
$RELDIR  = Join-Path $OUTDIR "release"
$DEBUGGER = "raddbg"

$NAME = "Canvxs"

# -------------------------------
# Utility functions
# -------------------------------
function Folders
{
	New-Item -ItemType Directory -Force -Path $DBGDIR | Out-Null
	New-Item -ItemType Directory -Force -Path $RELDIR | Out-Null
}

# -------------------------------
# Build targets
# -------------------------------
function Build-Debug
{
	Folders
	Push-Location $DBGDIR
	& $CC $CFLAGS $DISABLED_WARNINGS $DEBUG_FLAGS "/Fe$NAME.exe" $EXE_SRC $LINKER_FLAGS
	& $CC $CFLAGS $DISABLED_WARNINGS $DEBUG_FLAGS $DLL_SRC "/LD" $LINKER_FLAGS "/EXPORT:CanvasUpdateAndRender"
	Pop-Location
}

function Build-Release
{
	Folders
	Push-Location $RELDIR
	& $CC $CFLAGS $RELEASE_FLAGS "/Fe$NAME.exe" $EXE_SRC $LINKER_FLAGS
	Pop-Location
}

function RunProj
{
	Build-Debug
	& "$DBGDIR\$NAME.exe"
}

function RunInDebugger
{
	Build-Debug
	& $DEBUGGER "$DBGDIR\$NAME.exe"
}

function Clean
{
	if (Test-Path $OUTDIR)
	{
		Remove-Item -Recurse -Force $OUTDIR
		Write-Host "Cleaned build directory."
	} else
	{
		Write-Host "Nothing to clean."
	}
}

# -------------------------------
# Dispatcher
# -------------------------------
switch ($Target.ToLower())
{
	"debug"
	{ 
		Build-Debug
	}

	"release"
	{ 
		Build-Release 
	}

	"run"
	{ 
		RunProj
	}

	"dbg"
	{ 
		RunInDebugger
	}

	"clean"
	{ 
		Clean 
	}

	default
	{
		Write-Host "Unknown target: $Target"
		Write-Host "Available targets: debug, release, run, debugger, clean"
		exit 1
	}
}
