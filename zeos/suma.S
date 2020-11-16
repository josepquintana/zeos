#include <asm.h>
#include <segment.h>

ENTRY(addASM)
	push	%ebp
	mov		%esp,%ebp
	mov		0x8(%ebp),%eax
	mov		0xc(%ebp),%ecx
	add		%ecx,%eax
	mov		%ebp,%esp
	pop		%ebp
	ret    
