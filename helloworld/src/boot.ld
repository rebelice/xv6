ENTRY(start)

SECTIONS
{
	. = 0x7c00;
	.text :
	{
		*(.entry)
		*(.text)
	}
	
	.data : {*(.data)}
	
	.rodata : {*(.rodata)}
	
	__bss_start = .;
	.bss ALIGN(4) : {*(.bss) *(COMMON)}
	__bss_end = .;
	
	/DISCARD/ :
	{
		*(.note.gnu.build-id)
		*(.eh_frame)
		*(.eh_frame_hdr)
	}
}