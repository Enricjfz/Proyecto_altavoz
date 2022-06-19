#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/kfifo.h>
#include <linux/ioctl.h>
#include <asm/uaccess.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include "spkr-io.h"



MODULE_LICENSE("Dual BSD/GPL");


dev_t devID;
unsigned int firstminor, count = 1;
char *name = "spkr";
struct cdev c;
const char *name_class = "speaker";
const char *fmt = "intspkr";
struct class * module_class;
struct mutex open;
struct mutex write;
spinlock_t lock;
int write_proc = 0;
// declaración de una variable dentro de una estructura
struct info_dispo {
		 wait_queue_head_t lista_bloq;
} info;

typedef struct {
  char sonido [4];
  int datos_copiados;
} PrivateData;

/*
struct timer_list {
        unsigned long expires;
        void (*function)(unsigned long);
        unsigned long data;
};
*/
struct timer_list tl;

static int condition = 0;

module_param(firstminor, int, S_IRUGO);

static int seq_open(struct inode *inode, struct file *filp) {
  /*
     El open puede ser accedido concurrentemente, se guarda con mutex.
		 Si un proceso lo abre en modo escritura, estando ya en modo escritura por otro usuario -> error
		 Si un proceso lo abre en modo lectura, estando ya en modo escritura por otro usuario -> no pasa nada
	*/
    if(filp->f_mode & FMODE_WRITE) {
		if(write_proc == 1)
		{
	         printk(KERN_ALERT "-EBUSY\n");
			return -EBUSY;
		}
		else {
			mutex_lock(&open);
            write_proc = 1;
			printk(KERN_ALERT "PROCESO ESCRITURA\n");
		}
	}
	filp->private_data = kmalloc(sizeof(PrivateData), GFP_KERNEL);
    ((PrivateData *)(filp->private_data))->datos_copiados = 0;
    return 0;
}

static int seq_release(struct inode *inode, struct file *filp) {
    if(filp -> f_mode & FMODE_WRITE)
		{
		//se permite escribir a otro proceso
		 write_proc = 0;	
		 mutex_unlock(&open); //liberamos el mutex
		}
		kfree(filp->private_data);
		printk(KERN_ALERT "seq_release\n");
		return 0;
}

//metodo que desbloquea a todos los procesos dormidos por la escritura del sonido
static void unlockProc (struct  timer_list *tl){
   spin_lock_bh(&lock);
   condition = 1;
   wake_up_interruptible(&info.lista_bloq);
   spkr_off();
   spin_unlock_bh(&lock);
} 

//Funcion auxiliar que programa el temporizador y duerme al proceso
static int scheduleSound (char sonido []){
	int block;
	int freq, dur;
    freq = ((uint16_t) (sonido[1] << 8)) | (uint16_t) sonido[0];
    dur = (((uint16_t) sonido[3] << 8)) | (uint16_t) sonido[2];
	printk(KERN_ALERT "Sonido, duracion: %d frequencia: %d\n",dur,freq);
	spin_lock_bh(&lock);
	if(freq > 0)
	{
	   set_spkr_frequency(freq);
	   spkr_on();
	}
    tl.expires =  jiffies + msecs_to_jiffies(dur);
	timer_setup(&tl, unlockProc,0);
    add_timer(&tl);
	condition = 0;
    block = wait_event_interruptible(info.lista_bloq, condition != 0);
	spin_unlock_bh(&lock);
	if (block != 0){
		printk(KERN_ALERT "-ERESTARTSYS\n");
		return -ERESTARTSYS;
	} 
	return 1;
} 

static ssize_t seq_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
	  printk(KERN_ALERT "seq_write\n"); //Depurar
		// buff argument to the read and write methods is a user-space pointer
		// count is the number size of data to transfer

		int copy; //datos copiados
		int datos_a_escribir; //bytes que se escriben
		PrivateData *privateData = (PrivateData *)(filp -> private_data); //struct de sonido
		int datos_no_copiados;
		mutex_lock(&write); //exclusion mutua threads
		copy = 0;
		printk(KERN_ALERT "SONIDO VACIO %s\n",privateData->sonido);
		while (count > copy) //mientras haya mas datos en el buffer de los escritos
		{
            if(privateData->datos_copiados != 0)
			{
              //hay bytes previos copiados
			  int bytes_sonidos = count - copy;
			  if(bytes_sonidos + privateData->datos_copiados > 4) {
				//los bytes del buffer del usuario y los guardados suman mas de un sonido
				datos_a_escribir = 4 - privateData->datos_copiados;

			  }
			  else {
               //hay menos de un sonido para escribir
				datos_a_escribir = bytes_sonidos;
			  }

			}
			else {
				//no hay bytes previos guardados
				int bytes_sonidos = count - copy;
				if(bytes_sonidos >= 4){
					//hay mas de un sonido
					datos_a_escribir = 4;
					printk(KERN_ALERT "Escribes 4 bytes de golpe\n");
				}
				else {
					//hay menos de un sonido
					datos_a_escribir = count - copy;
				}
			}
		    datos_no_copiados = copy_from_user(privateData->sonido + privateData->datos_copiados, buf, datos_a_escribir); //se copia del buffer de usuario
			if (datos_no_copiados > 0)
			{
                     return -EFAULT;
			}
			copy += datos_a_escribir;
			privateData->datos_copiados += datos_a_escribir;
			if(privateData->datos_copiados == 4) //hay un sonido disponible
			{
				printk(KERN_ALERT "se escribe en el dispositivo 4 bytes \n");
				printk(KERN_ALERT "SONIDO: %s\n",privateData->sonido);
                //se llama a función sonido y se bloquea proceso
				scheduleSound(privateData->sonido);
				privateData->datos_copiados = 0;
				

			}
		}
		mutex_unlock(&write);
        return 0;
}

static ssize_t seq_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
	  printk(KERN_ALERT "seq_read --no hace nada --\n");
    return 0;
}

static struct file_operations fops = {
        .owner =    THIS_MODULE,
        .open =     seq_open,
        .release =  seq_release,
        .write =    seq_write,
        .read =     seq_read
};

static int __init spkr_init(void) {

  //reserva de major y minor
	if(alloc_chrdev_region(&devID,firstminor,count,name) < 0)
	{
    return -1;
	}
  printk(KERN_ALERT "salida_chrdev\n");
  //creación del dispositivo
	cdev_init(&c, &fops);

  //asociarla con el identificador de dispositivo reservado previamente
	if(cdev_add(&c,devID,count) < 0)
	{
		return -1;
	}

  //creación del fichero del dispositivo
  module_class = class_create (THIS_MODULE,name_class);
  //dar de alta al dispositivo asociandola a la clase
  device_create(module_class,NULL,devID,NULL,fmt);
  //se inicializan las colas, mutex del dispositivo
	init_waitqueue_head(&info.lista_bloq);
	mutex_init(&open);
	mutex_init(&write);
	spin_lock_init(&lock);

	return 0;
}


static void __exit spkr_exit(void) {
  device_destroy(module_class,devID);
  class_destroy(module_class);
  cdev_del(&c);
  unregister_chrdev_region(devID,count);
  mutex_destroy(&open);
  mutex_destroy(&write);
  printk(KERN_ALERT "salida_exit\n");
}

module_init(spkr_init);
module_exit(spkr_exit);
