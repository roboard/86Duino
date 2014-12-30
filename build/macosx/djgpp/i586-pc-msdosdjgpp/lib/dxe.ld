OUTPUT_FORMAT("coff-go32")

FORCE_COMMON_ALLOCATION

SECTIONS {
  .text : {
    *(.text)
    *(.text.*)
    *(.gnu.linkonce.t*)
    *(.const*)
    *(.ro*)
    *(.gnu.linkonce.r*)
    djgpp_first_ctor = . ;
    *(SORT(.ctors.*))
    *(.ctor)
    *(.ctors)
    djgpp_last_ctor = . ;
    djgpp_first_dtor = . ;
    *(SORT(.dtors.*))
    *(.dtor)
    *(.dtors)
    djgpp_last_dtor = . ;
    *(.data)
    *(.data.*)
    *(.gcc_exc*)
    ___EH_FRAME_BEGIN__ = . ;
    *(.eh_fram*)
    ___EH_FRAME_END__ = . ;
    LONG(0);
    *(.gnu.linkonce.d*)
    *(.bss)
    *(COMMON)
  }
}
