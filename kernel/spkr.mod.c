#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xeeab4c1e, "module_layout" },
	{ 0x3c03d27a, "cdev_del" },
	{ 0x6aa9e858, "kmalloc_caches" },
	{ 0xdc74c97c, "cdev_init" },
	{ 0x93c68c79, "param_ops_int" },
	{ 0xc498609c, "device_destroy" },
	{ 0x409bcb62, "mutex_unlock" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0x977f511b, "__mutex_init" },
	{ 0xc5850110, "printk" },
	{ 0x2ab7989d, "mutex_lock" },
	{ 0xba9cf391, "device_create" },
	{ 0x44d7ad45, "cdev_add" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xd60e87c0, "kmem_cache_alloc_trace" },
	{ 0x7c038979, "class_destroy" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0xbd59df1c, "__class_create" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "58F5EBD0799DE142E57AAFE");
