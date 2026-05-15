cmake_minimum_required(VERSION 3.28)
project(spoil-cpu-usage)

add_executable(spoil_cpu_usage src/spoil_cpu_usage.c)
install(TARGETS spoil_cpu_usage)

add_executable(show_cpu_usage src/show_cpu_usage.c)
install(TARGETS show_cpu_usage)

add_executable(show_cpu_usage_cgroups src/show_cpu_usage_cgroups.c)
install(TARGETS show_cpu_usage_cgroups)

add_executable(show_cpu_usage_schedstat src/show_cpu_usage_schedstat.c)
install(TARGETS show_cpu_usage_schedstat)
