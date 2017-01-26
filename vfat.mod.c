#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x1da0d6ca, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x7b99eb2f, __VMLINUX_SYMBOL_STR(fat_detach) },
	{ 0xfe6823e6, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x405c1144, __VMLINUX_SYMBOL_STR(get_seconds) },
	{ 0x11e0e337, __VMLINUX_SYMBOL_STR(drop_nlink) },
	{ 0xa675804c, __VMLINUX_SYMBOL_STR(utf8s_to_utf16s) },
	{ 0x343ff613, __VMLINUX_SYMBOL_STR(mark_buffer_dirty_inode) },
	{ 0xd6fbe4fa, __VMLINUX_SYMBOL_STR(__mark_inode_dirty) },
	{ 0xadaabe1b, __VMLINUX_SYMBOL_STR(pv_lock_ops) },
	{ 0xe69f429b, __VMLINUX_SYMBOL_STR(dput) },
	{ 0xcee11ccc, __VMLINUX_SYMBOL_STR(inc_nlink) },
	{ 0x8b486917, __VMLINUX_SYMBOL_STR(d_find_alias) },
	{ 0xb1bf412a, __VMLINUX_SYMBOL_STR(names_cachep) },
	{ 0xe48e40c2, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0x71fa22b6, __VMLINUX_SYMBOL_STR(mount_bdev) },
	{ 0xa973be29, __VMLINUX_SYMBOL_STR(fat_sync_inode) },
	{ 0x5f044076, __VMLINUX_SYMBOL_STR(fat_add_entries) },
	{ 0x7d11c268, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0x4717aed7, __VMLINUX_SYMBOL_STR(fat_remove_entries) },
	{ 0xf65d7f98, __VMLINUX_SYMBOL_STR(fat_alloc_new_dir) },
	{ 0x4502602a, __VMLINUX_SYMBOL_STR(fat_fill_super) },
	{ 0x19ed1f80, __VMLINUX_SYMBOL_STR(fat_build_inode) },
	{ 0x11089ac7, __VMLINUX_SYMBOL_STR(_ctype) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x604fe828, __VMLINUX_SYMBOL_STR(fat_attach) },
	{ 0xa1c4c12, __VMLINUX_SYMBOL_STR(d_move) },
	{ 0x5a921311, __VMLINUX_SYMBOL_STR(strncmp) },
	{ 0x626ac375, __VMLINUX_SYMBOL_STR(kmem_cache_free) },
	{ 0xf676ffba, __VMLINUX_SYMBOL_STR(mutex_lock) },
	{ 0xa7190c66, __VMLINUX_SYMBOL_STR(set_nlink) },
	{ 0xeeb684a5, __VMLINUX_SYMBOL_STR(fat_update_super) },
	{ 0x29009cbb, __VMLINUX_SYMBOL_STR(sync_dirty_buffer) },
	{ 0xe2338745, __VMLINUX_SYMBOL_STR(fat_getattr) },
	{ 0x4aa4713, __VMLINUX_SYMBOL_STR(__brelse) },
	{ 0x5590a034, __VMLINUX_SYMBOL_STR(kmem_cache_alloc) },
	{ 0xf0fdf6cb, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0xfc77c68f, __VMLINUX_SYMBOL_STR(kill_block_super) },
	{ 0x359df40c, __VMLINUX_SYMBOL_STR(fat_search_long) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0xb01d8a79, __VMLINUX_SYMBOL_STR(fat_config_init) },
	{ 0x6f20960a, __VMLINUX_SYMBOL_STR(full_name_hash) },
	{ 0x721679aa, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xd52bf1ce, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0xee6ba18e, __VMLINUX_SYMBOL_STR(fat_scan) },
	{ 0xcc6b3a62, __VMLINUX_SYMBOL_STR(register_filesystem) },
	{ 0x3bf18c78, __VMLINUX_SYMBOL_STR(__fat_fs_error) },
	{ 0x1cd1ba02, __VMLINUX_SYMBOL_STR(iput) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x69acdf38, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0xa03164af, __VMLINUX_SYMBOL_STR(d_splice_alias) },
	{ 0x6cba5ba3, __VMLINUX_SYMBOL_STR(fat_setattr) },
	{ 0xc4b795b4, __VMLINUX_SYMBOL_STR(fat_free_clusters) },
	{ 0x3a943293, __VMLINUX_SYMBOL_STR(fat_get_dotdot_entry) },
	{ 0xe6366e14, __VMLINUX_SYMBOL_STR(unregister_filesystem) },
	{ 0x7bfbb668, __VMLINUX_SYMBOL_STR(fat_time_unix2fat) },
	{ 0x28318305, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0xc60e0af5, __VMLINUX_SYMBOL_STR(fat_dir_empty) },
	{ 0xcc709d8a, __VMLINUX_SYMBOL_STR(d_instantiate) },
	{ 0xea06e503, __VMLINUX_SYMBOL_STR(clear_nlink) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=fat";


MODULE_INFO(srcversion, "7429164BDB322D1A8BB19D0");
