#include <linux/types.h>  
#include <linux/module.h>  
#include <linux/input.h>  
#include <linux/timer.h>  
#include <linux/interrupt.h>  
#include <linux/gpio.h>  
  
static struct input_dev *buttons_dev;  
static struct timer_list timer;  
struct button_desc* button_desc = NULL;  
  
struct button_desc{  
    char* name;  
    unsigned int pin;  
    unsigned int irq;  
    unsigned int val;  
};

/*
static unsigned int keys_pin[] = {
	S5PV210_GPH0(0),
	S5PV210_GPH0(1),
	S5PV210_GPH0(2),
	S5PV210_GPH0(3),
	S5PV210_GPH0(4),
	S5PV210_GPH0(5),
	S5PV210_GPH2(6),
	S5PV210_GPH2(7),
};
static unsigned int keys_irq[] = {};*/
  
static struct button_desc buttons_desc[8] = {  
    [0] = {  
        .name = "S1",  
        .pin = S5PV210_GPH0(0),
        .irq = IRQ_EINT(0),  
        .val = KEY_L,  
    },  
  
    [1] = {  
        .name = "S2",  
        .pin = S5PV210_GPH0(1),  
        .irq = IRQ_EINT(1),  
        .val = KEY_S,  
    },  
  
    [2] = {  
        .name = "S3",  
        .pin = S5PV210_GPH0(2),  
        .irq = IRQ_EINT(2),  
        .val = KEY_C,  
    },  
  
    [3] = {  
        .name = "S4",  
        .pin = S5PV210_GPH0(3),  
        .irq = IRQ_EINT(3),  
        .val = KEY_ENTER,  
    },  
  
    [4] = {  
        .name = "S5",  
        .pin = S5PV210_GPH0(4),  
        .irq = IRQ_EINT(4),  
        .val = KEY_LEFTCTRL,  
    },  
  
    [5] = {  
        .name = "S6",  
        .pin = S5PV210_GPH0(5),  
        .irq = IRQ_EINT(5),  
        .val = KEY_MINUS,  
    },  
  
    [6] = {  
        .name = "S7",  
        .pin = S5PV210_GPH2(6),  
        .irq = IRQ_EINT(22),  
        .val = KEY_CAPSLOCK,  
    },  
  
    [7] = {  
        .name = "S8",  
        .pin = S5PV210_GPH2(7),  
        .irq = IRQ_EINT(23),  
        .val = KEY_SPACE,  
    },  
};  
  
static void timer_function(unsigned long data){  
    if(button_desc == NULL)  
        return;  
  
    if(gpio_get_value(button_desc->pin)){  
        input_event(buttons_dev, EV_KEY, button_desc->val, 0);  
    }  
    else{  
        input_event(buttons_dev, EV_KEY, button_desc->val, 1);  
    }  
    input_sync(buttons_dev);  
}  
  
static irqreturn_t irq_handler(int irq, void *devid){  
    button_desc = (struct button_desc*)devid;  
    mod_timer(&timer, jiffies + HZ/100);  
    return IRQ_RETVAL(IRQ_HANDLED);  
}  
  
static int buttons_init(void){  
    int i;  
      
    buttons_dev = input_allocate_device();  
    if(buttons_dev == NULL){  
        printk(KERN_ERR "Error: allocate input device failed!\n");  
        return -ENOMEM;  
    }  
  
    __set_bit(EV_KEY, buttons_dev->evbit);  
    __set_bit(EV_REP, buttons_dev->evbit);  
  
    __set_bit(KEY_L,        buttons_dev->keybit);  
    __set_bit(KEY_S,        buttons_dev->keybit);  
    __set_bit(KEY_C,        buttons_dev->keybit);  
    __set_bit(KEY_SPACE,    buttons_dev->keybit);  
    __set_bit(KEY_MINUS,    buttons_dev->keybit);  
    __set_bit(KEY_ENTER,    buttons_dev->keybit);  
    __set_bit(KEY_LEFTCTRL, buttons_dev->keybit);  
    __set_bit(KEY_CAPSLOCK, buttons_dev->keybit);  
  
    printk("1\n");  
    if(input_register_device(buttons_dev)){  
        goto error_1;  
    }  
  
    printk("2\n");  
    init_timer(&timer);  
    timer.function = timer_function;  
    add_timer(&timer);  
  
    printk("3\n");  
    for(i = 0; i != 8; ++i){  
        if(request_irq(buttons_desc[i].irq, irq_handler,   
            IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, buttons_desc[i].name, &buttons_desc[i])){  
            goto error_2;  
        }  
    }  
    printk("4\n");  
      
    return 0;  
  
error_2:  
    for(--i; i >= 0; --i){  
        free_irq(buttons_desc[i].irq, &buttons_desc[i]);  
    }  
    input_unregister_device(buttons_dev);  
  
error_1:  
    input_free_device(buttons_dev);  
  
    return -EBUSY;  
}  
  
static void buttons_exit(void){  
    int i;  
    for(i = 0; i != 8; ++i){  
        free_irq(buttons_desc[i].irq, &buttons_desc[i]);  
    }  
  
    input_unregister_device(buttons_dev);  
    input_free_device(buttons_dev);  
}  
  
module_init(buttons_init);  
module_exit(buttons_exit);  
MODULE_LICENSE("GPL");



