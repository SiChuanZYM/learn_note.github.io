// 从u-boot.lds分析：
ENTRY(_start)
//起始地址不在lds中，经查找，在vectors.S中：

_start:

#ifdef CONFIG_SYS_DV_NOR_BOOT_CFG
	.word	CONFIG_SYS_DV_NOR_BOOT_CFG
#endif

	b	reset
        -> start.S中:
        -> reset:
	    b	save_boot_params
            ->ENTRY(save_boot_params)
	            b	save_boot_params_ret		@ back to my caller
            ENDPROC(save_boot_params)
	            .weak	save_boot_params
            // ENTRT定义函数入口，ENDPROC定义函数结束点。共同组成，有头有尾。
            // 同时.weak 弱定义，方便其他覆盖
            ->save_boot_params_ret:
                ->save_boot_params_ret函数段
                bl	cpu_init_cp15
                    ->设置CP15寄存器：缓存、MMU、TLB和可能设置I-cache
	            bl	cpu_init_crit
                    -> b lowlevel_init
                        -> lowlevel_init.S文件
                        	lowlevel_init
							ldr	sp, =CONFIG_SYS_INIT_SP_ADDR
							->bl s_init
								soc.c
								->return
                bl _main
					-> bl board_init_f_alloc_reserve
						board.init.c文件
						board_init_f_alloc_reserve
						malloc空间，预留CONFIG_SYS_MALLOC_F_LEN和global_data空间，并16字节对齐

					-> bl board_init_f_init_reserve
						board.init.c文件
						board_init_f_init_reserve
						-> 初始化global_data全局数据结构体，计算下次内存分配的起始地址
						且，此时r9保存这global_data的地址 0x0091fa00
					-> bl board_init_f
						board_f.c文件中
						
						


	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq

	.globl	_undefined_instruction
	.globl	_software_interrupt
	.globl	_prefetch_abort
	.globl	_data_abort
	.globl	_not_used
	.globl	_irq
	.globl	_fiq

// PC寄存器是CPU中的一个特殊寄存器，也是CPU中最重要的寄存器之一。
// 它存储的是指令的地址，CPU在执行指令时会不断地从PC寄存器中读取下一条指令的地址。
// 在执行完当前指令后，CPU会自动将PC寄存器的值加上当前指令的长度，
// 以便于执行下一条指令。




// reset函数段
reset:
	/* Allow the board to save important registers */
	b	save_boot_params
save_boot_params_ret:

	mrs	r0, cpsr
	and	r1, r0, #0x1f		@ mask mode bits
	teq	r1, #0x1a		@ test for HYP mode
	bicne	r0, r0, #0x1f		@ clear all mode bits
	orrne	r0, r0, #0x13		@ set SVC mode
	orr	r0, r0, #0xc0		@ disable FIQ and IRQ
	msr	cpsr,r0

    // cpsr寄存器 当前程序状态寄存器设置为SVC模式
    // 1. mrs r0, cpsr (特殊)将CPSR的值移动到r0
    // 2. and r1, r0, #0x1f 按位与运算，将r0与0x1f与运算后保存在r1. 获得cpsr模式位
    // 3. teq r1, #0x1a 将r1与0x1a判断相等性，作为下面的条件，判断是否为HYP模式
    // 4. bicne r0, r0, #0x1f ne为上面的结果notequal，0x1f按位取反后，与上r0，再保存进r0
    //   等于清零模式位的0~4
    // 5. orrne r0, r0, #0x13，将r0与0x13按位或，再写回r0，即设置为Supervisor(SVC)模式
    // 6. orr r0, r0, #0xc0， 将bit6，7置1，禁止IRQ,FIQ
    // 7. msr cpsr, r0 将r0 move回cpsr

/*
 * Setup vector:
 * (OMAP4 spl TEXT_BASE is not 32 byte aligned.
 * Continue to use ROM code vector only in OMAP4 spl)
 */
// 这一块应该没执行
// 设置中断中断向量表 
#if !(defined(CONFIG_OMAP44XX) && defined(CONFIG_SPL_BUILD))
	/* Set V=0 in CP15 SCTLR register - for VBAR to point to vector */
	mrc	p15, 0, r0, c1, c0, 0	@ Read CP15 SCTLR Register
	bic	r0, #CR_V		@ V = 0
	mcr	p15, 0, r0, c1, c0, 0	@ Write CP15 SCTLR Register

	/* Set vector address in CP15 VBAR register */
	ldr	r0, =_start
	mcr	p15, 0, r0, c12, c0, 0	@Set VBAR
#endif

// 这一块应该没执行
	/* the mask ROM code should have PLL and others stable */
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	bl	cpu_init_cp15
	bl	cpu_init_crit
#endif

	bl	_main








ENTRY(lowlevel_init)
	/*
	 * Setup a temporary stack. Global data is not available yet.
	 */
	ldr	sp, =CONFIG_SYS_INIT_SP_ADDR
		// mx6ul_14x14_evk.h中有解释
	bic	sp, sp, #7 /* 8-byte alignment for ABI compliance */
