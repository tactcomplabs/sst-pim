SECTIONS
{
  . = 0x00010000;
  .bss : { *(.bss) }
  . = 0x0E000000;
  .func_base : { *(.func_base) }
  . = 0x0E000100;
  .pimsram : { *(.pimsram) }
  . = 0x0E000500;
  .pimdram : { *(.pimdram) }
  . = 0x0E100500; 
  .data : { *(.data) }
  .text : { *(.text) }
}
