/*
 * advance_coroutine.asm
 *
 *  Created on: 2026年5月17日
 *      Author: Reed
 */


.extern adv_cor_intr_flag
.extern adv_cor_addr;
.extern adv_cor_main
.extern test_jmp

.macro tim4_reset
	li t0,0x40000824
	sw zero, 0(t0)
	li t0,0x40000810
	sw zero, 0(t0)
.endm

.macro tim4_start
	li t0,0x40000800
	lw t1, 0(t0)
	or t1, t1, 1
	sw t1, 0(t0)
.endm

.macro tim4_end
	li t0,0x40000800
	lw t1, 0(t0)
	//and t1,t1,0xfffe
	li t2,0xfffe
	and t1,t1,t2
	sw t1, 0(t0)
.endm

.align 4
.func
cor_try_intr:
	addi sp, sp, -128
	sw x5,  3  * 4( sp )
	sw x6,  4  * 4( sp )
	sw x7,  5  * 4( sp )

	csrr t1, mepc
	la t0,adv_cor_addr
	sw t1,0(t0)
	li t0, 0x40000810
	lw t0, 0(t0)
	li t1, 0x1
	and t0,t0,t1
	bne t0, t1, hang//not update,ignore

	//ebreak
	la t0, adv_cor_intr_flag
	lw t0, 0(t0)
	bne t0, zero, hang//hanging,we skip

	la t0, adv_cor_main
	csrr t1, mepc
	csrw mepc, t0

	sw t1,  1 * 4( sp )
	la t0,adv_cor_addr
	sw t1,0(t0)

	sw x1,  2  * 4( sp )
	sw x8,  6  * 4( sp )
	sw x9,  7  * 4( sp )
	sw x10, 8  * 4( sp )
	sw x11, 9  * 4( sp )
	sw x12, 10 * 4( sp )
	sw x13, 11 * 4( sp )
	sw x14, 12 * 4( sp )
	sw x15, 13 * 4( sp )
	sw x16, 14 * 4( sp )
	sw x17, 15 * 4( sp )
	sw x18, 16 * 4( sp )
	sw x19, 17 * 4( sp )
	sw x20, 18 * 4( sp )
	sw x21, 19 * 4( sp )
	sw x22, 20 * 4( sp )
	sw x23, 21 * 4( sp )
	sw x24, 22 * 4( sp )
	sw x25, 23 * 4( sp )
	sw x26, 24 * 4( sp )
	sw x27, 25 * 4( sp )
	sw x28, 26 * 4( sp )
	sw x29, 27 * 4( sp )
	sw x30, 28 * 4( sp )
	sw x31, 29 * 4( sp )
	sw gp, 30* 4(sp)
	sw tp, 31*4(sp)
	la t0, adv_cor_intr_flag
	sw sp, 0(t0)
	//ebreak
	//addi sp, sp, +256
	tim4_end
	tim4_reset
	j intr_end
	hang://recover
	tim4_end
	tim4_reset
	lw x5,  3  * 4( sp )
	lw x6,  4  * 4( sp )
	lw x7,  5  * 4( sp )
	addi sp, sp, 128//restore sp
	j intr_end
	intr_end:
	//fence.i
	mret
.endfunc

.align 4
.func
cor_try_restore:
	la t2, adv_cor_intr_flag
	lw t0, 0(t2)
	//ebreak
	beq t0, zero, restore_end
	lw sp, 0(t2)
	lw x1,  2  * 4( sp )
	lw x8,  6  * 4( sp )
	lw x9,  7  * 4( sp )
	lw x10, 8  * 4( sp )
	lw x11, 9  * 4( sp )
	lw x12, 10 * 4( sp )
	lw x13, 11 * 4( sp )
	lw x14, 12 * 4( sp )
	lw x15, 13 * 4( sp )
	lw x16, 14 * 4( sp )
	lw x17, 15 * 4( sp )
	lw x18, 16 * 4( sp )
	lw x19, 17 * 4( sp )
	lw x20, 18 * 4( sp )
	lw x21, 19 * 4( sp )
	lw x22, 20 * 4( sp )
	lw x23, 21 * 4( sp )
	lw x24, 22 * 4( sp )
	lw x25, 23 * 4( sp )
	lw x26, 24 * 4( sp )
	lw x27, 25 * 4( sp )
	lw x28, 26 * 4( sp )
	lw x29, 27 * 4( sp )
	lw x30, 28 * 4( sp )
	lw x31, 29 * 4( sp )
	lw gp, 30 * 4(sp)
	lw tp, 31 * 4(sp)

	la t0,adv_cor_addr
	lw t0, 0(t0)

	sw zero, 0(t2)//reset flag
	csrw mepc, t0
	lw x5,  3  * 4( sp )
	lw x6,  4  * 4( sp )
	lw x7,  5  * 4( sp )//restore t2
	addi sp, sp, 128 //restore sp
	restore_end:
	//fence.i
	mret
.endfunc


.globl _adv_cor_interrupt
.align 4
.func
_adv_cor_interrupt:
	j cor_try_intr
	.endfunc

.globl _adv_cor_restore
.align 4
.func
_adv_cor_restore:
	j cor_try_restore
	.endfunc

.globl TIM4_IRQHandler
.align 4
.func
TIM4_IRQHandler:
	//fence.i
	j _adv_cor_interrupt
	.endfunc

.globl SW_Handler
.align 4
.func
SW_Handler:
	//fence.i
	li t0, 0xE000F014
	sw zero, 0(t0)//remove sw intr trigger
	j _adv_cor_restore
	.endfunc

/*
目前恢复时有时可以跳转，有时会跑飞
不能跳转的问题怀疑是编译器抽了，清理了一次构建好了
*/
