
#ifndef __LINUX_FDTABLE_H
#define __LINUX_FDTABLE_H

#include <linux/posix_types.h>
#include <linux/compiler.h>
#include <linux/spinlock.h>
#include <linux/rcupdate.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/fs.h>

#include <linux/atomic.h>

#define NR_OPEN_DEFAULT BITS_PER_LONG

struct fdt_user {
	pid_t remover;
	pid_t installer;
	unsigned long remove_ts;
	unsigned long install_ts;
};

struct fdtable {
	unsigned int max_fds;
	struct file __rcu **fd;      
	struct fdt_user *user; 
	unsigned long *close_on_exec;
	unsigned long *open_fds;
	struct rcu_head rcu;
};

static inline bool close_on_exec(int fd, const struct fdtable *fdt)
{
	return test_bit(fd, fdt->close_on_exec);
}

static inline bool fd_is_open(int fd, const struct fdtable *fdt)
{
	return test_bit(fd, fdt->open_fds);
}

struct files_struct {
	atomic_t count;
	struct fdtable __rcu *fdt;
	struct fdtable fdtab;
  /*
   * written part on a separate cache line in SMP
   */
	spinlock_t file_lock ____cacheline_aligned_in_smp;
	int next_fd;
	unsigned long close_on_exec_init[1];
	unsigned long open_fds_init[1];
	struct file __rcu * fd_array[NR_OPEN_DEFAULT];
	struct fdt_user user_array[NR_OPEN_DEFAULT];
};

struct file_operations;
struct vfsmount;
struct dentry;

#define rcu_dereference_check_fdtable(files, fdtfd) \
	rcu_dereference_check((fdtfd), lockdep_is_held(&(files)->file_lock))

#define files_fdtable(files) \
	rcu_dereference_check_fdtable((files), (files)->fdt)

static inline struct file *__fcheck_files(struct files_struct *files, unsigned int fd)
{
	struct fdtable *fdt = rcu_dereference_raw(files->fdt);

	if (fd < fdt->max_fds)
		return rcu_dereference_raw(fdt->fd[fd]);
	return NULL;
}

static inline struct file *fcheck_files(struct files_struct *files, unsigned int fd)
{
	rcu_lockdep_assert(rcu_read_lock_held() ||
			   lockdep_is_held(&files->file_lock),
			   "suspicious rcu_dereference_check() usage");
	return __fcheck_files(files, fd);
}

#define fcheck(fd)	fcheck_files(current->files, fd)

struct task_struct;

struct files_struct *get_files_struct(struct task_struct *);
void put_files_struct(struct files_struct *fs);
void reset_files_struct(struct files_struct *);
int unshare_files(struct files_struct **);
struct files_struct *dup_fd(struct files_struct *, int *);
void do_close_on_exec(struct files_struct *);
int iterate_fd(struct files_struct *, unsigned,
		int (*)(const void *, struct file *, unsigned),
		const void *);

extern int __alloc_fd(struct files_struct *files,
		      unsigned start, unsigned end, unsigned flags);
extern void __fd_install(struct files_struct *files,
		      unsigned int fd, struct file *file);
extern int __close_fd(struct files_struct *files,
		      unsigned int fd);

extern struct kmem_cache *files_cachep;

#define FD_OVER_CHECK

#ifdef FD_OVER_CHECK
extern void fd_show_open_files(pid_t pid, struct files_struct *files, struct fdtable *fdt);
#endif

#endif 
