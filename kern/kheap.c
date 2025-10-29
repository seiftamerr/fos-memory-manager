#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

// NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
#define MAX_ALLOCATIONS ((KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE)
uint32 firstFreeVAInKHeap = KERNEL_HEAP_START ;

struct block {
    uint32 start;
    uint32 size;
};

struct block allocated_blocks[MAX_ALLOCATIONS];
int allocation_count = 0;

void* kmalloc(unsigned int size)
{
	uint32 reference  = firstFreeVAInKHeap;
    size = ROUNDUP(size, PAGE_SIZE);
    uint32 num_pages_needed = size / PAGE_SIZE;

    uint32 best_va = 0;
    uint32 best_size = 0;

    uint32 current_start = 0;
    uint32 current_size = 0;
    int inside_free = 0;

    for (uint32 va = KERNEL_HEAP_START; va < KERNEL_HEAP_MAX; va += PAGE_SIZE)
    {
        struct Frame_Info* frame_info = NULL;
        uint32* page_table = NULL;
        frame_info = get_frame_info(ptr_page_directory, (void*)va, &page_table);

        if (frame_info == NULL)
        {
            if (!inside_free)
            {
                inside_free = 1;
                current_start = va;
                current_size = PAGE_SIZE;
            }
            else
            {
                current_size += PAGE_SIZE;
            }
        }
        else
        {
            if (inside_free)
            {
                if (current_size >= size && current_size > best_size)
                {
                    best_va = current_start;
                    best_size = current_size;
                }
                inside_free = 0;
                current_size = 0;
            }
        }
    }

    if (inside_free && current_size >= size && current_size > best_size)
    {
        best_va = current_start;
        best_size = current_size;
    }

    if (best_size == 0)
        return NULL;

    for (uint32 i = 0; i < num_pages_needed; i++)
    {
        struct Frame_Info* frame_info = NULL;
        if (allocate_frame(&frame_info) != 0 || frame_info == NULL)
            return NULL;

        uint32 va = best_va + i * PAGE_SIZE;
        if (map_frame(ptr_page_directory, frame_info, (void*)va, PERM_PRESENT | PERM_WRITEABLE) != 0)
            return NULL;

        frame_info->references++;
    }

    // Properly track allocation
    if (allocation_count < MAX_ALLOCATIONS)
    {
        allocated_blocks[allocation_count].start = best_va;
        allocated_blocks[allocation_count].size = size;
        allocation_count++;
        firstFreeVAInKHeap+=PAGE_SIZE;
    }
    if (firstFreeVAInKHeap < best_va + size)
        firstFreeVAInKHeap = best_va + size;


    return (void*)best_va;
}




void kfree(void* virtual_address)
{
    uint32 va = (uint32)virtual_address;

    // Find allocated block index
    int block_index = -1;
    for (int i = 0; i < allocation_count; i++)
    {
        if (allocated_blocks[i].start == va)
        {
            block_index = i;
            break;
        }
    }

    if (block_index == -1)
        return; // Not allocated

    uint32 start = allocated_blocks[block_index].start;
    uint32 size = allocated_blocks[block_index].size;
    uint32 num_pages = size / PAGE_SIZE;

    // Unmap pages and adjust frame references
    for (uint32 i = 0; i < num_pages; i++)
    {
        uint32 addr = start + i * PAGE_SIZE;
        uint32* page_table = NULL;
        struct Frame_Info* frame_info = get_frame_info(ptr_page_directory, (void*)addr, &page_table);

        if (frame_info)
        {
            unmap_frame(ptr_page_directory, (void*)addr);
            frame_info->references--;
            if (frame_info->references == 0)
                free_frame(frame_info);
        }
    }

    // Remove the block correctly
    for (int i = block_index; i < allocation_count - 1; i++)
        allocated_blocks[i] = allocated_blocks[i + 1];

    allocation_count--;
}



unsigned int kheap_virtual_address(unsigned int physical_address)
{
	for(uint32 i = KERNEL_HEAP_START ; i< firstFreeVAInKHeap ; i+=PAGE_SIZE)
	{
		struct Frame_Info* frame_info = NULL;
		uint32* ptr_page_table;
		frame_info = get_frame_info(ptr_page_directory, (void*)i, &ptr_page_table);
		if(frame_info != NULL)
		{
			uint32 pa = to_physical_address(frame_info);
			if(pa == physical_address)
				return i;
		}
	}
	return 0;
}





unsigned int kheap_physical_address(unsigned int virtual_address)
{
	uint32* page_table = NULL;
	struct Frame_Info* frame_info = get_frame_info(ptr_page_directory, (void*)virtual_address, &page_table);

	if (frame_info != NULL)
	{
		return to_physical_address(frame_info);
	}
	return 0;
}

void* krealloc(void* virtual_address, uint32 new_size)
{
    panic("krealloc() is not required...!!");
    return NULL;
}

