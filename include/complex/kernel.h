#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

//symbols from linker.ld for section addresses
extern uchar __kernel_section_start;
extern uchar __kernel_section_end;
extern uchar __kernel_text_section_start;
extern uchar __kernel_text_section_end;
extern uchar __kernel_data_section_start;
extern uchar __kernel_data_section_end;
extern uchar __kernel_rodata_section_start;
extern uchar __kernel_rodata_section_end;
extern uchar __kernel_bss_section_start;
extern uchar __kernel_bss_section_end;

#endif


