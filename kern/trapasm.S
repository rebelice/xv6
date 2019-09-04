#include <inc/mmu.h>

	# vectors.S sends all traps here.
.globl alltraps
alltraps:
	# Your code here:
	#
	# Hint: you should build trap frame, set up data segments and then
	# call trap(tf)


	# Return falls through to trapret...
.globl trapret
trapret:
	# Restore registers.
	# Your code here:
	iret
