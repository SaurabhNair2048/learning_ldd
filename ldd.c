#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>//The library for proc filesystem related declarations

MODULE_LICENSE("GPL"); //*
MODULE_AUTHOR("Testing-Saura"); //!
MODULE_DESCRIPTION("Our first dynalically loadable read kernel module"); //!

static struct proc_dir_entry* proc_entry;

char user_message[128];


// read function implementation for proc_ops has the following structure which is a function pointer
// ssize_t (*func_ptr)(struct file* , char* __user, size_t, loff_t*);

// An example is, we define a function inpyjama_read and define a pointer to it in proc_ops structure:
ssize_t inpyjama_read(struct file* file_pointer,
                        char* user_space_buffer,
                        size_t count,
                        loff_t* offset){
                            printk("Inside pyjama read function !!!");
                            // Now to send data to user, we use the user_space_buffer to copy the message.
                            // char msg[] = "Ack!\n";
                            // size_t len = strlen(msg);
                            size_t len = strlen(user_message);

                            if(*offset >= len){
                                user_message[0] = '\0';
                                return 0;
                            }

                            // copy to user api takes the destination, source and no. of bytes to be copied
                            // int result = copy_to_user(user_space_buffer, msg, len);
                            int result = copy_to_user(user_space_buffer, user_message, len);
                            // return 0;

                            //return len; // this tells the number of bytes written to the user space buffer. This allows cat command to read it.
                            
                            // Now if we just return the value, and use cat to read the driver file,
                            // the way cat works is continues to read the file until it reach EOF or 0 is returned.
                            // In above case, once cat command read len bytes, it calls read again and read agian returns len and so it goes into an infinite loop.
                            // So we need to use the offset field of the function call to inform cat command where to read the next value from.
                            // what offset does is it can be used to keep track of how many bytes have been read and how many are yet to be read from the user buffer.
                            *offset += len;

                            return len;
                        }


ssize_t inpyjama_saura_write(struct file* file_pointer,
                        const char* user_space_buffer,
                        size_t count,
                        loff_t* offset){
                            if(*offset >= count){
                                return 0;
                            }
                            if(count > 127){
                                printk("Message bigger than memory");
                                return -1;
                            }
                            int res = copy_from_user(user_message, user_space_buffer, count);
                            if(res > 0){
                                printk("Failed to copy message to kernel buffer");
                                return -1;
                            }
                            printk("Written message to kernel buffer!!");
                            return count;
                        }


static struct proc_ops p_ops = {
    .proc_read = inpyjama_read,
    .proc_write = inpyjama_saura_write
};


static int saura_module_init(void) {
    printk("Hello, World! (from InPyjama by Saura!!)\n");
    printk("Starting to create the proc entry!!");
    proc_entry = proc_create("saura_inpyjama_driver_s_file",
                                0666,
                                NULL,
                                &p_ops);
    printk("Exiting the proc entry creation process!!");
    return 0;
}

static void saura_module_exit(void) {
    printk("Bye! (from InPyjama by Saura!!)\n");
    printk("Starting to remove the proc entry!!");
    proc_remove(proc_entry); //It removes the proc entry and returns void
    printk("Exiting the proc entry removal process!!");
}

module_init(saura_module_init);
module_exit(saura_module_exit);

/*
The function to create a proc directory entry. /proc is a virtual filesysem which contains virtual files which are used to
expose the status and characteristics of the kernel resources and processes
struct proc_dir_entry* proc_create(const char* name, // name of the proc directory entry
                                    umode_t mode, // the mode of the create entry
                                    struct proc_dir_entry* parent, // the parent of the proc directory entry
                                    const struct proc_ops* proc_ops); // the proc_ops the proc entry supports
proc_ops is a structure of function pointers. They point to functions to be called for specific actions such as read being called on the proc file
*/

/*
When to Use Which Function?
kmalloc(): Use this for small to medium-sized allocations (up to a few MB) where contiguous memory is required.
kzalloc(): Use this when you want to allocate memory and initialize it to zero.
vmalloc(): Use this when you need a large block of memory and physical contiguity is not required.
alloc_page() / get_free_pages(): Use these when you need to allocate pages directly, often when working with memory 
that is managed at the page level.

Why Not malloc() in Kernel Space?
malloc() and the memory allocation functions in the C Standard Library are designed for user-space memory management. 
They don't consider kernel constraints such as physical memory contiguity, interrupt context, or kernel-specific memory management policies.
Kernel memory allocation requires handling page boundaries, cache coherency, and other hardware-related concerns, 
which is why specialized functions like kmalloc(), vmalloc(), and kzalloc() exist.

*/

/*
Here are the key headers you need for each of these functions:
kmalloc(), kzalloc(), vmalloc() These functions are part of the general memory management APIs in the Linux kernel.

Header File: <linux/slab.h>
This header defines memory allocation functions like kmalloc(), kzalloc(), and other related functions.
It also provides memory caching mechanisms and allows dynamic memory management.
Example:

#include <linux/slab.h>  // For kmalloc(), kzalloc(), etc.


alloc_page() This function is specifically for allocating a single page of memory in kernel space.

Header File: <linux/gfp.h>
alloc_page() uses the gfp_t flags, which are defined in <linux/gfp.h>, and it returns a struct page *, which represents the page.
Example:

#include <linux/gfp.h>  // For gfp_t flags
#include <linux/mm.h>   // For alloc_page() and related page functions


get_free_pages() This function is used to allocate a contiguous block of memory (in the form of pages). 
It also requires the gfp_t flags, and get_free_pages() returns the starting physical address of the allocated block.

Header File: <linux/gfp.h> and <linux/mm.h>
get_free_pages() is often used for allocating blocks of contiguous pages.
Example:
#include <linux/gfp.h>   // For gfp_t flags
#include <linux/mm.h>    // For get_free_pages()


Summary of Required Headers:
Memory Allocation Function	Required Header Files
kmalloc()	<linux/slab.h>
kzalloc()	<linux/slab.h>
vmalloc()	<linux/slab.h>
alloc_page()	<linux/gfp.h>, <linux/mm.h>
get_free_pages()	<linux/gfp.h>, <linux/mm.h>
Example Code:

*/