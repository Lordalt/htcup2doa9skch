#ifndef __LINUX_GFP_H
#define __LINUX_GFP_H

#include <linux/mmdebug.h>
#include <linux/mmzone.h>
#include <linux/stddef.h>
#include <linux/linkage.h>
#include <linux/topology.h>

struct vm_area_struct;

#define ___GFP_DMA		0x01u
#define ___GFP_HIGHMEM		0x02u
#define ___GFP_DMA32		0x04u
#define ___GFP_MOVABLE		0x08u
#define ___GFP_WAIT		0x10u
#define ___GFP_HIGH		0x20u
#define ___GFP_IO		0x40u
#define ___GFP_FS		0x80u
#define ___GFP_COLD		0x100u
#define ___GFP_NOWARN		0x200u
#define ___GFP_REPEAT		0x400u
#define ___GFP_NOFAIL		0x800u
#define ___GFP_NORETRY		0x1000u
#define ___GFP_MEMALLOC		0x2000u
#define ___GFP_COMP		0x4000u
#define ___GFP_ZERO		0x8000u
#define ___GFP_NOMEMALLOC	0x10000u
#define ___GFP_HARDWALL		0x20000u
#define ___GFP_THISNODE		0x40000u
#define ___GFP_RECLAIMABLE	0x80000u
#define ___GFP_NOTRACK		0x200000u
#define ___GFP_NO_KSWAPD	0x400000u
#define ___GFP_OTHER_NODE	0x800000u
#define ___GFP_WRITE		0x1000000u
#define ___GFP_CMA		0x2000000u

#define __GFP_DMA	((__force gfp_t)___GFP_DMA)
#define __GFP_HIGHMEM	((__force gfp_t)___GFP_HIGHMEM)
#define __GFP_DMA32	((__force gfp_t)___GFP_DMA32)
#define __GFP_MOVABLE	((__force gfp_t)___GFP_MOVABLE)  
#define GFP_ZONEMASK	(__GFP_DMA|__GFP_HIGHMEM|__GFP_DMA32|__GFP_MOVABLE)
#define __GFP_WAIT	((__force gfp_t)___GFP_WAIT)	
#define __GFP_HIGH	((__force gfp_t)___GFP_HIGH)	
#define __GFP_IO	((__force gfp_t)___GFP_IO)	
#define __GFP_FS	((__force gfp_t)___GFP_FS)	
#define __GFP_COLD	((__force gfp_t)___GFP_COLD)	
#define __GFP_NOWARN	((__force gfp_t)___GFP_NOWARN)	
#define __GFP_REPEAT	((__force gfp_t)___GFP_REPEAT)	
#define __GFP_NOFAIL	((__force gfp_t)___GFP_NOFAIL)	
#define __GFP_NORETRY	((__force gfp_t)___GFP_NORETRY) 
#define __GFP_MEMALLOC	((__force gfp_t)___GFP_MEMALLOC)
#define __GFP_COMP	((__force gfp_t)___GFP_COMP)	
#define __GFP_ZERO	((__force gfp_t)___GFP_ZERO)	
#define __GFP_NOMEMALLOC ((__force gfp_t)___GFP_NOMEMALLOC) 
#define __GFP_HARDWALL   ((__force gfp_t)___GFP_HARDWALL) 
#define __GFP_THISNODE	((__force gfp_t)___GFP_THISNODE)
#define __GFP_RECLAIMABLE ((__force gfp_t)___GFP_RECLAIMABLE) 
#define __GFP_NOTRACK	((__force gfp_t)___GFP_NOTRACK)  

#define __GFP_NO_KSWAPD	((__force gfp_t)___GFP_NO_KSWAPD)
#define __GFP_OTHER_NODE ((__force gfp_t)___GFP_OTHER_NODE) 
#define __GFP_WRITE	((__force gfp_t)___GFP_WRITE)	
#define __GFP_CMA	((__force gfp_t)___GFP_CMA)	

#define __GFP_NOTRACK_FALSE_POSITIVE (__GFP_NOTRACK)

#define __GFP_BITS_SHIFT 26	
#define __GFP_BITS_MASK ((__force gfp_t)((1 << __GFP_BITS_SHIFT) - 1))

#define GFP_NOWAIT	(GFP_ATOMIC & ~__GFP_HIGH)
#define GFP_ATOMIC	(__GFP_HIGH)
#define GFP_NOIO	(__GFP_WAIT)
#define GFP_NOFS	(__GFP_WAIT | __GFP_IO)
#define GFP_KERNEL	(__GFP_WAIT | __GFP_IO | __GFP_FS)
#define GFP_TEMPORARY	(__GFP_WAIT | __GFP_IO | __GFP_FS | \
			 __GFP_RECLAIMABLE)
#define GFP_USER	(__GFP_WAIT | __GFP_IO | __GFP_FS | __GFP_HARDWALL)
#define GFP_HIGHUSER	(__GFP_WAIT | __GFP_IO | __GFP_FS | __GFP_HARDWALL | \
			 __GFP_HIGHMEM)
