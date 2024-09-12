SECTIONS
{
  . = 0x00010000;
  .text : { *(.text) }
  . = 0x08000000;
  .data : { *(.data) }
  .bss : { *(.bss) }
  . = 0x0E000000;
  .func_base : { *(.func_base) }
  . = 0x0F000000;
  .pimsram : { *(.pimsram) }
  . = 0x0F800000;
  .pimdram : { *(.pimdram) }
}
