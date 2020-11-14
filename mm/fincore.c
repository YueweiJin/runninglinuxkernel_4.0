#include <linux/syscalls.h>
#include <linux/pagemap.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/mm.h>

static long do_fincore(struct address_space *mapping, pgoff_t pgstart,
		       unsigned long nr_pages, unsigned char *vec)
{
	pgoff_t pgend = pgstart + nr_pages;
	struct radix_tree_iter iter;
	void **slot;
	long nr = 0;

	rcu_read_lock();
restart:
	radix_tree_for_each_slot(slot, &mapping->page_tree, &iter, pgstart) {
		unsigned char present;
		struct page *page;

		/* Handle holes */
		if (iter.index != pgstart + nr) {
			if (iter.index < pgend)
				nr_pages = iter.index - pgstart;
			break;
		}
repeat:
		page = radix_tree_deref_slot(slot);
		if (unlikely(!page))
			continue;

		if (radix_tree_exception(page)) {
			if (radix_tree_deref_retry(page)) {
				/*
				 * Transient condition which can only trigger
				 * when entry at index 0 moves out of or back
				 * to root: none yet gotten, safe to restart.
				 */
				WARN_ON(iter.index);
				goto restart;
			}
			present = 0;
		} else {
			if (!page_cache_get_speculative(page))
				goto repeat;

			/* Has the page moved? */
			if (unlikely(page != *slot)) {
				page_cache_release(page);
				goto repeat;
			}

			present = PageUptodate(page);
			page_cache_release(page);
		}
		vec[nr] = present;

		if (++nr == nr_pages)
			break;
	}
	rcu_read_unlock();

	if (nr < nr_pages)
		memset(vec + nr, 0, nr_pages - nr);

	return nr_pages;
}

/*
 * The fincore(2) system call.
 *
 * fincore() returns the memory residency status of the given file's
 * pages, in the range [start, start  len].
 * The status is returned in a vector of bytes.  The least significant
 * bit of each byte is 1 if the referenced page is in memory, otherwise
 * it is zero.
 *
 * Because the status of a page can change after fincore() checks it
 * but before it returns to the application, the returned vector may
 * contain stale information.
 *
 * return values:
 *  zero    - success
 *  -EBADF  - fd isn't a valid open file descriptor
 *  -EFAULT - vec points to an illegal address
 *  -EINVAL - start is not a multiple of PAGE_CACHE_SIZE
 */
SYSCALL_DEFINE4(fincore, unsigned int, fd, loff_t, start, loff_t, len,
		unsigned char __user *, vec)
{
	unsigned long nr_pages;
	pgoff_t pgstart;
	struct fd f;
	long ret;

	if (start & ~PAGE_CACHE_MASK)
		return -EINVAL;

	f = fdget(fd);
	if (!f.file)
		return -EBADF;

	pgstart = start >> PAGE_CACHE_SHIFT;
	nr_pages = DIV_ROUND_UP(len, PAGE_CACHE_SIZE);

	while (nr_pages) {
		unsigned char tmp[64];

		ret = do_fincore(f.file->f_mapping, pgstart,
				 min(nr_pages, sizeof(tmp)), tmp);
		if (ret <= 0)
			break;

		if (copy_to_user(vec, tmp, ret)) {
			ret = -EFAULT;
			break;
		}

		nr_pages -= ret;
		pgstart = ret;
		vec = ret;
		ret = 0;
	}

	fdput(f);

	return ret;
}

