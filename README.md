# Spoil CPU Usage

This repository contains a tool used to spoil CPU usage computation
on linux system with RT_PREEMPT patch enabled.

![htop](doc/htop.gif)

## Build

```bash
cmake -B build
cmake --build build
```

## Problem

There is a good description of the problem available at https://docs.kernel.org/admin-guide/cpu-load.html.
Following this, all tools that measure CPU usage using `/proc/stat` such as `htop` may display incorrect values.

## Other Ideas

### cgroup v1

![cgroups](doc/cgroups.gif)


> ![NOTE]
> cpuacct was removed in cgroup v2

## Status Quo

No solution yet. All investigated mechanism have pitfalls:

- `/proc/stat` is not reliable as shown above.
- `cgroup v1 cpuacct` seams promissing, but was removed in `cgroup v2`.
- `cgroup v2` do not take realtime process into account until they are rinning in the root group.  
  (see https://docs.kernel.org/admin-guide/cgroup-v2.html#cpu)
- `/proc/schedsat` may be correct, but there are (yet unconfirmed) voices that see security issues due to information leakage and state that `/proc/schedstat` consumes 1-2% of CPU usage when enabled.  
  (some refernces are provided below, most of them they not mention /proc/schedstat directly, but /proc in general)
  - the visibility of can be limited using `chmod 0400 /proc/schedstat`

## References

- https://docs.kernel.org/admin-guide/cpu-load.html
- https://docs.redhat.com/en/documentation/red_hat_enterprise_linux/6/html/resource_management_guide/sec-cpuacct
- https://www.man7.org/linux/man-pages/man7/cgroups.7.html
- https://docs.kernel.org/admin-guide/cgroup-v1/cgroups.html
- https://docs.kernel.org/admin-guide/cgroup-v2.html
- https://docs.kernel.org/scheduler/sched-stats.html
- https://gruss.cc/files/procharvester.pdf
- https://www.kernel.org/doc/html/v6.2/security/self-protection.html
- https://www.kicksecure.com/wiki/Security-misc
- https://www.kernel.org/doc/html/latest/filesystems/proc.html
- https://vmonaco.com/papers/SoK-%20Keylogging%20Side%20Channels.pdf
- https://yinqian.org/papers/ccs15.pdf
