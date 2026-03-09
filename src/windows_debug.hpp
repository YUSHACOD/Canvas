#ifndef WIN_DEBUG
#define WIN_DEBUG
//  windows debug code interfaces : -------------------------------------------------- (section)  //

#include "base/sugars.hpp"

typedef struct {
    void* memory;
    u64   size;
} DBG_FileStruct;

#define DBG_PLAT_READ_ENTIRE_FILE(name) DBG_FileStruct name(char* file_name)
typedef DBG_PLAT_READ_ENTIRE_FILE(dbg_plat_read_entire_file);

#define DBG_PLAT_FREE_FILE_MEMORY(name) void name(void* memory)
typedef DBG_PLAT_FREE_FILE_MEMORY(dbg_plat_free_file_memory);

#define DBG_PLAT_WRITE_ENTIRE_FILE(name) bool name(char* file_name, void* memory, u32 memory_size)
typedef DBG_PLAT_WRITE_ENTIRE_FILE(dbg_plat_write_entire_file);

//  (section) -------------------------------------------------- : windows debug code interfaces  //
#endif
