# FOS Memory Manager 

## 📌 Overview
This project is part of the **Operating Systems [OS'25] course** at Ain Shams University.  
It extends Milestone 1 (kernel heap allocator) by implementing **virtual memory management** features.

### ✅ Milestone 1 Recap
-## 🚀 Phase 1 Features
- **Kernel Heap Allocation**
  - Implemented `kmalloc(size)` to dynamically allocate memory in the kernel heap.
  - Implemented `kfree(va)` to free previously allocated memory (unmapping pages only, without deleting kernel tables).
- **Placement Strategies**
  - **Worst Fit** (initial implementation).
  - **Best Fit** (extended in Phase 2, but included here for testing).
- **Address Translation Helpers**
  - `kheap_virtual_address(pa)` → find virtual address for a given physical address.
  - `kheap_physical_address(va)` → find physical address for a given kernel virtual address.

---

---

### 🚀 Milestone 2 Features
1. **Page Fault Handler**
   - Handles page faults in the kernel (`trap.c`).
   - Implements **Modified Clock** page replacement algorithm.
   - Supports **page buffering** (free & modified lists).
   - Handles stack page growth and page reclamation.

2. **User Heap Management**
   - Implemented `malloc()` and `free()` for user programs (`lib/uheap.c`).
   - Uses **Best Fit strategy** for allocation.
   - Uses system calls (`sys_allocateMem`, `sys_freeMem`) to switch to kernel mode.
   - Kernel-side implementation (`allocateMem`, `__freeMem_with_buffering`).

3. **Environment Free**
   - Proper cleanup of:
     - All working set pages
     - Page tables
     - Page directory
     - Page file entries
   - Frees buffered modified pages correctly.

---


