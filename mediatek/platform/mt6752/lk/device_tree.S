	.section .rodata
	.globl device_tree
	.globl device_tree_size
	.align 4
device_tree:
	.incbin "platform/mediatek/mt6752/lk/mt6752_fpga.dtb"

	.align 4
device_tree_size:
	.long device_tree_size - device_tree
