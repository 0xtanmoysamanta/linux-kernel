#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched/signal.h>
#include <linux/notifier.h>

#define PROC_NAME "process_telemetry"

/* ---------- Telemetry Data ---------- */
static unsigned long fork_count = 0;
static unsigned long exit_count = 0;
static pid_t last_pid = -1;
static char last_comm[TASK_COMM_LEN] = "none";

/* ---------- Process Notifier ---------- */
static int process_event(struct notifier_block *nb,
                         unsigned long action, void *data)
{
    struct task_struct *task = data;

    switch (action) {
    case PROC_EVENT_FORK:
        fork_count++;
        last_pid = task->pid;
        strncpy(last_comm, task->comm, TASK_COMM_LEN);
        break;

    case PROC_EVENT_EXIT:
        exit_count++;
        break;
    }

    return NOTIFY_OK;
}

static struct notifier_block proc_nb = {
    .notifier_call = process_event,
};

/* ---------- /proc Interface ---------- */
static int telemetry_show(struct seq_file *m, void *v)
{
    seq_printf(m,
        "Process Telemetry Module\n"
        "------------------------\n"
        "Fork events  : %lu\n"
        "Exit events  : %lu\n"
        "Last PID     : %d\n"
        "Last Command : %s\n",
        fork_count,
        exit_count,
        last_pid,
        last_comm
    );
    return 0;
}

static int telemetry_open(struct inode *inode, struct file *file)
{
    return single_open(file, telemetry_show, NULL);
}

static const struct proc_ops telemetry_fops = {
    .proc_open    = telemetry_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

/* ---------- Module Init / Exit ---------- */
static int __init telemetry_init(void)
{
    if (!proc_create(PROC_NAME, 0444, NULL, &telemetry_fops))
        return -ENOMEM;

    register_task_notifier(&proc_nb);

    pr_info("process_telemetry loaded\n");
    return 0;
}

static void __exit telemetry_exit(void)
{
    unregister_task_notifier(&proc_nb);
    remove_proc_entry(PROC_NAME, NULL);

    pr_info("process_telemetry unloaded\n");
}

module_init(telemetry_init);
module_exit(telemetry_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tanmay Samanta");
MODULE_DESCRIPTION("Educational Linux Kernel Process Telemetry Module");
