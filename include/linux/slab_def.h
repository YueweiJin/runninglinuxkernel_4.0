#ifndef _LINUX_SLAB_DEF_H
#define	_LINUX_SLAB_DEF_H

#include <linux/reciprocal_div.h>

/*
 * Definitions unique to the original Linux SLAB allocator.
 */
/* JYW: 表示一个slab描述符 */
struct kmem_cache {
    /* JYW: 对象缓存池 */
	struct array_cache __percpu *cpu_cache;

/* 1) Cache tunables. Protected by slab_mutex */
    /*
     * JYW: 当前CPU的本地对象缓冲池array_cache为空时，
     * 从共享的缓冲池或者slabs_partial/slabs_free列表中获取对象的数目
     */
	unsigned int batchcount;
    /*
     * JYW: 当本地对象缓冲池的空闲对象数目大于limit时会主动释放batchcount个对象，
     *  便于内核回收和销毁slab
     */
	unsigned int limit;
	unsigned int shared;
    /* JYW: 对象的长度，加上align对齐字节 */
	unsigned int size;
	struct reciprocal_value reciprocal_buffer_size;
/* 2) touched by every alloc & free from the backend */

	unsigned int flags;		/* constant flags */
    /* JYW: 一个slab中最多可以有多少个对象 */
	unsigned int num;		/* # of objs per slab */

/* 3) cache_grow/shrink */
	/* order of pgs per slab (2^n) */
    /* JYW: 一个slab占用的页面阶数 */
	unsigned int gfporder;

	/* force GFP flags, e.g. GFP_DMA */
	gfp_t allocflags;

	size_t colour;			/* cache colouring range */
	unsigned int colour_off;	/* colour offset */
	struct kmem_cache *freelist_cache;
	unsigned int freelist_size;

	/* constructor func */
	void (*ctor)(void *obj);

/* 4) cache creation/removal */
    /* JYW: slab描述符的名称 */
	const char *name;
	struct list_head list;
	int refcount;
    /* JYW: 对象的实际大小 */
	int object_size;
    /* JYW: 对齐的长度 */
	int align;

/* 5) statistics */
#ifdef CONFIG_DEBUG_SLAB
	unsigned long num_active;
	unsigned long num_allocations;
	unsigned long high_mark;
	unsigned long grown;
	unsigned long reaped;
	unsigned long errors;
	unsigned long max_freeable;
	unsigned long node_allocs;
	unsigned long node_frees;
	unsigned long node_overflow;
	atomic_t allochit;
	atomic_t allocmiss;
	atomic_t freehit;
	atomic_t freemiss;

	/*
	 * If debugging is enabled, then the allocator can add additional
	 * fields and/or padding to every object. size contains the total
	 * object size including these internal fields, the following two
	 * variables contain the offset to the user object and its size.
	 */
	int obj_offset;
#endif /* CONFIG_DEBUG_SLAB */
#ifdef CONFIG_MEMCG_KMEM
	struct memcg_cache_params memcg_params;
#endif
    /* JYW: slab节点 */
	struct kmem_cache_node *node[MAX_NUMNODES];
};

#endif	/* _LINUX_SLAB_DEF_H */
