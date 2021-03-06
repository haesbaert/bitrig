//===-- linux_syscall_hooks.h ---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of public sanitizer interface.
//
// System call handlers.
//
// Interface methods declared in this header implement pre- and post- syscall
// actions for the active sanitizer.
// Usage:
//   __sanitizer_syscall_pre_getfoo(...args...);
//   int res = syscall(__NR_getfoo, ...args...);
//   __sanitizer_syscall_post_getfoo(res, ...args...);
//===----------------------------------------------------------------------===//
#ifndef SANITIZER_LINUX_SYSCALL_HOOKS_H
#define SANITIZER_LINUX_SYSCALL_HOOKS_H

#ifdef __cplusplus
extern "C" {
#endif

void __sanitizer_syscall_pre_rt_sigpending(void *p, size_t s);
void __sanitizer_syscall_pre_getdents(int fd, void *dirp, int count);
void __sanitizer_syscall_pre_getdents64(int fd, void *dirp, int count);
void __sanitizer_syscall_pre_recvmsg(int sockfd, void *msg, int flags);
void __sanitizer_syscall_pre_wait4(int pid, int *status, int options, void *r);
void __sanitizer_syscall_pre_waitpid(int pid, int *status, int options);

void __sanitizer_syscall_post_rt_sigpending(long res, void *p, size_t s);
void __sanitizer_syscall_post_getdents(long res, int fd, void *dirp, int count);
void __sanitizer_syscall_post_getdents64(long res, int fd, void *dirp,
                                         int count);
void __sanitizer_syscall_post_recvmsg(long res, int sockfd, void *msg,
                                      int flags);
void __sanitizer_syscall_post_wait4(long res, int pid, int *status, int options,
                                    void *r);
void __sanitizer_syscall_post_waitpid(long res, int pid, int *status,
                                      int options);

// And now a few syscalls we don't handle yet.

#define __sanitizer_syscall_pre_accept(...)
#define __sanitizer_syscall_pre_accept4(...)
#define __sanitizer_syscall_pre_access(...)
#define __sanitizer_syscall_pre_acct(...)
#define __sanitizer_syscall_pre_add_key(...)
#define __sanitizer_syscall_pre_adjtimex(...)
#define __sanitizer_syscall_pre_afs_syscall(...)
#define __sanitizer_syscall_pre_alarm(...)
#define __sanitizer_syscall_pre_arch_prctl(...)
#define __sanitizer_syscall_pre_bdflush(...)
#define __sanitizer_syscall_pre_bind(...)
#define __sanitizer_syscall_pre_break(...)
#define __sanitizer_syscall_pre_brk(...)
#define __sanitizer_syscall_pre_capget(...)
#define __sanitizer_syscall_pre_capset(...)
#define __sanitizer_syscall_pre_chdir(...)
#define __sanitizer_syscall_pre_chmod(...)
#define __sanitizer_syscall_pre_chown(...)
#define __sanitizer_syscall_pre_chown32(...)
#define __sanitizer_syscall_pre_chroot(...)
#define __sanitizer_syscall_pre_clock_adjtime(...)
#define __sanitizer_syscall_pre_clock_getres(...)
#define __sanitizer_syscall_pre_clock_gettime(...)
#define __sanitizer_syscall_pre_clock_nanosleep(...)
#define __sanitizer_syscall_pre_clock_settime(...)
#define __sanitizer_syscall_pre_clone(...)
#define __sanitizer_syscall_pre_close(...)
#define __sanitizer_syscall_pre_connect(...)
#define __sanitizer_syscall_pre_creat(...)
#define __sanitizer_syscall_pre_create_module(...)
#define __sanitizer_syscall_pre_delete_module(...)
#define __sanitizer_syscall_pre_dup(...)
#define __sanitizer_syscall_pre_dup2(...)
#define __sanitizer_syscall_pre_dup3(...)
#define __sanitizer_syscall_pre_epoll_create(...)
#define __sanitizer_syscall_pre_epoll_create1(...)
#define __sanitizer_syscall_pre_epoll_ctl(...)
#define __sanitizer_syscall_pre_epoll_ctl_old(...)
#define __sanitizer_syscall_pre_epoll_pwait(...)
#define __sanitizer_syscall_pre_epoll_wait(...)
#define __sanitizer_syscall_pre_epoll_wait_old(...)
#define __sanitizer_syscall_pre_eventfd(...)
#define __sanitizer_syscall_pre_eventfd2(...)
#define __sanitizer_syscall_pre_execve(...)
#define __sanitizer_syscall_pre_exit(...)
#define __sanitizer_syscall_pre_exit_group(...)
#define __sanitizer_syscall_pre_faccessat(...)
#define __sanitizer_syscall_pre_fadvise64(...)
#define __sanitizer_syscall_pre_fadvise64_64(...)
#define __sanitizer_syscall_pre_fallocate(...)
#define __sanitizer_syscall_pre_fanotify_init(...)
#define __sanitizer_syscall_pre_fanotify_mark(...)
#define __sanitizer_syscall_pre_fchdir(...)
#define __sanitizer_syscall_pre_fchmod(...)
#define __sanitizer_syscall_pre_fchmodat(...)
#define __sanitizer_syscall_pre_fchown(...)
#define __sanitizer_syscall_pre_fchown32(...)
#define __sanitizer_syscall_pre_fchownat(...)
#define __sanitizer_syscall_pre_fcntl(...)
#define __sanitizer_syscall_pre_fcntl64(...)
#define __sanitizer_syscall_pre_fdatasync(...)
#define __sanitizer_syscall_pre_fgetxattr(...)
#define __sanitizer_syscall_pre_flistxattr(...)
#define __sanitizer_syscall_pre_flock(...)
#define __sanitizer_syscall_pre_fork(...)
#define __sanitizer_syscall_pre_fremovexattr(...)
#define __sanitizer_syscall_pre_fsetxattr(...)
#define __sanitizer_syscall_pre_fstat(...)
#define __sanitizer_syscall_pre_fstat64(...)
#define __sanitizer_syscall_pre_fstatat64(...)
#define __sanitizer_syscall_pre_fstatfs(...)
#define __sanitizer_syscall_pre_fstatfs64(...)
#define __sanitizer_syscall_pre_fsync(...)
#define __sanitizer_syscall_pre_ftime(...)
#define __sanitizer_syscall_pre_ftruncate(...)
#define __sanitizer_syscall_pre_ftruncate64(...)
#define __sanitizer_syscall_pre_futex(...)
#define __sanitizer_syscall_pre_futimesat(...)
#define __sanitizer_syscall_pre_getcpu(...)
#define __sanitizer_syscall_pre_getcwd(...)
#define __sanitizer_syscall_pre_getegid(...)
#define __sanitizer_syscall_pre_getegid32(...)
#define __sanitizer_syscall_pre_geteuid(...)
#define __sanitizer_syscall_pre_geteuid32(...)
#define __sanitizer_syscall_pre_getgid(...)
#define __sanitizer_syscall_pre_getgid32(...)
#define __sanitizer_syscall_pre_getgroups(...)
#define __sanitizer_syscall_pre_getgroups32(...)
#define __sanitizer_syscall_pre_getitimer(...)
#define __sanitizer_syscall_pre_get_kernel_syms(...)
#define __sanitizer_syscall_pre_get_mempolicy(...)
#define __sanitizer_syscall_pre_getpeername(...)
#define __sanitizer_syscall_pre_getpgid(...)
#define __sanitizer_syscall_pre_getpgrp(...)
#define __sanitizer_syscall_pre_getpid(...)
#define __sanitizer_syscall_pre_getpmsg(...)
#define __sanitizer_syscall_pre_getppid(...)
#define __sanitizer_syscall_pre_getpriority(...)
#define __sanitizer_syscall_pre_getresgid(...)
#define __sanitizer_syscall_pre_getresgid32(...)
#define __sanitizer_syscall_pre_getresuid(...)
#define __sanitizer_syscall_pre_getresuid32(...)
#define __sanitizer_syscall_pre_getrlimit(...)
#define __sanitizer_syscall_pre_get_robust_list(...)
#define __sanitizer_syscall_pre_getrusage(...)
#define __sanitizer_syscall_pre_getsid(...)
#define __sanitizer_syscall_pre_getsockname(...)
#define __sanitizer_syscall_pre_getsockopt(...)
#define __sanitizer_syscall_pre_get_thread_area(...)
#define __sanitizer_syscall_pre_gettid(...)
#define __sanitizer_syscall_pre_gettimeofday(...)
#define __sanitizer_syscall_pre_getuid(...)
#define __sanitizer_syscall_pre_getuid32(...)
#define __sanitizer_syscall_pre_getxattr(...)
#define __sanitizer_syscall_pre_gtty(...)
#define __sanitizer_syscall_pre_idle(...)
#define __sanitizer_syscall_pre_init_module(...)
#define __sanitizer_syscall_pre_inotify_add_watch(...)
#define __sanitizer_syscall_pre_inotify_init(...)
#define __sanitizer_syscall_pre_inotify_init1(...)
#define __sanitizer_syscall_pre_inotify_rm_watch(...)
#define __sanitizer_syscall_pre_io_cancel(...)
#define __sanitizer_syscall_pre_ioctl(...)
#define __sanitizer_syscall_pre_io_destroy(...)
#define __sanitizer_syscall_pre_io_getevents(...)
#define __sanitizer_syscall_pre_ioperm(...)
#define __sanitizer_syscall_pre_iopl(...)
#define __sanitizer_syscall_pre_ioprio_get(...)
#define __sanitizer_syscall_pre_ioprio_set(...)
#define __sanitizer_syscall_pre_io_setup(...)
#define __sanitizer_syscall_pre_io_submit(...)
#define __sanitizer_syscall_pre_ipc(...)
#define __sanitizer_syscall_pre_kexec_load(...)
#define __sanitizer_syscall_pre_keyctl(...)
#define __sanitizer_syscall_pre_kill(...)
#define __sanitizer_syscall_pre_lchown(...)
#define __sanitizer_syscall_pre_lchown32(...)
#define __sanitizer_syscall_pre_lgetxattr(...)
#define __sanitizer_syscall_pre_link(...)
#define __sanitizer_syscall_pre_linkat(...)
#define __sanitizer_syscall_pre_listen(...)
#define __sanitizer_syscall_pre_listxattr(...)
#define __sanitizer_syscall_pre_llistxattr(...)
#define __sanitizer_syscall_pre__llseek(...)
#define __sanitizer_syscall_pre_lock(...)
#define __sanitizer_syscall_pre_lookup_dcookie(...)
#define __sanitizer_syscall_pre_lremovexattr(...)
#define __sanitizer_syscall_pre_lseek(...)
#define __sanitizer_syscall_pre_lsetxattr(...)
#define __sanitizer_syscall_pre_lstat(...)
#define __sanitizer_syscall_pre_lstat64(...)
#define __sanitizer_syscall_pre_madvise(...)
#define __sanitizer_syscall_pre_madvise1(...)
#define __sanitizer_syscall_pre_mbind(...)
#define __sanitizer_syscall_pre_migrate_pages(...)
#define __sanitizer_syscall_pre_mincore(...)
#define __sanitizer_syscall_pre_mkdir(...)
#define __sanitizer_syscall_pre_mkdirat(...)
#define __sanitizer_syscall_pre_mknod(...)
#define __sanitizer_syscall_pre_mknodat(...)
#define __sanitizer_syscall_pre_mlock(...)
#define __sanitizer_syscall_pre_mlockall(...)
#define __sanitizer_syscall_pre_mmap(...)
#define __sanitizer_syscall_pre_mmap2(...)
#define __sanitizer_syscall_pre_modify_ldt(...)
#define __sanitizer_syscall_pre_mount(...)
#define __sanitizer_syscall_pre_move_pages(...)
#define __sanitizer_syscall_pre_mprotect(...)
#define __sanitizer_syscall_pre_mpx(...)
#define __sanitizer_syscall_pre_mq_getsetattr(...)
#define __sanitizer_syscall_pre_mq_notify(...)
#define __sanitizer_syscall_pre_mq_open(...)
#define __sanitizer_syscall_pre_mq_timedreceive(...)
#define __sanitizer_syscall_pre_mq_timedsend(...)
#define __sanitizer_syscall_pre_mq_unlink(...)
#define __sanitizer_syscall_pre_mremap(...)
#define __sanitizer_syscall_pre_msgctl(...)
#define __sanitizer_syscall_pre_msgget(...)
#define __sanitizer_syscall_pre_msgrcv(...)
#define __sanitizer_syscall_pre_msgsnd(...)
#define __sanitizer_syscall_pre_msync(...)
#define __sanitizer_syscall_pre_munlock(...)
#define __sanitizer_syscall_pre_munlockall(...)
#define __sanitizer_syscall_pre_munmap(...)
#define __sanitizer_syscall_pre_name_to_handle_at(...)
#define __sanitizer_syscall_pre_nanosleep(...)
#define __sanitizer_syscall_pre_newfstatat(...)
#define __sanitizer_syscall_pre__newselect(...)
#define __sanitizer_syscall_pre_nfsservctl(...)
#define __sanitizer_syscall_pre_nice(...)
#define __sanitizer_syscall_pre_oldfstat(...)
#define __sanitizer_syscall_pre_oldlstat(...)
#define __sanitizer_syscall_pre_oldolduname(...)
#define __sanitizer_syscall_pre_oldstat(...)
#define __sanitizer_syscall_pre_olduname(...)
#define __sanitizer_syscall_pre_open(...)
#define __sanitizer_syscall_pre_openat(...)
#define __sanitizer_syscall_pre_open_by_handle_at(...)
#define __sanitizer_syscall_pre_pause(...)
#define __sanitizer_syscall_pre_perf_event_open(...)
#define __sanitizer_syscall_pre_personality(...)
#define __sanitizer_syscall_pre_pipe(...)
#define __sanitizer_syscall_pre_pipe2(...)
#define __sanitizer_syscall_pre_pivot_root(...)
#define __sanitizer_syscall_pre_poll(...)
#define __sanitizer_syscall_pre_ppoll(...)
#define __sanitizer_syscall_pre_prctl(...)
#define __sanitizer_syscall_pre_pread64(...)
#define __sanitizer_syscall_pre_preadv(...)
#define __sanitizer_syscall_pre_prlimit64(...)
#define __sanitizer_syscall_pre_process_vm_readv(...)
#define __sanitizer_syscall_pre_process_vm_writev(...)
#define __sanitizer_syscall_pre_prof(...)
#define __sanitizer_syscall_pre_profil(...)
#define __sanitizer_syscall_pre_pselect6(...)
#define __sanitizer_syscall_pre_ptrace(...)
#define __sanitizer_syscall_pre_putpmsg(...)
#define __sanitizer_syscall_pre_pwrite64(...)
#define __sanitizer_syscall_pre_pwritev(...)
#define __sanitizer_syscall_pre_query_module(...)
#define __sanitizer_syscall_pre_quotactl(...)
#define __sanitizer_syscall_pre_read(...)
#define __sanitizer_syscall_pre_readahead(...)
#define __sanitizer_syscall_pre_readdir(...)
#define __sanitizer_syscall_pre_readlink(...)
#define __sanitizer_syscall_pre_readlinkat(...)
#define __sanitizer_syscall_pre_readv(...)
#define __sanitizer_syscall_pre_reboot(...)
#define __sanitizer_syscall_pre_recvfrom(...)
#define __sanitizer_syscall_pre_recvmmsg(...)
#define __sanitizer_syscall_pre_remap_file_pages(...)
#define __sanitizer_syscall_pre_removexattr(...)
#define __sanitizer_syscall_pre_rename(...)
#define __sanitizer_syscall_pre_renameat(...)
#define __sanitizer_syscall_pre_request_key(...)
#define __sanitizer_syscall_pre_restart_syscall(...)
#define __sanitizer_syscall_pre_rmdir(...)
#define __sanitizer_syscall_pre_rt_sigaction(...)
#define __sanitizer_syscall_pre_rt_sigprocmask(...)
#define __sanitizer_syscall_pre_rt_sigqueueinfo(...)
#define __sanitizer_syscall_pre_rt_sigreturn(...)
#define __sanitizer_syscall_pre_rt_sigsuspend(...)
#define __sanitizer_syscall_pre_rt_sigtimedwait(...)
#define __sanitizer_syscall_pre_rt_tgsigqueueinfo(...)
#define __sanitizer_syscall_pre_sched_getaffinity(...)
#define __sanitizer_syscall_pre_sched_getparam(...)
#define __sanitizer_syscall_pre_sched_get_priority_max(...)
#define __sanitizer_syscall_pre_sched_get_priority_min(...)
#define __sanitizer_syscall_pre_sched_getscheduler(...)
#define __sanitizer_syscall_pre_sched_rr_get_interval(...)
#define __sanitizer_syscall_pre_sched_setaffinity(...)
#define __sanitizer_syscall_pre_sched_setparam(...)
#define __sanitizer_syscall_pre_sched_setscheduler(...)
#define __sanitizer_syscall_pre_sched_yield(...)
#define __sanitizer_syscall_pre_security(...)
#define __sanitizer_syscall_pre_select(...)
#define __sanitizer_syscall_pre_semctl(...)
#define __sanitizer_syscall_pre_semget(...)
#define __sanitizer_syscall_pre_semop(...)
#define __sanitizer_syscall_pre_semtimedop(...)
#define __sanitizer_syscall_pre_sendfile(...)
#define __sanitizer_syscall_pre_sendfile64(...)
#define __sanitizer_syscall_pre_sendmmsg(...)
#define __sanitizer_syscall_pre_sendmsg(...)
#define __sanitizer_syscall_pre_sendto(...)
#define __sanitizer_syscall_pre_setdomainname(...)
#define __sanitizer_syscall_pre_setfsgid(...)
#define __sanitizer_syscall_pre_setfsgid32(...)
#define __sanitizer_syscall_pre_setfsuid(...)
#define __sanitizer_syscall_pre_setfsuid32(...)
#define __sanitizer_syscall_pre_setgid(...)
#define __sanitizer_syscall_pre_setgid32(...)
#define __sanitizer_syscall_pre_setgroups(...)
#define __sanitizer_syscall_pre_setgroups32(...)
#define __sanitizer_syscall_pre_sethostname(...)
#define __sanitizer_syscall_pre_setitimer(...)
#define __sanitizer_syscall_pre_set_mempolicy(...)
#define __sanitizer_syscall_pre_setns(...)
#define __sanitizer_syscall_pre_setpgid(...)
#define __sanitizer_syscall_pre_setpriority(...)
#define __sanitizer_syscall_pre_setregid(...)
#define __sanitizer_syscall_pre_setregid32(...)
#define __sanitizer_syscall_pre_setresgid(...)
#define __sanitizer_syscall_pre_setresgid32(...)
#define __sanitizer_syscall_pre_setresuid(...)
#define __sanitizer_syscall_pre_setresuid32(...)
#define __sanitizer_syscall_pre_setreuid(...)
#define __sanitizer_syscall_pre_setreuid32(...)
#define __sanitizer_syscall_pre_setrlimit(...)
#define __sanitizer_syscall_pre_set_robust_list(...)
#define __sanitizer_syscall_pre_setsid(...)
#define __sanitizer_syscall_pre_setsockopt(...)
#define __sanitizer_syscall_pre_set_thread_area(...)
#define __sanitizer_syscall_pre_set_tid_address(...)
#define __sanitizer_syscall_pre_settimeofday(...)
#define __sanitizer_syscall_pre_setuid(...)
#define __sanitizer_syscall_pre_setuid32(...)
#define __sanitizer_syscall_pre_setxattr(...)
#define __sanitizer_syscall_pre_sgetmask(...)
#define __sanitizer_syscall_pre_shmat(...)
#define __sanitizer_syscall_pre_shmctl(...)
#define __sanitizer_syscall_pre_shmdt(...)
#define __sanitizer_syscall_pre_shmget(...)
#define __sanitizer_syscall_pre_shutdown(...)
#define __sanitizer_syscall_pre_sigaction(...)
#define __sanitizer_syscall_pre_sigaltstack(...)
#define __sanitizer_syscall_pre_signal(...)
#define __sanitizer_syscall_pre_signalfd(...)
#define __sanitizer_syscall_pre_signalfd4(...)
#define __sanitizer_syscall_pre_sigpending(...)
#define __sanitizer_syscall_pre_sigprocmask(...)
#define __sanitizer_syscall_pre_sigreturn(...)
#define __sanitizer_syscall_pre_sigsuspend(...)
#define __sanitizer_syscall_pre_socket(...)
#define __sanitizer_syscall_pre_socketcall(...)
#define __sanitizer_syscall_pre_socketpair(...)
#define __sanitizer_syscall_pre_splice(...)
#define __sanitizer_syscall_pre_ssetmask(...)
#define __sanitizer_syscall_pre_stat(...)
#define __sanitizer_syscall_pre_stat64(...)
#define __sanitizer_syscall_pre_statfs(...)
#define __sanitizer_syscall_pre_statfs64(...)
#define __sanitizer_syscall_pre_stime(...)
#define __sanitizer_syscall_pre_stty(...)
#define __sanitizer_syscall_pre_swapoff(...)
#define __sanitizer_syscall_pre_swapon(...)
#define __sanitizer_syscall_pre_symlink(...)
#define __sanitizer_syscall_pre_symlinkat(...)
#define __sanitizer_syscall_pre_sync(...)
#define __sanitizer_syscall_pre_sync_file_range(...)
#define __sanitizer_syscall_pre_syncfs(...)
#define __sanitizer_syscall_pre__sysctl(...)
#define __sanitizer_syscall_pre_sysfs(...)
#define __sanitizer_syscall_pre_sysinfo(...)
#define __sanitizer_syscall_pre_syslog(...)
#define __sanitizer_syscall_pre_tee(...)
#define __sanitizer_syscall_pre_tgkill(...)
#define __sanitizer_syscall_pre_time(...)
#define __sanitizer_syscall_pre_timer_create(...)
#define __sanitizer_syscall_pre_timer_delete(...)
#define __sanitizer_syscall_pre_timerfd_create(...)
#define __sanitizer_syscall_pre_timerfd_gettime(...)
#define __sanitizer_syscall_pre_timerfd_settime(...)
#define __sanitizer_syscall_pre_timer_getoverrun(...)
#define __sanitizer_syscall_pre_timer_gettime(...)
#define __sanitizer_syscall_pre_timer_settime(...)
#define __sanitizer_syscall_pre_times(...)
#define __sanitizer_syscall_pre_tkill(...)
#define __sanitizer_syscall_pre_truncate(...)
#define __sanitizer_syscall_pre_truncate64(...)
#define __sanitizer_syscall_pre_tuxcall(...)
#define __sanitizer_syscall_pre_ugetrlimit(...)
#define __sanitizer_syscall_pre_ulimit(...)
#define __sanitizer_syscall_pre_umask(...)
#define __sanitizer_syscall_pre_umount(...)
#define __sanitizer_syscall_pre_umount2(...)
#define __sanitizer_syscall_pre_uname(...)
#define __sanitizer_syscall_pre_unlink(...)
#define __sanitizer_syscall_pre_unlinkat(...)
#define __sanitizer_syscall_pre_unshare(...)
#define __sanitizer_syscall_pre_uselib(...)
#define __sanitizer_syscall_pre_ustat(...)
#define __sanitizer_syscall_pre_utime(...)
#define __sanitizer_syscall_pre_utimensat(...)
#define __sanitizer_syscall_pre_utimes(...)
#define __sanitizer_syscall_pre_vfork(...)
#define __sanitizer_syscall_pre_vhangup(...)
#define __sanitizer_syscall_pre_vm86(...)
#define __sanitizer_syscall_pre_vm86old(...)
#define __sanitizer_syscall_pre_vmsplice(...)
#define __sanitizer_syscall_pre_vserver(...)
#define __sanitizer_syscall_pre_waitid(...)
#define __sanitizer_syscall_pre_write(...)
#define __sanitizer_syscall_pre_writev(...)

#define __sanitizer_syscall_post_accept4(res, ...)
#define __sanitizer_syscall_post_accept(res, ...)
#define __sanitizer_syscall_post_access(res, ...)
#define __sanitizer_syscall_post_acct(res, ...)
#define __sanitizer_syscall_post_add_key(res, ...)
#define __sanitizer_syscall_post_adjtimex(res, ...)
#define __sanitizer_syscall_post_afs_syscall(res, ...)
#define __sanitizer_syscall_post_alarm(res, ...)
#define __sanitizer_syscall_post_arch_prctl(res, ...)
#define __sanitizer_syscall_post_bdflush(res, ...)
#define __sanitizer_syscall_post_bind(res, ...)
#define __sanitizer_syscall_post_break(res, ...)
#define __sanitizer_syscall_post_brk(res, ...)
#define __sanitizer_syscall_post_capget(res, ...)
#define __sanitizer_syscall_post_capset(res, ...)
#define __sanitizer_syscall_post_chdir(res, ...)
#define __sanitizer_syscall_post_chmod(res, ...)
#define __sanitizer_syscall_post_chown32(res, ...)
#define __sanitizer_syscall_post_chown(res, ...)
#define __sanitizer_syscall_post_chroot(res, ...)
#define __sanitizer_syscall_post_clock_adjtime(res, ...)
#define __sanitizer_syscall_post_clock_getres(res, ...)
#define __sanitizer_syscall_post_clock_gettime(res, ...)
#define __sanitizer_syscall_post_clock_nanosleep(res, ...)
#define __sanitizer_syscall_post_clock_settime(res, ...)
#define __sanitizer_syscall_post_clone(res, ...)
#define __sanitizer_syscall_post_close(res, ...)
#define __sanitizer_syscall_post_connect(res, ...)
#define __sanitizer_syscall_post_create_module(res, ...)
#define __sanitizer_syscall_post_creat(res, ...)
#define __sanitizer_syscall_post_delete_module(res, ...)
#define __sanitizer_syscall_post_dup2(res, ...)
#define __sanitizer_syscall_post_dup3(res, ...)
#define __sanitizer_syscall_post_dup(res, ...)
#define __sanitizer_syscall_post_epoll_create1(res, ...)
#define __sanitizer_syscall_post_epoll_create(res, ...)
#define __sanitizer_syscall_post_epoll_ctl_old(res, ...)
#define __sanitizer_syscall_post_epoll_ctl(res, ...)
#define __sanitizer_syscall_post_epoll_pwait(res, ...)
#define __sanitizer_syscall_post_epoll_wait_old(res, ...)
#define __sanitizer_syscall_post_epoll_wait(res, ...)
#define __sanitizer_syscall_post_eventfd2(res, ...)
#define __sanitizer_syscall_post_eventfd(res, ...)
#define __sanitizer_syscall_post_execve(res, ...)
#define __sanitizer_syscall_post_exit_group(res, ...)
#define __sanitizer_syscall_post_exit(res, ...)
#define __sanitizer_syscall_post_faccessat(res, ...)
#define __sanitizer_syscall_post_fadvise64_64(res, ...)
#define __sanitizer_syscall_post_fadvise64(res, ...)
#define __sanitizer_syscall_post_fallocate(res, ...)
#define __sanitizer_syscall_post_fanotify_init(res, ...)
#define __sanitizer_syscall_post_fanotify_mark(res, ...)
#define __sanitizer_syscall_post_fchdir(res, ...)
#define __sanitizer_syscall_post_fchmodat(res, ...)
#define __sanitizer_syscall_post_fchmod(res, ...)
#define __sanitizer_syscall_post_fchown32(res, ...)
#define __sanitizer_syscall_post_fchownat(res, ...)
#define __sanitizer_syscall_post_fchown(res, ...)
#define __sanitizer_syscall_post_fcntl64(res, ...)
#define __sanitizer_syscall_post_fcntl(res, ...)
#define __sanitizer_syscall_post_fdatasync(res, ...)
#define __sanitizer_syscall_post_fgetxattr(res, ...)
#define __sanitizer_syscall_post_flistxattr(res, ...)
#define __sanitizer_syscall_post_flock(res, ...)
#define __sanitizer_syscall_post_fork(res, ...)
#define __sanitizer_syscall_post_fremovexattr(res, ...)
#define __sanitizer_syscall_post_fsetxattr(res, ...)
#define __sanitizer_syscall_post_fstat64(res, ...)
#define __sanitizer_syscall_post_fstatat64(res, ...)
#define __sanitizer_syscall_post_fstatfs64(res, ...)
#define __sanitizer_syscall_post_fstatfs(res, ...)
#define __sanitizer_syscall_post_fstat(res, ...)
#define __sanitizer_syscall_post_fsync(res, ...)
#define __sanitizer_syscall_post_ftime(res, ...)
#define __sanitizer_syscall_post_ftruncate64(res, ...)
#define __sanitizer_syscall_post_ftruncate(res, ...)
#define __sanitizer_syscall_post_futex(res, ...)
#define __sanitizer_syscall_post_futimesat(res, ...)
#define __sanitizer_syscall_post_getcpu(res, ...)
#define __sanitizer_syscall_post_getcwd(res, ...)
#define __sanitizer_syscall_post_getegid32(res, ...)
#define __sanitizer_syscall_post_getegid(res, ...)
#define __sanitizer_syscall_post_geteuid32(res, ...)
#define __sanitizer_syscall_post_geteuid(res, ...)
#define __sanitizer_syscall_post_getgid32(res, ...)
#define __sanitizer_syscall_post_getgid(res, ...)
#define __sanitizer_syscall_post_getgroups32(res, ...)
#define __sanitizer_syscall_post_getgroups(res, ...)
#define __sanitizer_syscall_post_getitimer(res, ...)
#define __sanitizer_syscall_post_get_kernel_syms(res, ...)
#define __sanitizer_syscall_post_get_mempolicy(res, ...)
#define __sanitizer_syscall_post_getpeername(res, ...)
#define __sanitizer_syscall_post_getpgid(res, ...)
#define __sanitizer_syscall_post_getpgrp(res, ...)
#define __sanitizer_syscall_post_getpid(res, ...)
#define __sanitizer_syscall_post_getpmsg(res, ...)
#define __sanitizer_syscall_post_getppid(res, ...)
#define __sanitizer_syscall_post_getpriority(res, ...)
#define __sanitizer_syscall_post_getresgid32(res, ...)
#define __sanitizer_syscall_post_getresgid(res, ...)
#define __sanitizer_syscall_post_getresuid32(res, ...)
#define __sanitizer_syscall_post_getresuid(res, ...)
#define __sanitizer_syscall_post_getrlimit(res, ...)
#define __sanitizer_syscall_post_get_robust_list(res, ...)
#define __sanitizer_syscall_post_getrusage(res, ...)
#define __sanitizer_syscall_post_getsid(res, ...)
#define __sanitizer_syscall_post_getsockname(res, ...)
#define __sanitizer_syscall_post_getsockopt(res, ...)
#define __sanitizer_syscall_post_get_thread_area(res, ...)
#define __sanitizer_syscall_post_gettid(res, ...)
#define __sanitizer_syscall_post_gettimeofday(res, ...)
#define __sanitizer_syscall_post_getuid32(res, ...)
#define __sanitizer_syscall_post_getuid(res, ...)
#define __sanitizer_syscall_post_getxattr(res, ...)
#define __sanitizer_syscall_post_gtty(res, ...)
#define __sanitizer_syscall_post_idle(res, ...)
#define __sanitizer_syscall_post_init_module(res, ...)
#define __sanitizer_syscall_post_inotify_add_watch(res, ...)
#define __sanitizer_syscall_post_inotify_init1(res, ...)
#define __sanitizer_syscall_post_inotify_init(res, ...)
#define __sanitizer_syscall_post_inotify_rm_watch(res, ...)
#define __sanitizer_syscall_post_io_cancel(res, ...)
#define __sanitizer_syscall_post_ioctl(res, ...)
#define __sanitizer_syscall_post_io_destroy(res, ...)
#define __sanitizer_syscall_post_io_getevents(res, ...)
#define __sanitizer_syscall_post_ioperm(res, ...)
#define __sanitizer_syscall_post_iopl(res, ...)
#define __sanitizer_syscall_post_ioprio_get(res, ...)
#define __sanitizer_syscall_post_ioprio_set(res, ...)
#define __sanitizer_syscall_post_io_setup(res, ...)
#define __sanitizer_syscall_post_io_submit(res, ...)
#define __sanitizer_syscall_post_ipc(res, ...)
#define __sanitizer_syscall_post_kexec_load(res, ...)
#define __sanitizer_syscall_post_keyctl(res, ...)
#define __sanitizer_syscall_post_kill(res, ...)
#define __sanitizer_syscall_post_lchown32(res, ...)
#define __sanitizer_syscall_post_lchown(res, ...)
#define __sanitizer_syscall_post_lgetxattr(res, ...)
#define __sanitizer_syscall_post_linkat(res, ...)
#define __sanitizer_syscall_post_link(res, ...)
#define __sanitizer_syscall_post_listen(res, ...)
#define __sanitizer_syscall_post_listxattr(res, ...)
#define __sanitizer_syscall_post_llistxattr(res, ...)
#define __sanitizer_syscall_post__llseek(res, ...)
#define __sanitizer_syscall_post_lock(res, ...)
#define __sanitizer_syscall_post_lookup_dcookie(res, ...)
#define __sanitizer_syscall_post_lremovexattr(res, ...)
#define __sanitizer_syscall_post_lseek(res, ...)
#define __sanitizer_syscall_post_lsetxattr(res, ...)
#define __sanitizer_syscall_post_lstat64(res, ...)
#define __sanitizer_syscall_post_lstat(res, ...)
#define __sanitizer_syscall_post_madvise1(res, ...)
#define __sanitizer_syscall_post_madvise(res, ...)
#define __sanitizer_syscall_post_mbind(res, ...)
#define __sanitizer_syscall_post_migrate_pages(res, ...)
#define __sanitizer_syscall_post_mincore(res, ...)
#define __sanitizer_syscall_post_mkdirat(res, ...)
#define __sanitizer_syscall_post_mkdir(res, ...)
#define __sanitizer_syscall_post_mknodat(res, ...)
#define __sanitizer_syscall_post_mknod(res, ...)
#define __sanitizer_syscall_post_mlockall(res, ...)
#define __sanitizer_syscall_post_mlock(res, ...)
#define __sanitizer_syscall_post_mmap2(res, ...)
#define __sanitizer_syscall_post_mmap(res, ...)
#define __sanitizer_syscall_post_modify_ldt(res, ...)
#define __sanitizer_syscall_post_mount(res, ...)
#define __sanitizer_syscall_post_move_pages(res, ...)
#define __sanitizer_syscall_post_mprotect(res, ...)
#define __sanitizer_syscall_post_mpx(res, ...)
#define __sanitizer_syscall_post_mq_getsetattr(res, ...)
#define __sanitizer_syscall_post_mq_notify(res, ...)
#define __sanitizer_syscall_post_mq_open(res, ...)
#define __sanitizer_syscall_post_mq_timedreceive(res, ...)
#define __sanitizer_syscall_post_mq_timedsend(res, ...)
#define __sanitizer_syscall_post_mq_unlink(res, ...)
#define __sanitizer_syscall_post_mremap(res, ...)
#define __sanitizer_syscall_post_msgctl(res, ...)
#define __sanitizer_syscall_post_msgget(res, ...)
#define __sanitizer_syscall_post_msgrcv(res, ...)
#define __sanitizer_syscall_post_msgsnd(res, ...)
#define __sanitizer_syscall_post_msync(res, ...)
#define __sanitizer_syscall_post_munlockall(res, ...)
#define __sanitizer_syscall_post_munlock(res, ...)
#define __sanitizer_syscall_post_munmap(res, ...)
#define __sanitizer_syscall_post_name_to_handle_at(res, ...)
#define __sanitizer_syscall_post_nanosleep(res, ...)
#define __sanitizer_syscall_post_newfstatat(res, ...)
#define __sanitizer_syscall_post__newselect(res, ...)
#define __sanitizer_syscall_post_nfsservctl(res, ...)
#define __sanitizer_syscall_post_nice(res, ...)
#define __sanitizer_syscall_post_oldfstat(res, ...)
#define __sanitizer_syscall_post_oldlstat(res, ...)
#define __sanitizer_syscall_post_oldolduname(res, ...)
#define __sanitizer_syscall_post_oldstat(res, ...)
#define __sanitizer_syscall_post_olduname(res, ...)
#define __sanitizer_syscall_post_openat(res, ...)
#define __sanitizer_syscall_post_open_by_handle_at(res, ...)
#define __sanitizer_syscall_post_open(res, ...)
#define __sanitizer_syscall_post_pause(res, ...)
#define __sanitizer_syscall_post_perf_event_open(res, ...)
#define __sanitizer_syscall_post_personality(res, ...)
#define __sanitizer_syscall_post_pipe2(res, ...)
#define __sanitizer_syscall_post_pipe(res, ...)
#define __sanitizer_syscall_post_pivot_root(res, ...)
#define __sanitizer_syscall_post_poll(res, ...)
#define __sanitizer_syscall_post_ppoll(res, ...)
#define __sanitizer_syscall_post_prctl(res, ...)
#define __sanitizer_syscall_post_pread64(res, ...)
#define __sanitizer_syscall_post_preadv(res, ...)
#define __sanitizer_syscall_post_prlimit64(res, ...)
#define __sanitizer_syscall_post_process_vm_readv(res, ...)
#define __sanitizer_syscall_post_process_vm_writev(res, ...)
#define __sanitizer_syscall_post_profil(res, ...)
#define __sanitizer_syscall_post_prof(res, ...)
#define __sanitizer_syscall_post_pselect6(res, ...)
#define __sanitizer_syscall_post_ptrace(res, ...)
#define __sanitizer_syscall_post_putpmsg(res, ...)
#define __sanitizer_syscall_post_pwrite64(res, ...)
#define __sanitizer_syscall_post_pwritev(res, ...)
#define __sanitizer_syscall_post_query_module(res, ...)
#define __sanitizer_syscall_post_quotactl(res, ...)
#define __sanitizer_syscall_post_readahead(res, ...)
#define __sanitizer_syscall_post_readdir(res, ...)
#define __sanitizer_syscall_post_readlinkat(res, ...)
#define __sanitizer_syscall_post_readlink(res, ...)
#define __sanitizer_syscall_post_read(res, ...)
#define __sanitizer_syscall_post_readv(res, ...)
#define __sanitizer_syscall_post_reboot(res, ...)
#define __sanitizer_syscall_post_recvfrom(res, ...)
#define __sanitizer_syscall_post_recvmmsg(res, ...)
#define __sanitizer_syscall_post_remap_file_pages(res, ...)
#define __sanitizer_syscall_post_removexattr(res, ...)
#define __sanitizer_syscall_post_renameat(res, ...)
#define __sanitizer_syscall_post_rename(res, ...)
#define __sanitizer_syscall_post_request_key(res, ...)
#define __sanitizer_syscall_post_restart_syscall(res, ...)
#define __sanitizer_syscall_post_rmdir(res, ...)
#define __sanitizer_syscall_post_rt_sigaction(res, ...)
#define __sanitizer_syscall_post_rt_sigprocmask(res, ...)
#define __sanitizer_syscall_post_rt_sigqueueinfo(res, ...)
#define __sanitizer_syscall_post_rt_sigreturn(res, ...)
#define __sanitizer_syscall_post_rt_sigsuspend(res, ...)
#define __sanitizer_syscall_post_rt_sigtimedwait(res, ...)
#define __sanitizer_syscall_post_rt_tgsigqueueinfo(res, ...)
#define __sanitizer_syscall_post_sched_getaffinity(res, ...)
#define __sanitizer_syscall_post_sched_getparam(res, ...)
#define __sanitizer_syscall_post_sched_get_priority_max(res, ...)
#define __sanitizer_syscall_post_sched_get_priority_min(res, ...)
#define __sanitizer_syscall_post_sched_getscheduler(res, ...)
#define __sanitizer_syscall_post_sched_rr_get_interval(res, ...)
#define __sanitizer_syscall_post_sched_setaffinity(res, ...)
#define __sanitizer_syscall_post_sched_setparam(res, ...)
#define __sanitizer_syscall_post_sched_setscheduler(res, ...)
#define __sanitizer_syscall_post_sched_yield(res, ...)
#define __sanitizer_syscall_post_security(res, ...)
#define __sanitizer_syscall_post_select(res, ...)
#define __sanitizer_syscall_post_semctl(res, ...)
#define __sanitizer_syscall_post_semget(res, ...)
#define __sanitizer_syscall_post_semop(res, ...)
#define __sanitizer_syscall_post_semtimedop(res, ...)
#define __sanitizer_syscall_post_sendfile64(res, ...)
#define __sanitizer_syscall_post_sendfile(res, ...)
#define __sanitizer_syscall_post_sendmmsg(res, ...)
#define __sanitizer_syscall_post_sendmsg(res, ...)
#define __sanitizer_syscall_post_sendto(res, ...)
#define __sanitizer_syscall_post_setdomainname(res, ...)
#define __sanitizer_syscall_post_setfsgid32(res, ...)
#define __sanitizer_syscall_post_setfsgid(res, ...)
#define __sanitizer_syscall_post_setfsuid32(res, ...)
#define __sanitizer_syscall_post_setfsuid(res, ...)
#define __sanitizer_syscall_post_setgid32(res, ...)
#define __sanitizer_syscall_post_setgid(res, ...)
#define __sanitizer_syscall_post_setgroups32(res, ...)
#define __sanitizer_syscall_post_setgroups(res, ...)
#define __sanitizer_syscall_post_sethostname(res, ...)
#define __sanitizer_syscall_post_setitimer(res, ...)
#define __sanitizer_syscall_post_set_mempolicy(res, ...)
#define __sanitizer_syscall_post_setns(res, ...)
#define __sanitizer_syscall_post_setpgid(res, ...)
#define __sanitizer_syscall_post_setpriority(res, ...)
#define __sanitizer_syscall_post_setregid32(res, ...)
#define __sanitizer_syscall_post_setregid(res, ...)
#define __sanitizer_syscall_post_setresgid32(res, ...)
#define __sanitizer_syscall_post_setresgid(res, ...)
#define __sanitizer_syscall_post_setresuid32(res, ...)
#define __sanitizer_syscall_post_setresuid(res, ...)
#define __sanitizer_syscall_post_setreuid32(res, ...)
#define __sanitizer_syscall_post_setreuid(res, ...)
#define __sanitizer_syscall_post_setrlimit(res, ...)
#define __sanitizer_syscall_post_set_robust_list(res, ...)
#define __sanitizer_syscall_post_setsid(res, ...)
#define __sanitizer_syscall_post_setsockopt(res, ...)
#define __sanitizer_syscall_post_set_thread_area(res, ...)
#define __sanitizer_syscall_post_set_tid_address(res, ...)
#define __sanitizer_syscall_post_settimeofday(res, ...)
#define __sanitizer_syscall_post_setuid32(res, ...)
#define __sanitizer_syscall_post_setuid(res, ...)
#define __sanitizer_syscall_post_setxattr(res, ...)
#define __sanitizer_syscall_post_sgetmask(res, ...)
#define __sanitizer_syscall_post_shmat(res, ...)
#define __sanitizer_syscall_post_shmctl(res, ...)
#define __sanitizer_syscall_post_shmdt(res, ...)
#define __sanitizer_syscall_post_shmget(res, ...)
#define __sanitizer_syscall_post_shutdown(res, ...)
#define __sanitizer_syscall_post_sigaction(res, ...)
#define __sanitizer_syscall_post_sigaltstack(res, ...)
#define __sanitizer_syscall_post_signalfd4(res, ...)
#define __sanitizer_syscall_post_signalfd(res, ...)
#define __sanitizer_syscall_post_signal(res, ...)
#define __sanitizer_syscall_post_sigpending(res, ...)
#define __sanitizer_syscall_post_sigprocmask(res, ...)
#define __sanitizer_syscall_post_sigreturn(res, ...)
#define __sanitizer_syscall_post_sigsuspend(res, ...)
#define __sanitizer_syscall_post_socketcall(res, ...)
#define __sanitizer_syscall_post_socketpair(res, ...)
#define __sanitizer_syscall_post_socket(res, ...)
#define __sanitizer_syscall_post_splice(res, ...)
#define __sanitizer_syscall_post_ssetmask(res, ...)
#define __sanitizer_syscall_post_stat64(res, ...)
#define __sanitizer_syscall_post_statfs64(res, ...)
#define __sanitizer_syscall_post_statfs(res, ...)
#define __sanitizer_syscall_post_stat(res, ...)
#define __sanitizer_syscall_post_stime(res, ...)
#define __sanitizer_syscall_post_stty(res, ...)
#define __sanitizer_syscall_post_swapoff(res, ...)
#define __sanitizer_syscall_post_swapon(res, ...)
#define __sanitizer_syscall_post_symlinkat(res, ...)
#define __sanitizer_syscall_post_symlink(res, ...)
#define __sanitizer_syscall_post_sync_file_range(res, ...)
#define __sanitizer_syscall_post_syncfs(res, ...)
#define __sanitizer_syscall_post_sync(res, ...)
#define __sanitizer_syscall_post__sysctl(res, ...)
#define __sanitizer_syscall_post_sysfs(res, ...)
#define __sanitizer_syscall_post_sysinfo(res, ...)
#define __sanitizer_syscall_post_syslog(res, ...)
#define __sanitizer_syscall_post_tee(res, ...)
#define __sanitizer_syscall_post_tgkill(res, ...)
#define __sanitizer_syscall_post_timer_create(res, ...)
#define __sanitizer_syscall_post_timer_delete(res, ...)
#define __sanitizer_syscall_post_time(res, ...)
#define __sanitizer_syscall_post_timerfd_create(res, ...)
#define __sanitizer_syscall_post_timerfd_gettime(res, ...)
#define __sanitizer_syscall_post_timerfd_settime(res, ...)
#define __sanitizer_syscall_post_timer_getoverrun(res, ...)
#define __sanitizer_syscall_post_timer_gettime(res, ...)
#define __sanitizer_syscall_post_timer_settime(res, ...)
#define __sanitizer_syscall_post_times(res, ...)
#define __sanitizer_syscall_post_tkill(res, ...)
#define __sanitizer_syscall_post_truncate64(res, ...)
#define __sanitizer_syscall_post_truncate(res, ...)
#define __sanitizer_syscall_post_tuxcall(res, ...)
#define __sanitizer_syscall_post_ugetrlimit(res, ...)
#define __sanitizer_syscall_post_ulimit(res, ...)
#define __sanitizer_syscall_post_umask(res, ...)
#define __sanitizer_syscall_post_umount2(res, ...)
#define __sanitizer_syscall_post_umount(res, ...)
#define __sanitizer_syscall_post_uname(res, ...)
#define __sanitizer_syscall_post_unlinkat(res, ...)
#define __sanitizer_syscall_post_unlink(res, ...)
#define __sanitizer_syscall_post_unshare(res, ...)
#define __sanitizer_syscall_post_uselib(res, ...)
#define __sanitizer_syscall_post_ustat(res, ...)
#define __sanitizer_syscall_post_utimensat(res, ...)
#define __sanitizer_syscall_post_utime(res, ...)
#define __sanitizer_syscall_post_utimes(res, ...)
#define __sanitizer_syscall_post_vfork(res, ...)
#define __sanitizer_syscall_post_vhangup(res, ...)
#define __sanitizer_syscall_post_vm86old(res, ...)
#define __sanitizer_syscall_post_vm86(res, ...)
#define __sanitizer_syscall_post_vmsplice(res, ...)
#define __sanitizer_syscall_post_vserver(res, ...)
#define __sanitizer_syscall_post_waitid(res, ...)
#define __sanitizer_syscall_post_write(res, ...)
#define __sanitizer_syscall_post_writev(res, ...)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SANITIZER_LINUX_SYSCALL_HOOKS_H