#ifdef CONFIG_SPL_DM
	mov	r9, #0
#else
	/*
	 * Set up global data for boards that still need it. This will be
	 * removed soon.
	 */
#ifdef CONFIG_SPL_BUILD
	ldr	r9, =gdata
#else
	sub	sp, sp, #GD_SIZE
	bic	sp, sp, #7
	mov	r9, sp
#endif
#endif
	/*
	 * Save the old lr(passed in ip) and the current lr to stack
	 */
	push	{ip, lr}

	/*
	 * Call the very early init function. This should do only the
	 * absolute bare minimum to get started. It should not:
	 *
	 * - set up DRAM
	 * - use global_data
	 * - clear BSS
	 * - try to start a console
	 *
	 * For boards with SPL this should be empty since SPL can do all of
	 * this init in the SPL board_init_f() function which is called
	 * immediately after this.
	 */
	bl	s_init
	pop	{ip, pc}
ENDPROC(lowlevel_init)



ldr	sp, =CONFIG_SYS_INIT_SP_ADDR
		=CONFIG_SYS_INIT_RAM_ADDR(0x00900000 ) + CONFIG_SYS_INIT_SP_OFFSET
		=IRAM_BASE_ADDR + CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE
		=IRAM_BASE_ADDR + IRAM_SIZE - 256
		=0x00900000 + 0x00020000 - 0x100
		=0x0091ff00
		属于了SYS RAM ADDR以内


// 直接return了
void s_init(void)
{
	struct anatop_regs *anatop = (struct anatop_regs *)ANATOP_BASE_ADDR;
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	u32 mask480;
	u32 mask528;
	u32 reg, periph1, periph2;

	if (is_cpu_type(MXC_CPU_MX6SX) || is_cpu_type(MXC_CPU_MX6UL) ||
	    is_cpu_type(MXC_CPU_MX6ULL) || is_cpu_type(MXC_CPU_MX6SLL))
		return;

	/* Due to hardware limitation, on MX6Q we need to gate/ungate all PFDs
	 * to make sure PFD is working right, otherwise, PFDs may
	 * not output clock after reset, MX6DL and MX6SL have added 396M pfd
	 * workaround in ROM code, as bus clock need it
	 */

	mask480 = ANATOP_PFD_CLKGATE_MASK(0) |
		ANATOP_PFD_CLKGATE_MASK(1) |
		ANATOP_PFD_CLKGATE_MASK(2) |
		ANATOP_PFD_CLKGATE_MASK(3);
	mask528 = ANATOP_PFD_CLKGATE_MASK(1) |
		ANATOP_PFD_CLKGATE_MASK(3);

	reg = readl(&ccm->cbcmr);
	periph2 = ((reg & MXC_CCM_CBCMR_PRE_PERIPH2_CLK_SEL_MASK)
		>> MXC_CCM_CBCMR_PRE_PERIPH2_CLK_SEL_OFFSET);
	periph1 = ((reg & MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_MASK)
		>> MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_OFFSET);

	/* Checking if PLL2 PFD0 or PLL2 PFD2 is using for periph clock */
	if ((periph2 != 0x2) && (periph1 != 0x2))
		mask528 |= ANATOP_PFD_CLKGATE_MASK(0);

	if ((periph2 != 0x1) && (periph1 != 0x1) &&
		(periph2 != 0x3) && (periph1 != 0x3))
		mask528 |= ANATOP_PFD_CLKGATE_MASK(2);

	writel(mask480, &anatop->pfd_480_set);
	writel(mask528, &anatop->pfd_528_set);
	writel(mask480, &anatop->pfd_480_clr);
	writel(mask528, &anatop->pfd_528_clr);
}








// _main ，删除了不会生效的部分。全文在crt0.S
ENTRY(_main)

/*
 * Set up initial C runtime environment and call board_init_f(0).
 */

	ldr	sp, =(CONFIG_SYS_INIT_SP_ADDR)

	bic	sp, sp, #7	/* 8-byte alignment for ABI compliance */
	
	mov	r0, sp
	bl	board_init_f_alloc_reserve
	// board_init_f_alloc_reserve 参数为 r0，即0x0091ff00
	// 返回值为0x0091FA00
	mov	sp, r0
	/* set up gd here, outside any C code */
	mov	r9, r0
	bl	board_init_f_init_reserve
	
	mov	r0, #0
	bl	board_init_f

#if ! defined(CONFIG_SPL_BUILD)

