#include <linux/init.h>         // Macros __init and __exit
#include <linux/kernel.h>       // pr_info(), pr_err()
#include <linux/module.h>       // Kernel module infrastructure
#include <linux/proc_fs.h>      // /proc API
#include <linux/uaccess.h>      // copy_to_user(), copy_from_user()
#include <linux/string.h>       // strcmp(), strncmp(), strcspn()
#include <linux/mutex.h>        // mutex, mutex_lock, mutex_unlock

#define PROC_NAME "proc_counter"    // Name of the file in /proc
#define BUFFER_SIZE 128             // Maximum size of user input

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Icaro");
MODULE_DESCRIPTION("Kernel module with /proc R/W interface");
MODULE_VERSION("0.6");

// Ponteiro para a entrada criada em /proc
static struct proc_dir_entry *proc_entry;

/*
 * Internal state of the module.
 * These variables are shared across different accesses to the /proc file
 * so they must be protected with a mutex
 */
static int read_count = 0;
static int write_count = 0;
static int stored_value = 0;

/* Mutex used to protect the module's internal state */
static DEFINE_MUTEX(proc_counter_mutex);

/*
 * Function called when the user runs:
 * cat /proc/proc_counter
 */
static ssize_t proc_read(struct file *file, char __user *user_buffer,
                        size_t count, loff_t *offset)
{
    char buffer[128];
    int len;
    int local_read_count;
    int local_write_count;
    int local_stored_value;

    // Prevent infinite reads
    if (*offset > 0)
        return 0;

    /*
     * Critical section to safely update and read the state.
     */
    mutex_lock(&proc_counter_mutex);

    read_count++;

    /*
     * Copy values into local variables while holding the mutex.
     * This ensures a conssitent snapshot of the state.
     */
    local_read_count = read_count;
    local_write_count = write_count;
    local_stored_value = stored_value;

    mutex_unlock(&proc_counter_mutex);

    // Build the message to send to user space
    len = snprintf(buffer, sizeof(buffer),
                  "Read count: %d\n"
                  "Write count: %d\n"
                  "Stored value: %d\n",
                  local_read_count, local_write_count, local_stored_value);

    // Copy data from kernel space to user space
    if (copy_to_user(user_buffer, buffer, len))
        return -EFAULT;

    *offset = len; // Update offset to indicate data has been read

    return len; // Return number of bytes read
}

/*
 * Function called when writing to /proc/proc_counter
 * Example:
 * echo reset > /proc/proc_counter
 */
static ssize_t proc_write(struct file *file, const char __user *user_buffer,
                          size_t count, loff_t *offset)
{
    char buffer[BUFFER_SIZE];
    int value;
    ssize_t ret = count;

    // Prevent overflow
    if (count >= BUFFER_SIZE)
        return -EINVAL;

    // Copy data from user space to kernel space
    if (copy_from_user(buffer, user_buffer, count))
        return -EFAULT;

    buffer[count] = '\0'; // Ensure null-terminated string

    buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline from echo input

    mutex_lock(&proc_counter_mutex);

    write_count++;

    /*
     * Check commands
     */

    // Case: reset
    if (strcmp(buffer, "reset") == 0) {
        read_count = 0;
        write_count = 0;
        stored_value = 0;
        pr_info("proc_counter: reset command received\n");
        goto out_unlock;
    }
    
    /*
     * If not "reset", try to parse input as integer.
     */
    if (kstrtoint(buffer, 10, &value) == 0) {
        stored_value = value;
        pr_info("proc_counter: stored value updated to %d\n", stored_value);
        goto out_unlock;
    }

    /*
     * If input is neither reset nor a valid integer
     */
    pr_err("proc_counter: invalid input: \"%s\"\n", buffer);
    ret = -EINVAL;

out_unlock:
    mutex_unlock(&proc_counter_mutex);
    return ret;
}

/*
 * Structure defining operations for the /proc file
 */
static const struct proc_ops proc_file_ops = {
    .proc_read = proc_read, // Associate read handler
    .proc_write = proc_write, // Associate write handler
};

/*
 * Module initialization
 */
static int __init proc_counter_init(void)
{
    /*
     * Create /proc/proc_counter file
     */
    proc_entry = proc_create(PROC_NAME, 0666, NULL, &proc_file_ops);

    if (!proc_entry) {
        pr_err("proc_counter: failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }

    pr_info("proc_counter: module loaded\n");
    return 0;
}

/*
 * Module cleanup
 */
static void __exit proc_counter_exit(void)
{
    /*
     * Remove /proc entry
     */
    proc_remove(proc_entry);
    pr_info("proc_counter: module unloaded\n");
}

module_init(proc_counter_init);
module_exit(proc_counter_exit);

