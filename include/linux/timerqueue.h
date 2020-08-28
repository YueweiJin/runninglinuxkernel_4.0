#ifndef _LINUX_TIMERQUEUE_H
#define _LINUX_TIMERQUEUE_H

#include <linux/rbtree.h>
#include <linux/ktime.h>


struct timerqueue_node {
    /* JYW: 红黑树的节点 */
	struct rb_node node;
    /* JYW: 该节点代表队hrtimer的到期时间，与hrtimer结构中的_softexpires稍有不同 */
	ktime_t expires;
};

struct timerqueue_head {
    /* JYW: 红黑树的根 */
	struct rb_root head;
    /* JYW: 增加了一个next字段，用于保存树中最先到期的定时器节点 */
	struct timerqueue_node *next;
};


extern void timerqueue_add(struct timerqueue_head *head,
				struct timerqueue_node *node);
extern void timerqueue_del(struct timerqueue_head *head,
				struct timerqueue_node *node);
extern struct timerqueue_node *timerqueue_iterate_next(
						struct timerqueue_node *node);

/**
 * timerqueue_getnext - Returns the timer with the earliest expiration time
 *
 * @head: head of timerqueue
 *
 * Returns a pointer to the timer node that has the
 * earliest expiration time.
 */
static inline
struct timerqueue_node *timerqueue_getnext(struct timerqueue_head *head)
{
	return head->next;
}

static inline void timerqueue_init(struct timerqueue_node *node)
{
	RB_CLEAR_NODE(&node->node);
}

static inline void timerqueue_init_head(struct timerqueue_head *head)
{
	head->head = RB_ROOT;
	head->next = NULL;
}
#endif /* _LINUX_TIMERQUEUE_H */
