include		"arch/armv7/conf/generic.exynos"
config		bsd	root on sd0a swap on sd0b

option	MULTIPROCESSOR
cpu*		at mainbus?