#define GFP_HIGHUSER_MOVABLE	(__GFP_WAIT | __GFP_IO | __GFP_FS | \
				 __GFP_HARDWALL | __GFP_HIGHMEM | \
				 __GFP_MOVABLE)
#define GFP_IOFS	(__GFP_IO | __GFP_FS)
#define GFP_TRANSHUGE	(GFP_HIGHUSER_MOVABLE | __GFP_COMP | \
			 __GFP_NOMEMALLOC | __GFP_NORETRY | __GFP_NOWARN | \
			 __GFP_NO_KSWAPD)

#ifdef CONFIG_NUMA
#define GFP_THISNODE	(__GFP_THISNODE | __GFP_NOWARN | __GFP_NORETRY)
#else
#define GFP_THISNODE	((__force gfp_t)0)
#endif

#define GFP_MOVABLE_MASK (__GFP_RECLAIMABLE|__GFP_MOVABLE)

#define GFP_RECLAIM_MASK (__GFP_WAIT|__GFP_HIGH|__GFP_IO|__GFP_FS|\
			__GFP_NOWARN|__GFP_REPEAT|__GFP_NOFAIL|\
			__GFP_NORETRY|__GFP_MEMALLOC|__GFP_NOMEMALLOC)

#define GFP_BOOT_MASK (__GFP_BITS_MASK & ~(__GFP_WAIT|__GFP_IO|__GFP_FS))

#define GFP_CONSTRAINT_MASK (__GFP_HARDWALL|__GFP_THISNODE)

#define GFP_SLAB_BUG_MASK (__GFP_DMA32|__GFP_HIGHMEM|~__GFP_BITS_MASK)


#define GFP_DMA		__GFP_DMA

#define GFP_DMA32	__GFP_DMA32

static inline int gfpflags_to_migratetype(const gfp_t gfp_flags)
{
	WARN_ON((gfp_flags & GFP_MOVABLE_MASK) == GFP_MOVABLE_MASK);

	if (unlikely(page_group_by_mobility_disabled))
		return MIGRATE_UNMOVABLE;

	
	return (((gfp_flags & __GFP_MOVABLE) != 0) << 1) |
		((gfp_flags & __GFP_RECLAIMABLE) != 0);
}

#ifdef CONFIG_HIGHMEM
#define OPT_ZONE_HIGHMEM ZONE_HIGHMEM
#else
#define OPT_ZONE_HIGHMEM ZONE_NORMAL
#endif

#ifdef CONFIG_ZONE_DMA
#define OPT_ZONE_DMA ZONE_DMA
#else
#define OPT_ZONE_DMA ZONE_NORMAL
#endif

#ifdef CONFIG_ZONE_DMA32
#define OPT_ZONE_DMA32 ZONE_DMA32
#else
#define OPT_ZONE_DMA32 ZONE_NORMAL
#endif


#if 16 * ZONES_SHIFT > BITS_PER_LONG
#error ZONES_SHIFT too large to create GFP_ZONE_TABLE integer
#endif

#define GFP_ZONE_TABLE ( \
	(ZONE_NORMAL << 0 * ZONES_SHIFT)				      \
	| (OPT_ZONE_DMA << ___GFP_DMA * ZONES_SHIFT)			      \
	| (OPT_ZONE_HIGHMEM << ___GFP_HIGHMEM * ZONES_SHIFT)		      \
	| (OPT_ZONE_DMA32 << ___GFP_DMA32 * ZONES_SHIFT)		      \
	| (ZONE_NORMAL << ___GFP_MOVABLE * ZONES_SHIFT)			      \
	| (OPT_ZONE_DMA << (___GFP_MOVABLE | ___GFP_DMA) * ZONES_SHIFT)	      \
	| (ZONE_MOVABLE << (___GFP_MOVABLE | ___GFP_HIGHMEM) * ZONES_SHIFT)   \
	| (OPT_ZONE_DMA32 << (___GFP_MOVABLE | ___GFP_DMA32) * ZONES_SHIFT)   \
)

#define GFP_ZONE_BAD ( \
	1 << (___GFP_DMA | ___GFP_HIGHMEM)				      \
	| 1 << (___GFP_DMA | ___GFP_DMA32)				      \
	| 1 << (___GFP_DMA32 | ___GFP_HIGHMEM)				      \
	| 1 << (___GFP_DMA | ___GFP_DMA32 | ___GFP_HIGHMEM)		      \
	| 1 << (___GFP_MOVABLE | ___GFP_HIGHMEM | ___GFP_DMA)		      \
	| 1 << (___GFP_MOVABLE | ___GFP_DMA32 | ___GFP_DMA)		      \
	| 1 << (___GFP_MOVABLE | ___GFP_DMA32 | ___GFP_HIGHMEM)		      \
	| 1 << (___GFP_MOVABLE | ___GFP_DMA32 | ___GFP_DMA | ___GFP_HIGHMEM)  \
)

static inline enum zone_type gfp_zone(gfp_t flags)
{
	enum zone_type z;
	int bit = (__force int) (flags & GFP_ZONEMASK);

	z = (GFP_ZONE_TABLE >> (bit * ZONES_SHIFT)) &
					 ((1 << ZONES_SHIFT) - 1);
	VM_BUG_ON((GFP_ZONE_BAD >> bit) & 1);
	
#if defined(CONFIG_CMA) && defined(CONFIG_MTK_SVP)
	if (z == ZONE_MOVABLE && !(flags & __GFP_CMA))
		z -= 1;
#endif
	return z;
}


