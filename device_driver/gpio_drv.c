#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>


#define DEVICE_NAME "gpio_device"
#define BUFFER_SIZE 256

static dev_t first;       // Vble global para device number "first"
static struct cdev c_dev; // Global variable for the character device structure
static struct class *cl;  // Global variable for the device class

static char message[BUFFER_SIZE] = {0};
static short message_size;
static int gpio_pin = 24;  // echo 23
static int gpio_pin2 = 16;  // echo 15 

// Función de apertura del dispositivo
static int device_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Dispositivo abierto.\n");
    return 0;
}

// Función de liberación del dispositivo
static int device_release(struct inode *inode, struct file *file){
    printk(KERN_INFO "Dispositivo cerrado.\n");
    return 0;
}

// Función de lectura del dispositivo
static ssize_t device_read(struct file *file, char *buffer, size_t length, loff_t *offset)
{
    int gpio_value;
    char gpio_value_str[2];

    memset(message, 0, sizeof(message));  // Limpieza del búfer message

    gpio_value = gpio_get_value(gpio_pin);
    sprintf(gpio_value_str, "%d", gpio_value);
    strcat(message, gpio_value_str);
    strcat(message, "\n");

    message_size = strlen(message);

    if (*offset >= message_size)
        return 0;

    if (length > message_size - *offset)
        length = message_size - *offset;

    if (copy_to_user(buffer, message + *offset, length) != 0)
        return -EFAULT;

    *offset += length;

    return length;
}


// Función de escritura en el dispositivo
static ssize_t device_write(struct file *file, const char *buffer, size_t length, loff_t *offset){
    char command[BUFFER_SIZE];

    if (copy_from_user(command, buffer, length) != 0)
        return -EFAULT;

    if (length > 0) {
        if (strncmp(command, "out", 3) == 0) {
            gpio_direction_output(gpio_pin, 0);  // Configurar como salida
            printk(KERN_INFO "GPIO %d configurado como salida.\n", gpio_pin);
        } else if (strncmp(command, "in", 2) == 0) {
            gpio_direction_input(gpio_pin);  // Configurar como entrada
            printk(KERN_INFO "GPIO %d configurado como entrada.\n", gpio_pin);
        }else if (strncmp(command, "in2", 2) == 0) {
            gpio_direction_input(gpio_pin2);  // Configurar como entrada
            printk(KERN_INFO "GPIO %d configurado como entrada.\n", gpio_pin2);
        }  
        else if (strncmp(command, "toggle", 6) == 0) {
            printk(KERN_INFO "Toggleando GPIO.\n");
            if (gpio_pin == 24) {
                gpio_pin = 16;
                gpio_pin2 = 24;
            } else {
                gpio_pin = 24;
                gpio_pin2 = 16;
            }
            printk(KERN_INFO "GPIO pin cambiado a %d.\n", gpio_pin);
        }
        else if (strncmp(command, "0", 1) == 0) {
            gpio_set_value(gpio_pin, 0);
            printk(KERN_INFO "GPIO %d establecido en bajo.\n", gpio_pin);
        } else if (strncmp(command, "1", 1) == 0) {
            gpio_set_value(gpio_pin, 1);
            printk(KERN_INFO "GPIO %d establecido en alto.\n", gpio_pin);
        } else {
            printk(KERN_INFO "Comando no válido.\n");
        }
    }

    return length;
}

// Estructura de operaciones del dispositivo
static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write
};

// Función de inicialización del módulo
static int __init gpio_device_init(void)
{
    static int ret;
    struct device *dev_ret;

    printk(KERN_INFO "Inicializando módulo de dispositivo GPIO.\n");

    if ((ret = alloc_chrdev_region(&first, 0, 1, DEVICE_NAME)) < 0){
        printk(KERN_ALERT "Error al registrar el número principal de dispositivo.\n");
        return ret;
    }
    
    printk(KERN_INFO "Módulo de dispositivo GPIO registrado con número principal %d.\n", ret);

    if (IS_ERR(cl = class_create(THIS_MODULE, "chardrv"))){
        printk(KERN_ALERT "Error al crear la clase \n");
        unregister_chrdev_region(first, 1);
        return PTR_ERR(cl);
    }

    if (IS_ERR(dev_ret = device_create(cl, NULL, first, NULL, DEVICE_NAME))){
        printk(KERN_ALERT "Error al crear el dispositivo \n");
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return PTR_ERR(dev_ret);
    }

    if (gpio_request(gpio_pin, "GPIO_PIN") < 0) {
        printk(KERN_ALERT "Error al solicitar GPIO %d.\n", gpio_pin);
        unregister_chrdev(ret, DEVICE_NAME);
        return -ENODEV;
    }

    gpio_direction_input(gpio_pin);

    cdev_init(&c_dev, &fops);
    if ((ret = cdev_add(&c_dev, first, 1)) < 0){
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return ret;
    }

    return 0;
}

// Función de limpieza del módulo
static void __exit gpio_device_exit(void){
    cdev_del(&c_dev);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    gpio_unexport(gpio_pin);
    gpio_free(gpio_pin);
    printk(KERN_INFO "Módulo de dispositivo GPIO desregistrado.\n");
}

module_init(gpio_device_init);
module_exit(gpio_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Internautas");
MODULE_DESCRIPTION("Character device driver para control de GPIO en Raspberry Pi 400");