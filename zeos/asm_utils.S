#include <asm.h>
#include <segment.h>

ENTRY(task_switch)
	push	%ebp
	mov		%esp,%ebp
	push	%ebx
	push	%esi
	push	%edi

	push	8(%ebp) # why 8 ??
	call inner_task_switch
	addl	$4, %esp

	pop		%edi
	pop		%esi
	pop		%ebx
	mov		%ebp,%esp
	pop		%ebp
	ret



ENTRY(writeMSR)
	push	%ebp
	mov		%esp,%ebp
	mov		0x08(%ebp),%ecx
	mov		0x0c(%ebp),%eax
	mov		$0,%edx
	wrmsr						// Write the value in edx:eax to MSR specified by ecx.
	mov		%ebp,%esp
	pop		%ebp
	ret

