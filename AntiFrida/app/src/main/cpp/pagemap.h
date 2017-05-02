/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _PAGEMAP_PAGEMAP_H
#define _PAGEMAP_PAGEMAP_H
#include <stdint.h>
#include <stdio.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <linux/kernel-page-flags.h>
__BEGIN_DECLS
typedef struct pm_proportional_swap pm_proportional_swap_t;
typedef struct pm_swap_offset pm_swap_offset_t;
struct pm_swap_offset {
    unsigned int offset;
    SIMPLEQ_ENTRY(pm_swap_offset) simpleqe;
};
typedef struct pm_memusage pm_memusage_t;
/* Holds the various metrics for memory usage of a process or a mapping. */
struct pm_memusage {
    size_t vss;
    size_t rss;
    size_t pss;
    size_t uss;
    size_t swap;
    /* if non NULL then use swap_offset_list to compute proportional swap */
    pm_proportional_swap_t *p_swap;
    SIMPLEQ_HEAD(simpleqhead, pm_swap_offset) swap_offset_list;
};
typedef struct pm_swapusage pm_swapusage_t;
struct pm_swapusage {
    size_t proportional;
    size_t unique;
};
/* Clears a memusage. */
void pm_memusage_zero(pm_memusage_t *mu);
/* Adds one memusage (a) to another (b). */
void pm_memusage_add(pm_memusage_t *a, pm_memusage_t *b);
/* Adds a swap offset */
void pm_memusage_pswap_add_offset(pm_memusage_t *mu, unsigned int offset);
/* Enable proportional swap computing. */
void pm_memusage_pswap_init_handle(pm_memusage_t *mu, pm_proportional_swap_t *p_swap);
/* Computes and return the proportional swap */
void pm_memusage_pswap_get_usage(pm_memusage_t *mu, pm_swapusage_t *su);
void pm_memusage_pswap_free(pm_memusage_t *mu);
/* Initialize a proportional swap computing handle:
   assumes only 1 swap device, total swap size of this device in bytes to be given as argument */
pm_proportional_swap_t * pm_memusage_pswap_create(int swap_size);
void pm_memusage_pswap_destroy(pm_proportional_swap_t *p_swap);
typedef struct pm_kernel   pm_kernel_t;
typedef struct pm_process  pm_process_t;
typedef struct pm_map      pm_map_t;
/* pm_kernel_t holds the state necessary to interface to the kernel's pagemap
 * system on a global level. */
struct pm_kernel {
    int kpagecount_fd;
    int kpageflags_fd;
    int pagesize;
};
/* pm_process_t holds the state necessary to interface to a particular process'
 * pagemap. */
struct pm_process {
    pm_kernel_t *ker;
    pid_t pid;
    pm_map_t **maps;
    int num_maps;
    int pagemap_fd;
};
/* pm_map_t holds the state necessary to access information about a particular
 * mapping in a particular process. */
struct pm_map {
    pm_process_t *proc;
    uint64_t start;
    uint64_t end;
    uint64_t offset;
    int flags;
    char *name;
};
/* Create a pm_kernel_t. */
int pm_kernel_create(pm_kernel_t **ker_out);
#define pm_kernel_pagesize(ker) ((ker)->pagesize)
/* Get a list of probably-existing PIDs (returned through *pids_out).
 * Length of the array (in sizeof(pid_t) units) is returned through *len.
 * The array should be freed by the caller. */
int pm_kernel_pids(pm_kernel_t *ker, pid_t **pids_out, size_t *len);
/* Get the map count (from /proc/kpagecount) of a physical frame.
 * The count is returned through *count_out. */
int pm_kernel_count(pm_kernel_t *ker, uint64_t pfn, uint64_t *count_out);
/* Get the page flags (from /proc/kpageflags) of a physical frame.
 * Flag constants are in <linux/kernel-page-flags.h>.
 * The count is returned through *flags_out.
 */
int pm_kernel_flags(pm_kernel_t *ker, uint64_t pfn, uint64_t *flags_out);
/* Destroy a pm_kernel_t. */
int pm_kernel_destroy(pm_kernel_t *ker);
/* Get the PID of a pm_process_t. */
#define pm_process_pid(proc) ((proc)->pid)
/* Create a pm_process_t and returns it through *proc_out.
 * Takes a pm_kernel_t, and the PID of the process. */
int pm_process_create(pm_kernel_t *ker, pid_t pid, pm_process_t **proc_out);
/* Get the total memory usage of a process and store in *usage_out. */
int pm_process_usage(pm_process_t *proc, pm_memusage_t *usage_out);
/* Get the total memory usage of a process and store in *usage_out, only
 * counting pages with specified flags. */
int pm_process_usage_flags(pm_process_t *proc, pm_memusage_t *usage_out,
                           uint64_t flags_mask, uint64_t required_flags);
/* Get the working set of a process (if ws_out != NULL), and reset it
 * (if reset != 0). */
int pm_process_workingset(pm_process_t *proc, pm_memusage_t *ws_out, int reset);
/* Get the PFNs corresponding to a range of virtual addresses.
 * The array of PFNs is returned through *range_out, and the caller has the
 * responsibility to free it. */
int pm_process_pagemap_range(pm_process_t *proc,
                             uint64_t low, uint64_t hi,
                             uint64_t **range_out, size_t *len);
#define _BITS(x, offset, bits) (((x) >> (offset)) & ((1LL << (bits)) - 1))
#define PM_PAGEMAP_PRESENT(x)     (_BITS(x, 63, 1))
#define PM_PAGEMAP_SWAPPED(x)     (_BITS(x, 62, 1))
#define PM_PAGEMAP_SHIFT(x)       (_BITS(x, 55, 6))
#define PM_PAGEMAP_PFN(x)         (_BITS(x, 0, 55))
#define PM_PAGEMAP_SWAP_OFFSET(x) (_BITS(x, 5, 50))
#define PM_PAGEMAP_SWAP_TYPE(x)   (_BITS(x, 0,  5))
/* Get the maps in the virtual address space of this process.
 * Returns an array of pointers to pm_map_t through *maps.
 * The array should be freed by the caller, but the maps should not be
 * modified or destroyed. */
int pm_process_maps(pm_process_t *proc, pm_map_t ***maps_out, size_t *len);
/* Destroy a pm_process_t. */
int pm_process_destroy(pm_process_t *proc);
/* Get the name, flags, start/end address, or offset of a map. */
#define pm_map_name(map)   ((map)->name)
#define pm_map_flags(map)  ((map)->flags)
#define PM_MAP_READ  1
#define PM_MAP_WRITE 2
#define PM_MAP_EXEC  4
#define PM_MAP_PERMISSIONS (PM_MAP_READ | PM_MAP_WRITE | PM_MAP_EXEC)
#define pm_map_start(map)  ((map)->start)
#define pm_map_end(map)    ((map)->end)
#define pm_map_offset(map) ((map)->offset)
/* Get the PFNs of the pages in the virtual address space of this map.
 * Array of PFNs is returned through *pagemap_out, and should be freed by the
 * caller. */
int pm_map_pagemap(pm_map_t *map, uint64_t **pagemap_out, size_t *len);
/* Get the memory usage of this map alone. */
int pm_map_usage(pm_map_t *map, pm_memusage_t *usage_out);
/* Get the memory usage of this map alone, only counting pages with specified
 * flags. */
int pm_map_usage_flags(pm_map_t *map, pm_memusage_t *usage_out,
                       uint64_t flags_mask, uint64_t required_flags);
/* Get the working set of this map alone. */
int pm_map_workingset(pm_map_t *map, pm_memusage_t *ws_out);
__END_DECLS
#endif