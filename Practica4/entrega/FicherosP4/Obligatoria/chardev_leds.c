#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for copy_to_user */
#include <linux/cdev.h>

// modleds imports
#include <asm-generic/errno.h>
#include <linux/init.h>
#include <linux/tty.h>      /* For fg_console */
#include <linux/kd.h>       /* For KDSETLED */
#include <linux/vt_kern.h>
#include <linux/version.h> /* For LINUX_VERSION_CODE */
 
struct tty_driver* kbd_driver= NULL;//manejador de los leds

MODULE_LICENSE("GPL");

/*
 *  Prototypes de chardev, el unico que cambiamos es write.
 *  Tambien usamos tal cual las funciones de modleds setleds
 */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "leds" /* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80		/* Max length of the message from the device */
#define ALL_LEDS_ON 0x7
#define ALL_LEDS_OFF 0
/*
 * Global variables are declared as static, so are global within the file.
 */
dev_t start;//para major minor (dev_t)
struct cdev* chardev=NULL;
static int Device_Open = 0;	/* Is device open? 0 -> no 1 ->si
				 * Used to prevent multiple access to device */
static char msg[BUF_LEN];	/* The msg the device will give when asked */// de donde leemos
static char *msg_Ptr;		/* This will be initialized every time the
				   device is opened successfully */
static int counter=0;		/* Tracks the number of times the character
				 * device has been opened */

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

// Dos funciones de modleds
/* Get driver handler */
struct tty_driver* get_kbd_driver_handler(void)//devuelve el manejador
{
    printk(KERN_INFO "modleds: loading\n");
    printk(KERN_INFO "modleds: fgconsole is %x\n", fg_console);
#if ( LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32) )
    return vc_cons[fg_console].d->port.tty->driver;
#else
    return vc_cons[fg_console].d->vc_tty->driver;
#endif
}

/* Set led state to that specified by mask */
static inline int set_leds(struct tty_driver* handler, unsigned int mask)//pone en leds el valor de la mascara
{
#if ( LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32) )
    return (handler->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,mask);
#else
    return (handler->ops->ioctl) (vc_cons[fg_console].d->vc_tty, NULL, KDSETLED, mask);
#endif
}


/*
 * This function is called when the module is loaded
 */
int init_module(void)
{
    int major;	/* Major number assigned to our device driver */
    int minor;  /* Minor number assigned to the associated character device */
    int ret;

    // Get available (major,minor) range for the driver
    // 0 es el minor que queremos que tenga (con el major que pueda); 1 es la cantidad de minors que reservamos
    if ((ret=alloc_chrdev_region (&start, 0, 1, DEVICE_NAME))) {
        printk(KERN_INFO "Can't allocate chardev_leds_region()");
        return ret;
        //return -ENOMEM;porque ret y no ENOMEM?/ es por el codigo del error que tiene ret?
    }
    
    /* Create associated cdev */

    if ((chardev=cdev_alloc())==NULL) {
        printk(KERN_INFO "cdev_alloc() failed ");
        unregister_chrdev_region(start, 1);// es necesario?
        return -ENOMEM;
    }

    cdev_init(chardev,&fops); // asocia operaciones al dispositivo
    
    // Add CHARACTER DEVICE (chardev) device to the system.

    if ((ret=cdev_add(chardev,start,1))) {
        printk(KERN_INFO "cdev_add() failed ");
        kobject_put(&chardev->kobj);
        unregister_chrdev_region(start, 1);
        return ret;
    }

    major=MAJOR(start);
    minor=MINOR(start);
    //Escribe la informacion en un fichero log
    printk(KERN_INFO "Es usted un genio que ha superado la prueba chardevleds!!");
    printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);
    printk(KERN_INFO "the driver, create a dev file with\n");
    printk(KERN_INFO "'sudo mknod -m 666 /dev/%s c %d %d'.\n", DEVICE_NAME, major,minor);
    printk(KERN_INFO "Try to cat and echo to the device file.\n");
    printk(KERN_INFO "Remove the device file and module when done.\n");

    return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
    /* Destroy chardev */
    if (chardev)
        cdev_del(chardev);

    //Deja los leds apagados como exit de modleds
    set_leds(kbd_driver,ALL_LEDS_OFF);

    /*
     * Unregister the device
     */
    unregister_chrdev_region(start, 1);
}


 
/*
 * Called when a process tries to open the device file, like
 * "cat /dev/chardev"
 */
