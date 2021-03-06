#include <linux/module.h>
#include <net/sock.h>
#include <linux/init.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#define NETLINK_USER 31

struct sock *nl_sk = NULL;

static int parse_input(char command[])
{

	int success = 0;

	char param_1[128], param_2[32];
	struct task_struct *task;

	strcpy(param_1, command + 3);

	// Get first parameter
	printk(KERN_INFO "parameter: %s", param_1);

	// Determine which function is being used
	// For Kill Process by Name
	if (strncmp(command, "kn", 2) == 0)
	{

		for_each_process(task)
		{
			if(strstr(param_1, task->comm) != NULL)
			{
				printk(KERN_INFO "%s has process id %d", task->comm, task->pid);
				send_sig_info(SIGKILL, NULL, task);
				success = 1;
			}
		
		}
	}

	return success;
}

static void receive_msg(struct sk_buff *skb) {

	struct nlmsghdr *nlh;
	int pid;
	struct sk_buff *skb_out;
	int msg_size;
	char msg[8100]; 
	char param_1[128];

	msg_size = strlen(msg);

	nlh = (struct nlmsghdr*)skb->data;

	char *command = (char*)nlmsg_data(nlh);

	// Command should be two characters long, so the param is
	// 3 past the beginning including the space
	// Excludes null character so that it works properlly with strstr
	// with task->comm (hence strlen(command + 4))
	memcpy(param_1, command + 3, strlen(command + 4));

	if (strncmp(command, "ps", 2) == 0)
	{
		struct task_struct *task;

		// Used after for_each_process loop. Copy stock message into
		// msg if nothing is found
		int num_found = 0;

		for_each_process(task)
		{
			// If param is a substring of comm, print it out to message	
			if (strstr(task->comm, param_1) != NULL)
			{
				strcpy(msg + strlen(msg), "\n");
				strcpy(msg + strlen(msg), task->comm);
				strcpy(msg + strlen(msg), ", pid: ");
				sprintf(msg + strlen(msg), "%d", task->pid);
				num_found++;
			}
		}

		if(num_found == 0)
		{
			strcpy(msg, "\nProcess not found.");
		}
	}
	else
	{
		// Parse input and see if kn command is being used. 
		// Will not kill process without providing complete full name
		if (parse_input(command) == 0)
		{
			strcpy(msg, "\nProcess not found. No processes were killed.");
		}
		else
		{
			strcpy(msg, "\nProcess killed!");
		}
	}

	// Get final message length to properlly send back to user program
	msg_size = strlen(msg);

	pid = nlh->nlmsg_pid; 

	skb_out = nlmsg_new(msg_size,0);

	if (!skb_out)
	{

	    printk(KERN_ERR "SKB not allocated properlly\n");
	    return;

	} 

	nlh = nlmsg_put(skb_out,0,0,NLMSG_DONE,msg_size,0);  
	NETLINK_CB(skb_out).dst_group = 0;
	strncpy(nlmsg_data(nlh),msg,msg_size);

	int result = nlmsg_unicast(nl_sk,skb_out,pid);

	if (result < 0) { printk(KERN_INFO "Sending error.\n"); }

}

static int __init kscan_init(void)
{

	// Used by netlink_kernel_create to set the function to call upon receiving a packet
	struct netlink_kernel_cfg cfg = 
	{
	    .input = receive_msg,
	};

	nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);

	if (!nl_sk)
	{
	    printk(KERN_ALERT "Error creating socket.\n");
	    return -10;
	}	

	return 0;	
}

static void __exit kscan_exit(void) 
{

	printk(KERN_INFO "exiting\n");
	netlink_kernel_release(nl_sk);
}

module_init(kscan_init); module_exit(kscan_exit);

MODULE_LICENSE("GPL");