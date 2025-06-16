OUTPUT_ARCH("riscv")
ENTRY(_start)

/* Memory layout */
MEMORY
{
  /* FLASH   (xrw) : ORIGIN = 0x20000000, LENGTH = 10K */
  FLASH (wrx)  : ORIGIN = 0x80000000, LENGTH = 128K
}

/* Stack and heap configuration */
_estack           = ORIGIN(FLASH) + LENGTH(FLASH);  /* End of FLASH */
_alloc_heap_size  = 0x100;                     /* 8KB heap */
_alloc_stack_size = 0x800;                     /* No reserved stack (can be handled in startup) */

/* Sections layout */
SECTIONS
{
  /* Code */
  .text :
  {
    . = ALIGN(4);
    KEEP(*(.init)) KEEP(*(.fini))
    *(.text) *(.text*)
    *(.rodata) *(.rodata*)
    *(.eh_frame)
    . = ALIGN(4);
    _etext = .;
  } > FLASH

  /* Initialized data section in FLASH (copied from FLASH at runtime) */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data) *(.data*)
    . = ALIGN(4);
    _edata = .;
  } > FLASH

  /* Uninitialized data (zero-initialized) */
  .bss :
  {
    . = ALIGN(4);
    _sbss = .; __bss_start = _sbss;
    *(.bss) *(.bss*) *(COMMON)
    . = ALIGN(4);
    _ebss = .; __bss_end__ = _ebss;
  } > FLASH

  /* Heap and optional stack region */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE(end = .);
    PROVIDE(_end = .);
    _ld_start_heap = .;
    . = . + _alloc_heap_size;
    _ld_end_heap = .;
    . = . + _alloc_stack_size;
    . = ALIGN(8);
  } > FLASH

  /* Strip unused standard library code (optional in embedded) */
  /DISCARD/ :
  {
    *(.comment)
    *(.note*)
    *(.eh_frame_hdr)
    *(.gcc_except_table)
    *(.riscv.attributes)
  }
}
