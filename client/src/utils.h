#ifndef _UTILS_H_
#define _UTILS_H_

//define CLIENT_DEBUG
#undef PDEBUG             /* undef it, just in case */
#ifdef CLIENT_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "custom application: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#endif // _UTILS_H_
