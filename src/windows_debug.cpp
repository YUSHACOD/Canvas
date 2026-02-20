
#include <windows.h>

#include "windows_debug.hpp"


// Debug File IO -------------------------------------------------------------------------------- //
DBG_PLAT_READ_ENTIRE_FILE(DBG_PlatReadEntireFile) {
    DBG_FileStruct result = {};

    HANDLE file_handle =
        CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(file_handle, &FileSize)) {
            result.memory =
                VirtualAlloc(0, FileSize.QuadPart + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            if (result.memory) {
                u32   FileSize32 = SafeTruncateU64(FileSize.QuadPart);
                DWORD BytesToRead;
                if (ReadFile(file_handle, result.memory, FileSize32, &BytesToRead, 0) &&
                    (FileSize32 == BytesToRead)) {
                    result.size                         = FileSize32;
                    ((char*)result.memory)[result.size] = '\0';
                } else {
                    if (result.memory) {
                        VirtualFree(result.memory, 0, MEM_RELEASE);
                    }
                }
            }
        }
        CloseHandle(file_handle);
    } else {
    }

    return result;
}

DBG_PLAT_FREE_FILE_MEMORY(DBG_PlatFreeFilememory) {
    if (memory) {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}


DBG_PLAT_WRITE_ENTIRE_FILE(DBG_PlatWriteEntireFile) {

    bool result = false;

    HANDLE file_handle = CreateFileA(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE) {
        if (memory) {
            u32   memorySize32 = SafeTruncateU64(memory_size);
            DWORD bytes_to_write;

            if (WriteFile(file_handle, memory, memorySize32, &bytes_to_write, 0)) {
                result = true;
            } else {
                OutputDebugStringA("Couldn't Write the File\n");
            }
        } else {
            OutputDebugStringA("The memory is Null\n");
        }
        CloseHandle(file_handle);
    } else {
        OutputDebugStringA("File, not opened\n");
    }

    return result;
}
// Debug File IO -------------------------------------------------------------------------------- //
