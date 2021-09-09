# CS550_PageFaultTracking




to run the program(in sudo mode):
$make
$insmod pf_probe_A.ko pid=$pid
$insmod pf_probe_B.ko pid=$pid
$rmmod pf_probe_A.ko
$rmmod pf_probe_B.ko

$dmesg


you also could use the shellscript(cmd.sh) to test the correctness of pf_probe_B

in my three examples(I/O and compute  compute  Network I/O)
I use speedtest cmd to demo the Network intensive. 
I use a iterate.c file to demo the compute intensive.(you could compile it by yourself, i have offered the source file)
I use make cmd(for pf_probe_A) to demo the kernel compilation case.(you could revise the make file i offered easily to see whats happened).

in my case, the max of occurence of documented page fault is 100. you could revise it to see more.
