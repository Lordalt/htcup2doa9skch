#ifndef _LINUX_PAGEMAP_H
#define _LINUX_PAGEMAP_H

/*
 * Copyright 1995 Linus Torvalds
 */
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/highmem.h>
#include <linux/compiler.h>
#include <asm/uaccess.h>
#include <linux/gfp.h>
#include <linux/bitops.h>
#include <linux/hardirq.h> 
#include <linux/hugetlb_inline.h>

enum mapping_flags {
	AS_EIO		= __GFP_BITS_SHIFT + 0,	
	AS_ENOSPC	= __GFP_BITS_SHIFT + 1,	
	AS_MM_ALL_LOCKS	= __GFP_BITS_SHIFT + 2,	
	AS_UNEVICTABLE	= __GFP_BITS_SHIFT + 3,	
	AS_EXITING	= __GFP_BITS_SHIFT + 4, 
};

static inline void mapping_set_error(struct address_space *mapping, int error)
{
	if (unlikely(error)) {
		if (error == -ENOSPC)
			set_bit(AS_ENOSPC, &mapping->flags);
		else
			set_bit(AS_EIO, &mapping->flags);
	}
}

static inline void mapping_set_unevictable(struct address_space *mapping)
{
	set_bit(AS_UNEVICTABLE, &mapping->flags);
}

static inline void mapping_clear_unevictable(struct address_space *mapping)
{
	clear_bit(AS_UNEVICTABLE, &mapping->flags);
}

static inline int mapping_unevictable(struct address_space *mapping)
{
	if (mapping)
		return test_bit(AS_UNEVICTABLE, &mapping->flags);
	return !!mapping;
}

static inline void mapping_set_exiting(struct address_space *mapping)
{
	set_bit(AS_EXITING, &mapping->flags);
}

static inline int mapping_exiting(struct address_space *mapping)
{
	return test_bit(AS_EXITING, &mapping->flags);
}

static inline gfp_t mapping_gfp_mask(struct address_space * mapping)
{
	return (__force gfp_t)mapping->flags & __GFP_BITS_MASK;
}

static inline void mapping_set_gfp_mask(struct address_space *m, gfp_t mask)
{
	m->flags = (m->flags & ~(__force unsigned long)__GFP_BITS_MASK) |
				(__force unsigned long)mask;
}

#define PAGE_CACHE_SHIFT	PAGE_SHIFT
#define PAGE_CACHE_SIZE		PAGE_SIZE
#define PAGE_CACHE_MASK		PAGE_MASK
#define PAGE_CACHE_ALIGN(addr)	(((addr)+PAGE_CACHE_SIZE-1)&PAGE_CACHE_MASK)

#define page_cache_get(page)		get_page(page)
#define page_cache_release(page)	put_page(page)
void release_pages(struct page **pages, int nr, bool cold);

static inline int page_cache_get_speculative(struct page *page)
{
	VM_BUG_ON(in_interrupt());

#ifdef CONFIG_TINY_RCU
# ifdef CONFIG_PREEMPT_COUNT
	VM_BUG_ON(!in_atomic());
# endif
	VM_BUG_ON_PAGE(page_count(page) == 0, page);
	atomic_inc(&page->_count);

#else
	if (unlikely(!get_page_unless_zero(page))) {
		return 0;
	}
#endif
	VM_BUG_ON_PAGE(PageTail(page), page);

	return 1;
}

static inline int page_cache_add_speculative(struct page *page, int count)
{
	VM_BUG_ON(in_interrupt());

#if !defined(CONFIG_SMP) && defined(CONFIG_TREE_RCU)
# ifdef CONFIG_PREEMPT_COUNT
	VM_BUG_ON(!in_atomic());
# endif
	VM_BUG_ON_PAGE(page_count(page) == 0, page);
	atomic_add(count, &page->_count);

#else
	if (unlikely(!atomic_add_unless(&page->_count, count, 0)))
		return 0;
#endif
	VM_BUG_ON_PAGE(PageCompound(page) && page != compound_head(page), page);

	return 1;
}

static inline int page_freeze_refs(struct page *page, int count)
{
	return likely(atomic_cmpxchg(&page->_count, count, 0) == count);
}

static inline void page_unfreeze_refs(struct page *page, int count)
{
	VM_BUG_ON_PAGE(page_count(page) != 0, page);
	VM_BUG_ON(count == 0);

	atomic_set(&page->_count, count);
}

#ifdef CONFIG_NUMA
extern struct page *__page_cache_alloc(gfp_t gfp);
#else
static inline struct page *__page_cache_alloc(gfp_t gfp)
{
	return alloc_pages(gfp, 0);
}
#endif

static inline struct page *page_cache_alloc(struct address_space *x)
{
	return __page_cache_alloc(mapping_gfp_mask(x));
}

static inline struct page *page_cache_alloc_cold(struct address_space *x)
{
	return __page_cache_alloc(mapping_gfp_mask(x)|__GFP_COLD);
}

