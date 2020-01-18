#ifndef _LINUX_SLUB_DEF_H
#define _LINUX_SLUB_DEF_H

/*
 * SLUB : A Slab allocator without object queues.
 *
 * (C) 2007 SGI, Christoph Lameter
 */
#include <linux/kobject.h>

enum stat_item {
	ALLOC_FASTPATH,		/* Allocation from cpu slab */
	ALLOC_SLOWPATH,		/* Allocation by getting a new cpu slab */
	FREE_FASTPATH,		/* Free to cpu slab */
	FREE_SLOWPATH,		/* Freeing not to cpu slab */
	FREE_FROZEN,		/* Freeing to frozen slab */
	FREE_ADD_PARTIAL,	/* Freeing moves slab to partial list */
	FREE_REMOVE_PARTIAL,	/* Freeing removes last object */
	ALLOC_FROM_PARTIAL,	/* Cpu slab acquired from node partial list */
	ALLOC_SLAB,		/* Cpu slab acquired from page allocator */
	ALLOC_REFILL,		/* Refill cpu slab from slab freelist */
	ALLOC_NODE_MISMATCH,	/* Switching cpu slab */
	FREE_SLAB,		/* Slab freed to the page allocator */
	CPUSLAB_FLUSH,		/* Abandoning of the cpu slab */
	DEACTIVATE_FULL,	/* Cpu slab was full when deactivated */
	DEACTIVATE_EMPTY,	/* Cpu slab was empty when deactivated */
	DEACTIVATE_TO_HEAD,	/* Cpu slab was moved to the head of partials */
	DEACTIVATE_TO_TAIL,	/* Cpu slab was moved to the tail of partials */
	DEACTIVATE_REMOTE_FREES,/* Slab contained remotely freed objects */
	DEACTIVATE_BYPASS,	/* Implicit deactivation */
	ORDER_FALLBACK,		/* Number of times fallback was necessary */
	CMPXCHG_DOUBLE_CPU_FAIL,/* Failure of this_cpu_cmpxchg_double */
	CMPXCHG_DOUBLE_FAIL,	/* Number of times that cmpxchg double did not match */
	CPU_PARTIAL_ALLOC,	/* Used cpu partial on alloc */
	CPU_PARTIAL_FREE,	/* Refill cpu partial on free */
	CPU_PARTIAL_NODE,	/* Refill cpu partial from node partial */
	CPU_PARTIAL_DRAIN,	/* Drain cpu partial to node partial */
	NR_SLUB_STAT_ITEMS };

/* JYW: 本地内存缓存池的描述, 每个CPU一个 */
struct kmem_cache_cpu {
    /* JYW: 指向下一个可用的object */
	void **freelist;	/* Pointer to next available object */
    /* JYW: 用于同步作用 */
	unsigned long tid;	/* Globally unique transaction id */
    /* JYW: slub内存的page指针 */
	struct page *page;	/* The slab from which we are allocating */
    /* JYW: 本地slab partial链表。主要是一些部分使用object的slab。 */
	struct page *partial;	/* Partially allocated frozen slabs */
#ifdef CONFIG_SLUB_STATS
	unsigned stat[NR_SLUB_STAT_ITEMS];
#endif
};

/*
 * Word size structure that can be atomically updated or read and that
 * contains both the order and the number of objects that a slab of the
 * given order would contain.
 */
struct kmem_cache_order_objects {
	unsigned long x;
};

/*
 * Slab cache management.
 */
/* JYW: 表示一个slab描述符 */
struct kmem_cache {
    /* JYW: 本地内存缓存池，当分配内存的时候优先从本地cpu分配内存以保证cache的命中率 */
    struct kmem_cache_cpu __percpu *cpu_slab;
	/* Used for retriving partial slabs etc */
    /* JYW: object分配掩码 */
	unsigned long flags;
    /*
     * JYW: 限制struct kmem_cache_node中的partial链表slab的数量
     *      如果大于这个mini_partial的值，那么多余的slab就会被释放
     */
	unsigned long min_partial;
    /* JYW: object_size对齐后的size大小 */
	int size;		/* The size of an object including meta data */
    /* JYW: 实际的object size，就是创建kmem_cache时候传递进来的参数 */
    int object_size;	/* The size of an object without meta data */
    /* JYW: 存储下个object地址数据相对于这个object首地址的偏移 */
	int offset;		/* Free pointer offset. */
    /*
     * JYW: per cpu partial中所有slab的free object的数量的最大值
     *  超过这个值就会将所有的slab转移到kmem_cache_node的partial链表
     */
	int cpu_partial;	/* Number of per cpu partial objects to keep around */
    /* JYW: 低16位代表一个slab中所有object的数量（oo &((1 << 16) - 1)），高16位代表一个slab管理的page数量（(2^(oo  16)) pages） */
	struct kmem_cache_order_objects oo;

	/* Allocation and freeing of slabs */
	struct kmem_cache_order_objects max;
    /* JYW: 当按照oo大小分配内存的时候出现内存不足就会考虑min大小方式分配。min只需要可以容纳一个object即可 */
	struct kmem_cache_order_objects min;
    /* JYW: 从伙伴系统分配内存掩码 */
	gfp_t allocflags;	/* gfp flags to use on each alloc */
	int refcount;		/* Refcount for slab cache destroy */
	void (*ctor)(void *);
    /* JYW: object_size按照word对齐之后的大小 */
	int inuse;		/* Offset to metadata */
    /* JYW: 字节对齐大小 */
	int align;		/* Alignment */
	int reserved;		/* Reserved bytes at the end of slabs */
    /* JYW: sysfs文件系统显示使用 */
	const char *name;	/* Name (only for display!) */
    /* JYW: 系统有一个slab_caches链表，所有的slab都会挂入此链表 */
	struct list_head list;	/* List of slab caches */
#ifdef CONFIG_SYSFS
	struct kobject kobj;	/* For sysfs */
#endif
#ifdef CONFIG_MEMCG_KMEM
	struct memcg_cache_params memcg_params;
	int max_attr_size; /* for propagation, maximum size of a stored attr */
#ifdef CONFIG_SYSFS
	struct kset *memcg_kset;
#endif
#endif

#ifdef CONFIG_NUMA
	/*
	 * Defragmentation by allocating from a remote node.
	 */
	int remote_node_defrag_ratio;
#endif
    /* JYW: slab节点。在NUMA系统中，每个node都有一个struct kmem_cache_node数据结构 */
	struct kmem_cache_node *node[MAX_NUMNODES];
};

#ifdef CONFIG_SYSFS
#define SLAB_SUPPORTS_SYSFS
void sysfs_slab_remove(struct kmem_cache *);
#else
static inline void sysfs_slab_remove(struct kmem_cache *s)
{
}
#endif


/**
 * virt_to_obj - returns address of the beginning of object.
 * @s: object's kmem_cache
 * @slab_page: address of slab page
 * @x: address within object memory range
 *
 * Returns address of the beginning of object
 */
static inline void *virt_to_obj(struct kmem_cache *s,
				const void *slab_page,
				const void *x)
{
	return (void *)x - ((x - slab_page) % s->size);
}

void object_err(struct kmem_cache *s, struct page *page,
		u8 *object, char *reason);

#endif /* _LINUX_SLUB_DEF_H */
