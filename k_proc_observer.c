#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched/signal.h>
#include <linux/notifier.h>

#define PROC_NAME "kproc_observer"

static struct proc_dir_entry *proc_entry;
static unsigned long process_count = 0;
static pid_t last_pid = -1;
static char last_comm[TASK_COMM_LEN];

/* ---------- Process Notifier ---------- */
static int process_event(struct notifier_block *nb,
                         unsigned long action, void *data)
{
    struct task_struct *task = data;

    if (action == PROC_EVENT_FORK) {
        process_count++;
        last_pid = task->pid;
        strncpy(last_comm, task->comm, TASK_COMM_LEN);
    }
    return NOTIFY_OK;
}

static struct notifier_block proc_nb = {
    .notifier_call = process_event,
};

/* ---------- /proc Interface ---------- */
static int proc_show(struct seq_file *m, void *v)
{
    seq_printf(m,
        "Kernel Process Observer\n"
        "-----------------------\n"
        "Processes created : %lu\n"
        "Last PID          : %d\n"
        "Last Command      : %s\n",
        process_count,
        last_pid,
        last_comm
    );
    return 0;
}

static int proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_show, NULL);
}

static const struct proc_ops proc_fops = {
    .proc_open    = proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

/* ---------- Module Init / Exit ---------- */
static int __init kproc_init(void)
{
    proc_entry = proc_create(PROC_NAME, 0444, NULL, &proc_fops);
    if (!proc_entry)
        return -ENOMEM;

    register_task_notifier(&proc_nb);

    pr_info("kproc_observer loaded\n");
    return 0;
}

static void __exit kproc_exit(void)
{
    unregister_task_notifier(&proc_nb);
    proc_remove(proc_entry);
    pr_info("kproc_observer unloaded\n");
}

module_init(kproc_init);
module_exit(kproc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tanmay Samanta");
MODULE_DESCRIPTION("Educational Linux Kernel Process Observer");