static inline struct page *page_cache_alloc_readahead(struct address_space *x)
{
	return __page_cache_alloc(mapping_gfp_mask(x) |
				  __GFP_COLD | __GFP_NORETRY | __GFP_NOWARN | __GFP_CMA);
}

typedef int filler_t(void *, struct page *);

pgoff_t page_cache_next_hole(struct address_space *mapping,
			     pgoff_t index, unsigned long max_scan);
pgoff_t page_cache_prev_hole(struct address_space *mapping,
			     pgoff_t index, unsigned long max_scan);

#define FGP_ACCESSED		0x00000001
#define FGP_LOCK		0x00000002
#define FGP_CREAT		0x00000004
#define FGP_WRITE		0x00000008
#define FGP_NOFS		0x00000010
#define FGP_NOWAIT		0x00000020

struct page *pagecache_get_page(struct address_space *mapping, pgoff_t offset,
		int fgp_flags, gfp_t cache_gfp_mask);

static inline struct page *find_get_page(struct address_space *mapping,
					pgoff_t offset)
{
	return pagecache_get_page(mapping, offset, 0, 0);
}

static inline struct page *find_get_page_flags(struct address_space *mapping,
					pgoff_t offset, int fgp_flags)
{
	return pagecache_get_page(mapping, offset, fgp_flags, 0);
}

static inline struct page *find_lock_page(struct address_space *mapping,
					pgoff_t offset)
{
	return pagecache_get_page(mapping, offset, FGP_LOCK, 0);
}

static inline struct page *find_or_create_page(struct address_space *mapping,
					pgoff_t offset, gfp_t gfp_mask)
{
	return pagecache_get_page(mapping, offset,
					FGP_LOCK|FGP_ACCESSED|FGP_CREAT,
					gfp_mask);
}

static inline struct page *grab_cache_page_nowait(struct address_space *mapping,
				pgoff_t index)
{
	return pagecache_get_page(mapping, index,
			FGP_LOCK|FGP_CREAT|FGP_NOFS|FGP_NOWAIT,
			mapping_gfp_mask(mapping));
}

struct page *find_get_entry(struct address_space *mapping, pgoff_t offset);
struct page *find_lock_entry(struct address_space *mapping, pgoff_t offset);
unsigned find_get_entries(struct address_space *mapping, pgoff_t start,
			  unsigned int nr_entries, struct page **entries,
			  pgoff_t *indices);
unsigned find_get_pages(struct address_space *mapping, pgoff_t start,
			unsigned int nr_pages, struct page **pages);
unsigned find_get_pages_contig(struct address_space *mapping, pgoff_t start,
			       unsigned int nr_pages, struct page **pages);
unsigned find_get_pages_tag(struct address_space *mapping, pgoff_t *index,
			int tag, unsigned int nr_pages, struct page **pages);

struct page *grab_cache_page_write_begin(struct address_space *mapping,
			pgoff_t index, unsigned flags);

static inline struct page *grab_cache_page(struct address_space *mapping,
								pgoff_t index)
{
	return find_or_create_page(mapping, index, mapping_gfp_mask(mapping));
}

extern struct page * read_cache_page(struct address_space *mapping,
				pgoff_t index, filler_t *filler, void *data);
extern struct page * read_cache_page_gfp(struct address_space *mapping,
				pgoff_t index, gfp_t gfp_mask);
extern int read_cache_pages(struct address_space *mapping,
		struct list_head *pages, filler_t *filler, void *data);

static inline struct page *read_mapping_page(struct address_space *mapping,
				pgoff_t index, void *data)
{
	filler_t *filler = (filler_t *)mapping->a_ops->readpage;
	return read_cache_page(mapping, index, filler, data);
}

static inline pgoff_t page_to_pgoff(struct page *page)
{
	if (unlikely(PageHeadHuge(page)))
		return page->index << compound_order(page);
	else
		return page->index << (PAGE_CACHE_SHIFT - PAGE_SHIFT);
}

static inline loff_t page_offset(struct page *page)
{
	return ((loff_t)page->index) << PAGE_CACHE_SHIFT;
}

static inline loff_t page_file_offset(struct page *page)
{
	return ((loff_t)page_file_index(page)) << PAGE_CACHE_SHIFT;
}

extern pgoff_t linear_hugepage_index(struct vm_area_struct *vma,
				     unsigned long address);

static inline pgoff_t linear_page_index(struct vm_area_struct *vma,
					unsigned long address)
{
	pgoff_t pgoff;
	if (unlikely(is_vm_hugetlb_page(vma)))
		return linear_hugepage_index(vma, address);
	pgoff = (address - vma->vm_start) >> PAGE_SHIFT;
	pgoff += vma->vm_pgoff;
	return pgoff >> (PAGE_CACHE_SHIFT - PAGE_SHIFT);
}

extern void __lock_page(struct page *page);
extern int __lock_page_killable(struct page *page);
extern int __lock_page_or_retry(struct page *page, struct mm_struct *mm,
				unsigned int flags);
