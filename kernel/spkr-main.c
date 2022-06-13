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
// declaración de una variable dentro de una estructura
struct info_dispo {
		 wait_queue_head_t lista_bloq;
} info;

typedef struct {
  char sonido [4];
	int datos_copiados;
} PrivateData;

module_param(firstminor, int, S_IRUGO);

static int seq_open(struct inode *inode, struct file *filp) {
  /*
     El open puede ser accedido concurrentemente, se guarda con mutex.
		 Si un proceso lo abre en modo escritura, estando ya en modo escritura por otro usuario -> error
		 Si un proceso lo abre en modo lectura, estando ya en modo escritura por otro usuario -> no pasa nada
	*/
    if(filp->f_mode & FMODE_WRITE)
		{
        mutex_lock(&open);
        printk(KERN_ALERT "-EBUSY\n");
				return -EBUSY;
		}
		filp->private_data = kmalloc(sizeof(PrivateData), GFP_KERNEL);
    ((PrivateData *)(filp->private_data))->datos_copiados = 0;
    return 0;
}

static int seq_release(struct inode *inode, struct file *filp) {
    if(filp -> f_mode & FMODE_WRITE)
		{
			mutex_unlock(&open); //liberamos el mutex
		}
		printk(KERN_ALERT "seq_release\n");
		return 0;
}
static ssize_t seq_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
	  printk(KERN_ALERT "seq_write\n"); //Depurar
		// buff argument to the read and write methods is a user-space pointer
		// count is the number size of data to transfer
    //inicializamos el buffer donde se va a escribir en el kernel
		/*
      Cada 4 bytes que se escriben en el dispositivo definen un sonido:
			los dos primeros bytes especifican la frecuencia, mientras que los dos siguientes determinan la duración
			de ese sonido en milisegundos. Completada esa duración, el altavoz se desactivará, a no ser que haya más
			sonidos pendientes de generarse formando parte del mismo write. Un sonido con un valor de frecuencia de 0
			corresponderá a desactivar el altavoz durante la duración indicada (se trata de un silencio o pausa en la
			secuencia de sonidos). Por otro lado, dado que un sonido con una duración de 0 no tendrá ningún efecto,
			se podrá implementar como se considere oportuno siempre que tenga ese comportamiento nulo.
			Asimismo, téngase en cuenta que una única operación de escritura puede incluir numerosos sonidos
			(así, por ejemplo, un write de 4KiB incluiría 1024 sonidos), que se ejecutarán como una secuencia.
			Nótese que si el manejador recibe en una escritura un número de bytes que no es múltiplo de 4,
			habrá un sonido incompleto final, que habrá que guardar hasta que una escritura posterior lo complete.
		*/
		int copy;
		size_t datos_a_escribir;
		PrivateData *privateData = (PrivateData *)(filp -> private_data);
		int datos_no_copiados;
		mutex_lock(&write);
		copy = 0;
		while (count > copy)
		{
      datos_a_escribir = privateData -> datos_copiados%4;
      if((count - copy) < datos_a_escribir)
			{
        datos_a_escribir = count - copy;

			}
		  datos_no_copiados = copy_from_user(privateData->sonido + privateData->datos_copiados, buf, datos_a_escribir);
			if (datos_no_copiados > 0)
			{
         return -EFAULT;
			}
			copy += datos_a_escribir;
			privateData->datos_copiados += datos_a_escribir;
			if(privateData->datos_copiados%4 == 0)
			{
				privateData->datos_copiados = 0;
				//se llama a función sonido
			}
		}
		mutex_unlock(&write);
    return 0;
}

static ssize_t seq_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
	  printk(KERN_ALERT "seq_read\n");
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
	init_waitqueue_head(&info.lista_bloq);
	mutex_init(&open);
	mutex_init(&write);

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
