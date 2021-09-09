#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/ktime.h>
#include <linux/slab.h>
#include <linux/math64.h>

static int pid = 1;

static ktime_t t1,t2;

static char pf[31][72];

typedef struct pft {
        ktime_t t;
        u64 addr;
}pft;


static pft page_fault_table[100];
static int i = 0;
module_param(pid,int,S_IRUSR | S_IWUSR |S_IRGRP |S_IROTH);

/* For each probe you need to allocate a kprobe structure */
static struct kprobe kp = {
	.symbol_name	= "handle_mm_fault",
};

/* kprobe pre_handler: called just before the probed instruction is executed */
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
#ifdef CONFIG_X86
    if(current->pid == pid){
        if(i==0){
            t1 = ktime_get();
        }
        if(i<100){
            page_fault_table[i].t = ktime_get();
            page_fault_table[i].addr = (regs->si)>>12;
            printk(KERN_CONT "page_fault_table[%d].addr=%lld\n",i,page_fault_table[i].addr);
            i++;
        }
    }
#endif

	/* A dump_stack() here will give a stack backtrace */
	return 0;
}

/* kprobe post_handler: called after the probed instruction is executed */
static void handler_post(struct kprobe *p, struct pt_regs *regs,
				unsigned long flags)
{
#ifdef CONFIG_X86
	/*pr_info("<%s> post_handler: p->addr = 0x%p, flags = 0x%lx\n",
		p->symbol_name, p->addr, regs->flags);*/
#endif

}

/*
 * fault_handler: this is called if an exception is generated for any
 * instruction within the pre- or post-handler, or when Kprobes
 * single-steps the probed instruction.
 */
static int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
	printk(KERN_INFO "fault_handler: p->addr = 0x%p, trap #%dn", p->addr, trapnr);
	/* Return 0 because we don't handle the fault. */
	return 0;
}

static int __init kprobe_init(void)
{
	int ret,m,n;
	kp.pre_handler = handler_pre;
	kp.post_handler = handler_post;
	kp.fault_handler = handler_fault;
	for(m=0;m<31;m++)
	    for(n=0;n<71;n++)
	        pf[m][n]=' ';
	ret = register_kprobe(&kp);
	if (ret < 0) {
		pr_err("register_kprobe failed, returned %d\n", ret);
		return ret;
	}
	printk(KERN_INFO "Planted kprobe at %p\n", kp.addr);
	return 0;
}

static void __exit kprobe_exit(void)
{
    int j,x,y,s,t;
    u64 tslice;
    u64 divisor_t = 70;
    u64 aslice;
    u64 divisor_a = 30;
    u64 max = page_fault_table[0].addr;
    u64 min = page_fault_table[0].addr;
    t2 = ktime_get();
    tslice=div64_u64(ktime_sub(t2,t1),divisor_t);
    for(j=0;j<i;j++){
        if(page_fault_table[j].addr >max)
            max = page_fault_table[j].addr;
        if(page_fault_table[j].addr <min)
            min = page_fault_table[j].addr;
    }
    aslice = div64_u64(max-min,divisor_a);
    printk(KERN_INFO "aslice = %lld,tslice = %lld,i=%d\n",aslice,tslice,i);
    for(j=0;j<i;j++){
        y = div64_u64((page_fault_table[j].addr - min),aslice);
        x = div64_u64(ktime_sub(page_fault_table[j].t,t1),tslice);
        pf[x][y+1] = '*';
    }

    printk(KERN_CONT "*************time-virtual_addr plot is as follow**********************\n");

    for(s=30;s>=0;s--){
        for(t=0;t<=71;t++){
        	if(t==0)
        		if(s==30)
        			printk(KERN_CONT "^");
        		else
        			printk("|");
        	else	
            	printk(KERN_CONT "%c",pf[s][t]);
        }
        printk(KERN_CONT "\n");
    }
    for (s=0;s<73;s++)
    {
    	if(s==72)
    		printk(KERN_CONT ">");
    	else
    		printk(KERN_CONT "-");
    }
    printk(KERN_CONT "\n");
    printk(KERN_CONT "X-axis is time ranging from the moment the first pagefault happened to the moment the module unloaded\n");
    printk(KERN_CONT "Y-axis is virtual page number ranging from the min of virtual page number to the max\n");
    printk(KERN_CONT "**************************************************************************************\n");
	unregister_kprobe(&kp);
	printk(KERN_INFO "kprobe at %p unregistered\n", kp.addr);
}

module_init(kprobe_init)
module_exit(kprobe_exit)
MODULE_LICENSE("GPL");
