SECTIONS
{
  . = 0x00010000;
  .bss : { *(.bss) }
  . = 0x0E000000;
  .func_base : { *(.func_base) }
  . = 0xe000100;
  .pimsram : { *(.pimsram) }
  . = 0xe000500;
  .pimdram : { *(.pimdram) }
  . = 0xe400500;
  .data : { *(.data) }
  .text : { *(.text) }
}
