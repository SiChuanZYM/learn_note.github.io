


// cpsr寄存器 当前程序状态寄存器设置为SVC模式
1.  mrs r标, r源
    将源寄存器值移动到目标寄存器(比较特殊，用于cpsr这种特殊寄存器)
    如：mrs r0, cpsr (特殊)将CPSR的值移动到r0

2.  and r标, r源, #值
    将r源与值进行与运算，并将结果保存
    如：and r1, r0, #0x1f 按位与运算，将r0与0x1f与运算后保存在r1. 获得cpsr模式位

3.  teq r源, #值
    将r与值判断是否相等，并将结果用作后续条件判断
    如：teq r1, #0x1a 将r1与0x1a判断相等性，作为下面的条件，判断是否为HYP模式

4.  bic, r0, r1, #value
    value先取反，在与r1进行与运算，并将结果保存再r0
    如：bic r0, r0, #0x1f 0x1f按位取反后，与上r0，再保存进r0

5.  bicne xxxx
    和bic一致，加上了ne判断，取上一次的结果，如果不等于(not equal)就执行后续

6.  orr r0, r1, #val
    r1 | val, wr in r0
    eg: orrne r0, r0, #0x13

7.  b
    跳转，不返回

8.  bl
    跳转，保存跳转前地址，子函数执行完后返回跳转点

9.  .section
    伪指令，定义了代码段(section)
    eg: .globl _start
            .section ".vectors", "ax"
        _start:
            xxxx

        伪指令，定义了一个名为".vectors"的代码段，并且拥有"ax"属性
        "a"这个代码段包含可执行的代码
        "x"这个代码段包含只读数据

10. ldr
    寄存器赋值
    eg: ldr	sp, =CONFIG_SYS_INIT_SP_ADDR
    将sp赋值为 CONFIG_SYS_INIT_SP_ADDR的值

    eg: ldr	sp, [r9, #GD_START_ADDR_SP]
    为sp取值，取的值为[r9 + #GD_START_ADDR_SP]地址对应的值
    即 sp = *(unsigned int *)(0x0091FA00 + 60)

10.2 str
    将值写入寄存器

11. push	{ip, lr}
    压栈，将ip和lr的值压栈

12. lr寄存器
    链接寄存器，储存下一条指令的位置。当执行一个跳转 bl或bx时，会把跳转指令的下一条指令的地址存储到lr中。
    跳转前压栈，返回后出栈。一般和ip联合使用，以便恢复现场。

13. ip寄存器
    存储中间结果。别名r12.

14. pop	{ip, pc}
    出栈。恢复ip(r12)和pc
    
15. r9寄存器
    r9寄存器是全局指针寄存器，会一直保存写入的值，直到被改变

16. sub
    减法计算，并赋给寄存器
    eg: sub	r9, r9, #GD_SIZE
    r9寄存器的值，减去GD_SIZE后，保存在r9

17. subs
    减法计算，并赋给寄存器，并作为条件判断
    r4, r0, r1
    ↓
18. beq
    如果 eq，则 执行b跳转
    beq	relocate_done
    ↑以17. 的为条件，如果r0==r1，则执行b跳转

19. !
    地址会自增
    r1!，r1参与的这条语句结束后，地址会自增
20. ldmia
    从内存中读取一组数据到另一组寄存器中
    ldmia	r1!, {r10-r11}
    从r1中一次读取两个32位数据，分别放入r10和r11，然后r1再+8 byte
    可以用于一组连续的数
21. stmia
    将一组连续数据，存储到内存中
    stmia	r0!, {r10-r11}
    将r10和r11的数据，一次性放入r1中，然后r1再+8 byte
    可以用于一组连续的数
22. cmp
    比较内存值
    cmp	r1, r2
    比较 r1和r2
23. blo
    条件分支指令之一，如果结果 < xxx 就b到目标

24. 




