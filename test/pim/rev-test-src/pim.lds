SECTIONS
{
  . = 0x00010000;
  .bss : { *(.bss) }
  . = 0x0E000000;
  .func_base : { *(.func_base) }
  . = 0x0E000100;
  .pimsram : { *(.pimsram) }
  . = 0x0E001100;
  .pimdram : { *(.pimdram) }
  . = 0x0E400500; 
  .data : { *(.data) }
  .text : { *(.text) }
}
