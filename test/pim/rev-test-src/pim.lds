SECTIONS
{
  . = 0x00010000;
  .text : { *(.text) }
  . = 0x08000000;
  .data : { *(.data) }
  .bss : { *(.bss) }
  . = 0xF000000;
  .pimsram : { *(.pimsram) }
    . = 0xF800000;
  .pimdram : { *(.pimdram) }
}