static inline int gfp_zonelist(gfp_t flags)
{
	if (IS_ENABLED(CONFIG_NUMA) && unlikely(flags & __GFP_THISNODE))
		return 1;

	return 0;
}

static inline struct zonelist *node_zonelist(int nid, gfp_t flags)
{
	return NODE_DATA(nid)->node_zonelists + gfp_zonelist(flags);
}

#ifndef HAVE_ARCH_FREE_PAGE
static inline void arch_free_page(struct page *page, int order) { }
#endif
#ifndef HAVE_ARCH_ALLOC_PAGE
static inline void arch_alloc_page(struct page *page, int order) { }
#endif

struct page *
__alloc_pages_nodemask(gfp_t gfp_mask, unsigned int order,
		       struct zonelist *zonelist, nodemask_t *nodemask);

static inline struct page *
__alloc_pages(gfp_t gfp_mask, unsigned int order,
		struct zonelist *zonelist)
{
	return __alloc_pages_nodemask(gfp_mask, order, zonelist, NULL);
}

static inline struct page *alloc_pages_node(int nid, gfp_t gfp_mask,
						unsigned int order)
{
	
	if (nid < 0)
		nid = numa_node_id();

	return __alloc_pages(gfp_mask, order, node_zonelist(nid, gfp_mask));
}

static inline struct page *alloc_pages_exact_node(int nid, gfp_t gfp_mask,
						unsigned int order)
{
	VM_BUG_ON(nid < 0 || nid >= MAX_NUMNODES || !node_online(nid));

	return __alloc_pages(gfp_mask, order, node_zonelist(nid, gfp_mask));
}

#ifdef CONFIG_NUMA
extern struct page *alloc_pages_current(gfp_t gfp_mask, unsigned order);

static inline struct page *
alloc_pages(gfp_t gfp_mask, unsigned int order)
{
	return alloc_pages_current(gfp_mask, order);
}
extern struct page *alloc_pages_vma(gfp_t gfp_mask, int order,
			struct vm_area_struct *vma, unsigned long addr,
			int node);
#else
#define alloc_pages(gfp_mask, order) \
		alloc_pages_node(numa_node_id(), gfp_mask, order)
#define alloc_pages_vma(gfp_mask, order, vma, addr, node)	\
	alloc_pages(gfp_mask, order)
#endif
#define alloc_page(gfp_mask) alloc_pages(gfp_mask, 0)
#define alloc_page_vma(gfp_mask, vma, addr)			\
	alloc_pages_vma(gfp_mask, 0, vma, addr, numa_node_id())
#define alloc_page_vma_node(gfp_mask, vma, addr, node)		\
	alloc_pages_vma(gfp_mask, 0, vma, addr, node)

extern struct page *alloc_kmem_pages(gfp_t gfp_mask, unsigned int order);
extern struct page *alloc_kmem_pages_node(int nid, gfp_t gfp_mask,
					  unsigned int order);

extern unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order);
extern unsigned long get_zeroed_page(gfp_t gfp_mask);

void *alloc_pages_exact(size_t size, gfp_t gfp_mask);
void free_pages_exact(void *virt, size_t size);
void * __meminit alloc_pages_exact_nid(int nid, size_t size, gfp_t gfp_mask);

#define __get_free_page(gfp_mask) \
		__get_free_pages((gfp_mask), 0)

#define __get_dma_pages(gfp_mask, order) \
		__get_free_pages((gfp_mask) | GFP_DMA, (order))

extern void __free_pages(struct page *page, unsigned int order);
extern void free_pages(unsigned long addr, unsigned int order);
extern void free_hot_cold_page(struct page *page, bool cold);
extern void free_hot_cold_page_list(struct list_head *list, bool cold);

extern void __free_kmem_pages(struct page *page, unsigned int order);
extern void free_kmem_pages(unsigned long addr, unsigned int order);

#define __free_page(page) __free_pages((page), 0)
#define free_page(addr) free_pages((addr), 0)

void page_alloc_init(void);
void drain_zone_pages(struct zone *zone, struct per_cpu_pages *pcp);
void drain_all_pages(void);
void drain_local_pages(void *dummy);

extern gfp_t gfp_allowed_mask;

bool gfp_pfmemalloc_allowed(gfp_t gfp_mask);

extern void pm_restrict_gfp_mask(void);
extern void pm_restore_gfp_mask(void);

#ifdef CONFIG_PM_SLEEP
extern bool pm_suspended_storage(void);
#else
static inline bool pm_suspended_storage(void)
{
	return false;
}
#endif 

#ifdef CONFIG_CMA

extern int alloc_contig_range(unsigned long start, unsigned long end,
			      unsigned migratetype);
extern void free_contig_range(unsigned long pfn, unsigned nr_pages);

extern void init_cma_reserved_pageblock(struct page *page);

#endif

#endif 
