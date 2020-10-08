#include <asm.h>

ENTRY(write)
	push	%ebp
	mov    	%esp,%ebp
	push 	%ebx
	mov    	0x08(%ebp),%ebx
	mov    	0x0c(%ebp),%ecx
	mov    	0x10(%ebp),%edx
	
	mov 	$0x04,%eax
	
	int 	$0x80
	
	cmp 	$0,%eax
	bge		return
	neg 	%eax
	mov 	%eax,errno
	mov		$-1,%eax
	
return:
	pop		%ebx
	mov		%ebp,%esp
	pop    	%ebp
	ret