static int device_open(struct inode *inode, struct file *file)
{
    if (Device_Open){
        printk(KERN_ALERT "Failed device_open\n"); 
        return -EBUSY;
    }

    Device_Open++;

    /* Initialize msg */
    sprintf(msg, "I already told you %d times Hello world!\n", counter++);

    /* Initially, this points to the beginning of the message */
    msg_Ptr = msg;

    /* Increase the module's reference counter */
    try_module_get(THIS_MODULE);

    return SUCCESS;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
    Device_Open--;		/* We're now ready for our next caller */

    /*
     * Decrement the usage count, or else once you opened the file, you'll
     * never get get rid of the module.
     */
    module_put(THIS_MODULE);
    printk(KERN_INFO "RELEASE\n");
    return 0;
}

/*
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
                           char *buffer,	/* buffer to fill with data */
                           size_t length,	/* length of the buffer     */
                           loff_t * offset)
{
    /*
     * Number of bytes actually written to the buffer
     */
    int bytes_to_read = length;

    /*
     * If we're at the end of the message,
     * return 0 -> end of file
     */
    if (*msg_Ptr == 0)
        return 0;

    /* Make sure we don't read more chars than
     * those remaining to read
     */
    if (bytes_to_read > strlen(msg_Ptr))
        bytes_to_read=strlen(msg_Ptr);

    /*
     * Actually transfer the data onto the userspace buffer.
     * For this task we use copy_to_user() due to security issues
     */
    if (copy_to_user(buffer,msg_Ptr,bytes_to_read))
        return -EFAULT;

    /* Update the pointer for the next read operation */
    msg_Ptr+=bytes_to_read;

    /*
     * The read operation returns the actual number of bytes
     * we copied  in the user's buffer
     */
    return bytes_to_read;
}

/*
 * Called when a process writes to dev file: echo "hi" > /dev/chardev
 */
// Recibimos del usuario, un array de numeros que no accedemos directamente
static ssize_t
device_write(struct file *filp, /* see include/linux/fs.h   */ // descriptor de fichero
            const char *buff, /* buffer with data to write */
            size_t len, /* length of the buffer     */
            loff_t * off)/* desplazamiento(0)   */
{
    printk(KERN_INFO "Estamos en el write\n");

	char kbuf[len]; // Trabajamos sobre este
    int caps_lock=0,  num_lock= 0,scroll_lock=0;  // empiezan apagados
	int bytes_to_write = len;
    int ledsMask = 0x0;
    int val;
    int i;	
	

    //Siempre se ha de trabajar con una copia privada de los datos en espacio de kernel
    //sustituido por kbuf

    // Por ese motivo debemos copiar de forma segura los bytes de buff a un array auxiliar (variable local) usando copy_from_user()

	if (copy_from_user(kbuf,buff,bytes_to_write) != 0) {
		printk(KERN_ALERT "Failed copy_from_user\n"); 
		return -EFAULT;
	} 
	//Recorremos el mensaje entero para ver que leds hay que encender
    //Asignamos valores segun el guion : bit 0 scroll, bit 1 num, bit 2 caps, bit(2,1,0) -> (2,1,3)
	for (i = 0; i < bytes_to_write; i++) {
		val = kbuf[i];
		if 		(val == '1') num_lock 	= 1; 
		else if (val == '2') caps_lock 	= 1;
		else if (val == '3') scroll_lock = 1;
	}
	
	
	// Encendemos valores segun la tabla
	if (scroll_lock == 1)       
        ledsMask = (ledsMask | 0x1); // 001 = 1<<0
	if (num_lock== 1)     
        ledsMask = (ledsMask | 0x2); // 010 = 1<<1
	if (caps_lock  == 1)   
        ledsMask = (ledsMask | 0x4); // 100 = 1<<2

	kbd_driver= get_kbd_driver_handler();
    set_leds(kbd_driver, ledsMask);
        
	return bytes_to_write;
}