/*
 * Set up intermediate environment (new sp and gd) and call
 * relocate_code(addr_moni). Trick here is that we'll return
 * 'here' but relocated.
 */

	ldr	sp, [r9, #GD_START_ADDR_SP]	/* sp = gd->start_addr_sp */
#if defined(CONFIG_CPU_V7M)	/* v7M forbids using SP as BIC destination */
	mov	r3, sp
	bic	r3, r3, #7
	mov	sp, r3
#else
	bic	sp, sp, #7	/* 8-byte alignment for ABI compliance */
#endif
	ldr	r9, [r9, #GD_BD]		/* r9 = gd->bd */
	sub	r9, r9, #GD_SIZE		/* new GD is below bd */

	adr	lr, here
	ldr	r0, [r9, #GD_RELOC_OFF]		/* r0 = gd->reloc_off */
	add	lr, lr, r0
#if defined(CONFIG_CPU_V7M)
	orr	lr, #1				/* As required by Thumb-only */
#endif
	ldr	r0, [r9, #GD_RELOCADDR]		/* r0 = gd->relocaddr */
	b	relocate_code
here:
/*
 * now relocate vectors
 */

	bl	relocate_vectors

/* Set up final (full) environment */

	bl	c_runtime_cpu_setup	/* we still call old routine here */
#endif
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_FRAMEWORK)
# ifdef CONFIG_SPL_BUILD
	/* Use a DRAM stack for the rest of SPL, if requested */
	bl	spl_relocate_stack_gd
	cmp	r0, #0
	movne	sp, r0
	movne	r9, r0
# endif
	ldr	r0, =__bss_start	/* this is auto-relocated! */

#ifdef CONFIG_USE_ARCH_MEMSET
	ldr	r3, =__bss_end		/* this is auto-relocated! */
	mov	r1, #0x00000000		/* prepare zero to clear BSS */

	subs	r2, r3, r0		/* r2 = memset len */
	bl	memset
#else
	ldr	r1, =__bss_end		/* this is auto-relocated! */
	mov	r2, #0x00000000		/* prepare zero to clear BSS */

clbss_l:cmp	r0, r1			/* while not at end of BSS */
#if defined(CONFIG_CPU_V7M)
	itt	lo
#endif
	strlo	r2, [r0]		/* clear 32-bit BSS word */
	addlo	r0, r0, #4		/* move to next */
	blo	clbss_l
#endif

#if ! defined(CONFIG_SPL_BUILD)
	bl coloured_LED_init
	bl red_led_on
#endif
	/* call board_init_r(gd_t *id, ulong dest_addr) */
	mov     r0, r9                  /* gd_t */
	ldr	r1, [r9, #GD_RELOCADDR]	/* dest_addr */
	/* call board_init_r */
#if defined(CONFIG_SYS_THUMB_BUILD)
	ldr	lr, =board_init_r	/* this is auto-relocated! */
	bx	lr
#else
	ldr	pc, =board_init_r	/* this is auto-relocated! */
#endif
	/* we should not return here. */
#endif

ENDPROC(_main)














ulong board_init_f_alloc_reserve(ulong top)
{
	/* Reserve early malloc arena */
#if defined(CONFIG_SYS_MALLOC_F)
	top -= CONFIG_SYS_MALLOC_F_LEN;
#endif
	/* LAST : reserve GD (rounded up to a multiple of 16 bytes) */
	top = rounddown(top-sizeof(struct global_data), 16);

	return top;
}



可以得出这个表
-----------<---0x00900000(CONFIG_SYS_INIT_RAM_ADDR)
|         |
|         |
|         |
|         |
|         |
|         |
-----------<---0x0091FA00(top)
|         |
|         |预留global_data空间且16字节对齐
|         |
-----------<---0x0091FB00
|         |
|         |
|         |(0X400)CONFIG_SYS_MALLOC_F_LEN
|         |
|         |
-----------<---0x0091FF00(CONFIG_SYS_INIT_SP_ADDR)
|         |
-----------<---0x0091FFFF





// 使用上面malloc的空间
// 从 0x0091FA00开始，先占用global_data的空间，并16字节向后对齐
// 再预留CONFIG_SYS_MALLOC_F_LEN 空间，base重新指到了0x0091FF00(CONFIG_SYS_INIT_SP_ADDR)地址
void board_init_f_init_reserve(ulong base)
{
	struct global_data *gd_ptr;
#ifndef _USE_MEMCPY
	int *ptr;
#endif

	/*
	 * clear GD entirely and set it up.
	 * Use gd_ptr, as gd may not be properly set yet.
	 */

	gd_ptr = (struct global_data *)base;
	/* zero the area */
#ifdef _USE_MEMCPY
	memset(gd_ptr, '\0', sizeof(*gd));
#else
	for (ptr = (int *)gd_ptr; ptr < (int *)(gd_ptr + 1); )
		*ptr++ = 0;
#endif
	/* set GD unless architecture did it already */
#if !defined(CONFIG_ARM)
	arch_setup_gd(gd_ptr);
#endif
	/* next alloc will be higher by one GD plus 16-byte alignment */
	base += roundup(sizeof(struct global_data), 16);

	/*
	 * record early malloc arena start.
	 * Use gd as it is now properly set for all architectures.
	 */

#if defined(CONFIG_SYS_MALLOC_F)
	/* go down one 'early malloc arena' */
	gd->malloc_base = base;
	/* next alloc will be higher by one 'early malloc arena' size */
	base += CONFIG_SYS_MALLOC_F_LEN;
#endif
}