extern void unlock_page(struct page *page);

static inline void __set_page_locked(struct page *page)
{
	__set_bit(PG_locked, &page->flags);
}

static inline void __clear_page_locked(struct page *page)
{
	__clear_bit(PG_locked, &page->flags);
}

static inline int trylock_page(struct page *page)
{
	return (likely(!test_and_set_bit_lock(PG_locked, &page->flags)));
}

static inline void lock_page(struct page *page)
{
	might_sleep();
	if (!trylock_page(page))
		__lock_page(page);
}

static inline int lock_page_killable(struct page *page)
{
	might_sleep();
	if (!trylock_page(page))
		return __lock_page_killable(page);
	return 0;
}

static inline int lock_page_or_retry(struct page *page, struct mm_struct *mm,
				     unsigned int flags)
{
	might_sleep();
	return trylock_page(page) || __lock_page_or_retry(page, mm, flags);
}

extern void wait_on_page_bit(struct page *page, int bit_nr);

extern int wait_on_page_bit_killable(struct page *page, int bit_nr);
extern int wait_on_page_bit_killable_timeout(struct page *page,
					     int bit_nr, unsigned long timeout);

static inline int wait_on_page_locked_killable(struct page *page)
{
	if (PageLocked(page))
		return wait_on_page_bit_killable(page, PG_locked);
	return 0;
}

extern wait_queue_head_t *page_waitqueue(struct page *page);
static inline void wake_up_page(struct page *page, int bit)
{
	__wake_up_bit(page_waitqueue(page), &page->flags, bit);
}

static inline void wait_on_page_locked(struct page *page)
{
	if (PageLocked(page))
		wait_on_page_bit(page, PG_locked);
}

static inline void wait_on_page_writeback(struct page *page)
{
	if (PageWriteback(page))
		wait_on_page_bit(page, PG_writeback);
}

extern void end_page_writeback(struct page *page);
void wait_for_stable_page(struct page *page);

void page_endio(struct page *page, int rw, int err);

extern void add_page_wait_queue(struct page *page, wait_queue_t *waiter);

static inline int fault_in_pages_writeable(char __user *uaddr, int size)
{
	int ret;

	if (unlikely(size == 0))
		return 0;

	ret = __put_user(0, uaddr);
	if (ret == 0) {
		char __user *end = uaddr + size - 1;

		if (((unsigned long)uaddr & PAGE_MASK) !=
				((unsigned long)end & PAGE_MASK))
			ret = __put_user(0, end);
	}
	return ret;
}

static inline int fault_in_pages_readable(const char __user *uaddr, int size)
{
	volatile char c;
	int ret;

	if (unlikely(size == 0))
		return 0;

	ret = __get_user(c, uaddr);
	if (ret == 0) {
		const char __user *end = uaddr + size - 1;

		if (((unsigned long)uaddr & PAGE_MASK) !=
				((unsigned long)end & PAGE_MASK)) {
			ret = __get_user(c, end);
			(void)c;
		}
	}
	return ret;
}

static inline int fault_in_multipages_writeable(char __user *uaddr, int size)
{
	int ret = 0;
	char __user *end = uaddr + size - 1;

	if (unlikely(size == 0))
		return ret;

	while (uaddr <= end) {
		ret = __put_user(0, uaddr);
		if (ret != 0)
			return ret;
		uaddr += PAGE_SIZE;
	}

	
	if (((unsigned long)uaddr & PAGE_MASK) ==
			((unsigned long)end & PAGE_MASK))
		ret = __put_user(0, end);

	return ret;
}

static inline int fault_in_multipages_readable(const char __user *uaddr,
					       int size)
{
	volatile char c;
	int ret = 0;
	const char __user *end = uaddr + size - 1;

	if (unlikely(size == 0))
		return ret;

	while (uaddr <= end) {
		ret = __get_user(c, uaddr);
		if (ret != 0)
			return ret;
		uaddr += PAGE_SIZE;
	}

	
	if (((unsigned long)uaddr & PAGE_MASK) ==
			((unsigned long)end & PAGE_MASK)) {
		ret = __get_user(c, end);
		(void)c;
	}

	return ret;
}

int add_to_page_cache_locked(struct page *page, struct address_space *mapping,
				pgoff_t index, gfp_t gfp_mask);
int add_to_page_cache_lru(struct page *page, struct address_space *mapping,
				pgoff_t index, gfp_t gfp_mask);
extern void delete_from_page_cache(struct page *page);
extern void __delete_from_page_cache(struct page *page, void *shadow);
int replace_page_cache_page(struct page *old, struct page *new, gfp_t gfp_mask);

static inline int add_to_page_cache(struct page *page,
		struct address_space *mapping, pgoff_t offset, gfp_t gfp_mask)
{
	int error;

	__set_page_locked(page);
	error = add_to_page_cache_locked(page, mapping, offset, gfp_mask);
	if (unlikely(error))
		__clear_page_locked(page);
	return error;
}

#endif 
