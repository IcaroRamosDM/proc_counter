#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#define PROC_NAME "proc_counter"
#define INPUT_CAPACITY 128U

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Icaro Ramos Rodrigues dos Santos");
MODULE_DESCRIPTION("Educational /proc interface with synchronized counters");
MODULE_VERSION("0.7");

static struct proc_dir_entry *proc_entry;
static unsigned long long read_count;
static unsigned long long write_count;
static int stored_value;
static DEFINE_MUTEX(counter_mutex);

static int counter_show(struct seq_file *output, void *unused) {
    unsigned long long reads;
    unsigned long long writes;
    int value;

    (void)unused;
    mutex_lock(&counter_mutex);
    read_count++;
    reads = read_count;
    writes = write_count;
    value = stored_value;
    mutex_unlock(&counter_mutex);

    /* seq_file handles the caller's size and offset for partial reads. */
    seq_printf(output, "Read count: %llu\nWrite count: %llu\nStored value: %d\n", reads, writes,
               value);
    return 0;
}

static int counter_open(struct inode *inode, struct file *file) {
    (void)inode;
    return single_open(file, counter_show, NULL);
}

static ssize_t counter_write(struct file *file, const char __user *user_buffer, size_t count,
                             loff_t *offset) {
    char buffer[INPUT_CAPACITY];
    char *command;
    int value = 0;
    bool reset;

    (void)file;
    (void)offset;
    if (count == 0) {
        return 0;
    }
    if (count >= sizeof buffer) {
        return -EINVAL;
    }
    if (copy_from_user(buffer, user_buffer, count)) {
        return -EFAULT;
    }
    /* Embedded NUL bytes must not hide an unparsed suffix. */
    if (memchr(buffer, '\0', count)) {
        return -EINVAL;
    }
    buffer[count] = '\0';
    command = strim(buffer);
    reset = strcmp(command, "reset") == 0;
    if (!reset && kstrtoint(command, 10, &value) != 0) {
        return -EINVAL;
    }

    if (mutex_lock_interruptible(&counter_mutex)) {
        return -ERESTARTSYS;
    }
    if (reset) {
        read_count = 0;
        write_count = 0;
        stored_value = 0;
    } else {
        write_count++;
        stored_value = value;
    }
    mutex_unlock(&counter_mutex);
    return (ssize_t)count;
}

static const struct proc_ops counter_operations = {
    .proc_open = counter_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = counter_write,
};

static int __init counter_init(void) {
    proc_entry = proc_create(PROC_NAME, 0644, NULL, &counter_operations);
    if (proc_entry == NULL) {
        return -ENOMEM;
    }
    pr_info("proc_counter: created /proc/%s\n", PROC_NAME);
    return 0;
}

static void __exit counter_exit(void) {
    proc_remove(proc_entry);
    pr_info("proc_counter: removed /proc/%s\n", PROC_NAME);
}

module_init(counter_init);
module_exit(counter_exit);
