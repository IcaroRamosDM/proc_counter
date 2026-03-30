#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Icaro");
MODULE_DESCRIPTION("A simple Linux kernel module that will later expose a /proc interface");
MODULE_VERSION("0.1");

static int __init proc_counter_init(void)
{
    pr_info("proc_counter: module loaded\n");
    return 0;
}

static void __exit proc_counter_exit(void)
{
    pr_info("proc_counter: module unloaded\n");
}

module_init(proc_counter_init);
module_exit(proc_counter_exit);

