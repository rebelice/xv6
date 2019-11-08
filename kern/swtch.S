# Context switch
#
#   void swtch(struct context **old, struct context *new);
# 
# Save the current registers on the stack, creating
# a struct context, and save its address in *old.
# Switch stacks to new and pop previously-saved registers.

.globl swtch
swtch:
	# TODO: your code here

	# Save old callee-saved registers

	# Switch stacks

	# Load new callee-saved registers

	ret