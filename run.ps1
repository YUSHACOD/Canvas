param(
	[string]$Target = "debug"
)

# -------------------------------
# Global settings
# -------------------------------
$CC = "cl"
$CFLAGS = @("-nologo", "-GR-", "-Gm-", "-MT", "-Zi", "-EHsc", "-EHa-", "-W4", "-WX", "-Fm")
$LINKER_FLAGS = @("/link", "-opt:ref")

$DISABLED_WARNINGS = @("-wd4100", "-wd4201", "-wd4996")

$DEBUG_FLAGS   = @("-Oi", "-DDEBUG=1")
$RELEASE_FLAGS = @("-Oi", "-O2")

$SRC_DIR = "src"
$SRC     = Get-ChildItem "$SRC_DIR/windows.cpp"

$OUTDIR  = "build"
$DBGDIR  = Join-Path $OUTDIR "debug"
$RELDIR  = Join-Path $OUTDIR "release"
$DEBUGGER = "raddbg"

$NAME = "Proj"

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
	& $CC $CFLAGS $DISABLED_WARNINGS $DEBUG_FLAGS "/Fe$NAME.exe" $SRC $LINKER_FLAGS
	Pop-Location
}

function Build-Release
{
	Folders
	Push-Location $RELDIR
	& $CC $CFLAGS $RELEASE_FLAGS "/Fe$NAME.exe" $SRC $LINKER_FLAGS
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

	"debugger"
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
