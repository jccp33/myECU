	.cpu cortex-m3
	.arch armv7-m
	.fpu softvfp
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 1
	.eabi_attribute 30, 4
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.file	"myecu_sizes.cpp"
	.text
	.section	.rodata
	.align	2
	.type	_ZL17SIZE_FAULT_RECORD, %object
	.size	_ZL17SIZE_FAULT_RECORD, 4
_ZL17SIZE_FAULT_RECORD:
	.word	16
	.type	_ZL18SIZE_SIGNAL_SAMPLE, %object
	.size	_ZL18SIZE_SIGNAL_SAMPLE, 4
_ZL18SIZE_SIGNAL_SAMPLE:
	.word	32
	.type	_ZL12SIZE_CONTROL, %object
	.size	_ZL12SIZE_CONTROL, 4
_ZL12SIZE_CONTROL:
	.word	1
	.type	_ZL18SIZE_FAULT_MANAGER, %object
	.size	_ZL18SIZE_FAULT_MANAGER, 4
_ZL18SIZE_FAULT_MANAGER:
	.word	6176
	.type	_ZL17SIZE_SIGNAL_STORE, %object
	.size	_ZL17SIZE_SIGNAL_STORE, 4
_ZL17SIZE_SIGNAL_STORE:
	.word	4104
	.type	_ZL18SIZE_SYSTEM_CONFIG, %object
	.size	_ZL18SIZE_SYSTEM_CONFIG, 4
_ZL18SIZE_SYSTEM_CONFIG:
	.word	7176
	.ident	"GCC: (15:13.2.rel1-2) 13.2.1 20231009"
