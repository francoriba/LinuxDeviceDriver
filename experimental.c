#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>

#define EchoPin 18
#define FILE_PATH "/dev/gpio_read"

static dev_t first;
static struct cdev c_dev;
static struct class *cl;
const int v_sonido = 34300;

static void set_gpio_input(unsigned int gpio)
{
    unsigned int reg = gpio / 10;
    unsigned int shift = (gpio % 10) * 3;

    unsigned int* addr = ioremap(0x3F200000 + reg, 4);
    unsigned int value = ioread32(addr);
    value &= ~(7 << shift);
    iowrite32(value, addr);
    iounmap(addr);
}

static int read_sensor_data(void)
{
    struct timeval inicio_pulso, fin_pulso;

    while (gpio_get_value(EchoPin) == 0)
    {
        gettimeofday(&inicio_pulso, NULL);
    }

    while (gpio_get_value(EchoPin) == 1)
    {
        gettimeofday(&fin_pulso, NULL);
    }

    double tiempo = (fin_pulso.tv_sec - inicio_pulso.tv_sec) + (fin_pulso.tv_usec - inicio_pulso.tv_usec) / 1000000.0;
    double distancia = v_sonido * (tiempo / 2);

    char buffer[50];
    int len = snprintf(buffer, sizeof(buffer), "Distancia: %.2f cm\n", distancia);

    int file = open(FILE_PATH, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (file < 0)
    {
        printk(KERN_ALERT "Error al abrir el archivo\n");
        return -1;
    }

    ssize_t written = write(file, buffer, len);

    close(file);

    return written;
}

static int my_open(struct inode *i, struct file *f)
{
    printk(KERN_INFO "cdd_sensado: open()\n");
    return 0;
}

static int my_close(struct inode *i, struct file *f)
{
    printk(KERN_INFO "cdd_sensado: close()\n");
    return 0;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "cdd_sensado: read()\n");

    int bytes_written = read_sensor_data();

    if (bytes_written <= 0)
        return bytes_written;

    if (*off >= bytes_written)
        return 0;

    if (len > bytes_written - *off)
        len = bytes_written - *off;

    if (copy_to_user(buf, "OK\n", 3) != 0)
        return -EFAULT;

    *off += len;

    return len;
}

static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "cdd_sensado: write()\n");
    return len;
}

static struct file_operations pugs_fops =
{
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .write = my_write
};

static int __init cdd_init(void)
{
    int ret;
    struct device *dev_ret;

    printk(KERN_INFO "cdd_sensado: registrado exitosamente!\n");

    if ((ret = alloc_chrdev_region(&first, 0, 2, "cdf_internautas")) < 0)
    {
        return ret;
    }

    if (IS_ERR(cl = class_create(THIS_MODULE, "chardrv")))
    {
        unregister_chrdev_region(first, 1);
        return PTR_ERR(cl);
    }

    if (IS_ERR(dev_ret = device_create(cl, NULL, first, NULL, "gpio_read")))
    {
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return PTR_ERR(dev_ret);
    }

    cdev_init(&c_dev, &pugs_fops);
    if ((ret = cdev_add(&c_dev, first, 1)) < 0)
    {
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return ret;
    }
    printk(KERN_INFO "<Major, Minor>: <%d, %d>\n", MAJOR(first), MINOR(first));

    // Configurar pin GPIO como entrada
    set_gpio_input(EchoPin);

    printk(KERN_INFO "Medición de la distancia en curso\n");

    return 0;
}

static void __exit cdd_exit(void)
{
    cdev_del(&c_dev);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    printk(KERN_INFO "cdd_sensado: descargado del kernel!\n");
}

module_init(cdd_init);
module_exit(cdd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Internautas");
MODULE_DESCRIPTION("Character device driver for reading of 2 signals from a Raspberry Pi 400");
