#include <inc/lib.h>



#define UHEAP_START USER_HEAP_START

#define UHEAP_END   USER_HEAP_MAX

#define UHEAP_SIZE  (UHEAP_END - UHEAP_START)

#define MAX_ALLOCATIONS (UHEAP_SIZE / PAGE_SIZE)



#ifndef UINT32_MAX

#define UINT32_MAX 0xFFFFFFFF

#endif



#define MAX_USER_ALLOCATIONS 1024

#define UHEAP_START USER_HEAP_START
#define UHEAP_END   USER_HEAP_MAX
#define UHEAP_SIZE  (UHEAP_END - UHEAP_START)
#define MAX_ALLOCATIONS (UHEAP_SIZE / PAGE_SIZE)
#define MAX_USER_ALLOCATIONS 1024

struct UserAllocation {
    uint32 start;
    uint32 size;
};

struct UserAllocation user_allocations[MAX_USER_ALLOCATIONS];
int user_alloc_count = 0;

struct UserHeapBlock {
    uint32 va;
    uint32 size;
    uint8 used;
};

static struct UserHeapBlock uheap_blocks[MAX_ALLOCATIONS];
static int last_fit_index = 0;

void* malloc(uint32 size)
{
    if (size == 0) return NULL;

    size = ROUNDUP(size, PAGE_SIZE);

    static int initialized = 0;
    if (!initialized)
    {
        for (int i = 0; i < MAX_ALLOCATIONS; i++)
            uheap_blocks[i] = (struct UserHeapBlock){0, 0, 0};

        uheap_blocks[0].va = UHEAP_START;
        uheap_blocks[0].size = UHEAP_SIZE;
        uheap_blocks[0].used = 0;

        initialized = 1;
    }

    int start = last_fit_index;
    for (int n = 0; n < MAX_ALLOCATIONS; n++)
    {
        int i = (start + n) % MAX_ALLOCATIONS;

        if (!uheap_blocks[i].used && uheap_blocks[i].size >= size && uheap_blocks[i].va >= UHEAP_START)
        {
            uint32 alloc_va = uheap_blocks[i].va;
            uint32 remaining = uheap_blocks[i].size - size;

            if (remaining > 0)
            {
                int new_index = -1;
                for (int j = 0; j < MAX_ALLOCATIONS; j++)
                {
                    if (uheap_blocks[j].va == 0 && uheap_blocks[j].size == 0 && !uheap_blocks[j].used)
                    {
                        new_index = j;
                        break;
                    }
                }

                if (new_index == -1) return NULL;

                uheap_blocks[new_index].va = alloc_va + size;
                uheap_blocks[new_index].size = remaining;
                uheap_blocks[new_index].used = 0;

                uheap_blocks[i].size = size;
            }

            uheap_blocks[i].used = 1;
            last_fit_index = (i + 1) % MAX_ALLOCATIONS;

            if (user_alloc_count < MAX_USER_ALLOCATIONS)
            {
                user_allocations[user_alloc_count].start = alloc_va;
                user_allocations[user_alloc_count].size = size;
                user_alloc_count++;
            }

            sys_allocateMem((uint32)alloc_va, size);
            return (void*)alloc_va;
        }
    }

    return NULL;
}

void free(void* virtual_address)
{
    if (!virtual_address) return;

    uint32 va = ROUNDDOWN((uint32)virtual_address, PAGE_SIZE);

    int block_index = -1;
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        if (uheap_blocks[i].used && uheap_blocks[i].va == va) {
            block_index = i;
            break;
        }
    }

    if (block_index == -1) return;

    uint32 block_size = uheap_blocks[block_index].size;
    uheap_blocks[block_index].used = 0;

    for (int i = 0; i < user_alloc_count; i++) {
        if (user_allocations[i].start == va) {
            for (int j = i; j < user_alloc_count - 1; j++) {
                user_allocations[j] = user_allocations[j + 1];
            }
            user_alloc_count--;
            break;
        }
    }

    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        if (i == block_index || uheap_blocks[i].used || uheap_blocks[i].size == 0)
            continue;

        if (uheap_blocks[block_index].va + uheap_blocks[block_index].size == uheap_blocks[i].va) {
            uheap_blocks[block_index].size += uheap_blocks[i].size;
            uheap_blocks[i].va = 0;
            uheap_blocks[i].size = 0;
            uheap_blocks[i].used = 0;
        }
        else if (uheap_blocks[i].va + uheap_blocks[i].size == uheap_blocks[block_index].va) {
            uheap_blocks[i].size += uheap_blocks[block_index].size;
            uheap_blocks[block_index].va = 0;
            uheap_blocks[block_index].size = 0;
            uheap_blocks[block_index].used = 0;
            block_index = i;
        }
    }

    sys_freeMem((uint32)va, block_size);
}

// Stubs for unimplemented shared memory features
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
    panic("smalloc() is not required...!!");
    return 0;
}

void* sget(int32 ownerEnvID, char *sharedVarName)
{
    panic("sget() is not required ...!!");
    return 0;
}

void sfree(void* virtual_address)
{
    panic("sfree() is not required ...!!");
}

void *realloc(void *virtual_address, uint32 new_size)
{
    panic("realloc() is not required yet...!!");
}
