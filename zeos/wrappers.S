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
	jge		return
	neg 	%eax
	mov 	%eax,errno
	mov		$-1,%eax
	
return:
	pop		%ebx
	mov		%ebp,%esp
	pop    	%ebp
	ret



ENTRY(writeMSR)
	push	%ebp
	mov    	%esp,%ebp
	push 	%ebx
	mov    	0x08(%ebp),%ebx
	mov    	0x0c(%ebp),%ecx
	mov    	0x10(%ebp),%edx
	
	mov 	$0x04,%eax
	
	sysenter
	
	cmp 	$0,%eax
	jge		return
	neg 	%eax
	mov 	%eax,errno
	mov		$-1,%eax
	
return:
	pop		%ebx
	mov		%ebp,%esp
	pop    	%ebp
	ret


