u-boot.lds内有这些SECTION(代码段)

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
 . = 0x00000000;    // 初步设置代码起始地址为0x00000000
 . = ALIGN(4);      // 四字节对齐。4*8为32
                    // 四字节对齐，也与32位的数据呼应。比如0x00001000写一个val 0x12345678
                    // 地址增量变为 0x00001004 接着写0x87654321
/*
 * .text段，只读。可执行程序加载到内存后，会将.text段设置为只读
 * 一般用于放程序代码
 */
                                      // 起始地址              长度
 .text :                              // 0x0000000087800000    0x3c80c
 {
  *(.__image_copy_start)              // 0x0000000087800000    0
  *(.vectors)                         // 0x0000000087800000    0x300
  arch/arm/cpu/armv7/start.o (.text*) // 0x0000000087800300    0xb0
  *(.text*)                           // 0x00000000878003b0    0x3c500
 }

 /*
  * rodata段，只读。read only data 段
  * 用于存放常量数据
  */
// SORT_BY_ALIGNMENT  排成直线
// SORT_BY_NAME       按名称排序
 . = ALIGN(4);
 .rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) } // 0x000000008783c810   0xf56a
 . = ALIGN(4);

 /*
  * data段，可读可写。存放编译时就可知的全局变量(已初始化)。
  */
 .data : {                            // 0x000000008784bda8   0x20a8
  *(.data*)
 }
 . = ALIGN(4);
 . = .;
 . = ALIGN(4);
 .u_boot_list : {
  KEEP(*(SORT(.u_boot_list*)));       // 0x000000008784de5c   0xb90
 }
 . = ALIGN(4);
 .image_copy_end :
 {
  *(.__image_copy_end)                // 0x000000008784e9ec   0
 }

  /*
  * .rel.dyn是重定位相关的，不理解。先不管
  */
 .rel_dyn_start :                     // 0x000000008784e9ec   0x8690
 {
  *(.__rel_dyn_start)
 }
 .rel.dyn : {
  *(.rel*)
 }
 .rel_dyn_end :                       // 0x000000008785707c   0
 {
  *(.__rel_dyn_end)
 }
 .end :                               // 0x000000008785707c   0
 {
  *(.__end)
 }
 _image_binary_end = .;
 . = ALIGN(4096);            // 将位置计数器对齐到4096字节边界（很多计算机体系结构使用页内存管理，通常是4096字节）
 .mmutable : {
  *(.mmutable)
 }

 /*
  * bss段，可读可写。未初始化或者初始化为0的全局变量
  */
 .bss_start __rel_dyn_start (OVERLAY) : {   // .bss_start 的值为 __rel_dyn_start的地址，(OVERLAY)表示这个标签的地址在编译时不确定，要在链接时进行计算
  KEEP(*(.__bss_start));              // 0x000000008784e9ec   0   // KEEP 表示保留这个标签的对象，防止被优化掉
  __bss_base = .;
 }
 .bss __bss_base (OVERLAY) : {        // 0x000000008784e9ec   0x4afe8
  *(.bss*)
   . = ALIGN(4);
   __bss_limit = .;
 }
 .bss_end __bss_limit (OVERLAY) : {   // 0x00000000878999d4   0
  KEEP(*(.__bss_end));
 }
 .dynsym _image_binary_end : { *(.dynsym) }   // .dynsym 动态符号表
 .dynbss : { *(.dynbss) }                     // .dynbss 动态符号表的bss段
 .dynstr : { *(.dynstr*) }                    // .dynstr 动态符号表的字符串段
 .dynamic : { *(.dynamic*) }                  // .dynamic 动态连接信息
 .plt : { *(.plt*) }                          // .plt 过程链接表
 .interp : { *(.interp*) }                    // .interp 动态链接器路径
 .gnu.hash : { *(.gnu.hash) }                 // .gnu.hash 哈希表
 .gnu : { *(.gnu*) }                          // .gnu gcc特有段
 .ARM.exidx : { *(.ARM.exidx*) }              // .ARM.exidx 异常处理表
 .gnu.linkonce.armexidx : { *(.gnu.linkonce.armexidx.*) }   // .gnu.linkonce.armexidx 异常处理表中只会出现一次的符号
}


