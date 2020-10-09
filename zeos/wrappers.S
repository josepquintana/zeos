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
	push 	%esi
	mov    	0x08(%ebp),%ebx
	mov    	0x0c(%ebp),%ecx
	mov    	0x10(%ebp),%edx
	
	mov 	$0x04,%eax
	
	push	%ecx
	push	%edx
	
	leal	mylabel,%esi
	push	%esi
	
	push 	%ebp
	mov    	%esp,%ebp
	
	sysenter
	
retsysexit:
	pop		%ebp
	pop		%esi
	
	pop		%edx
	pop		%ecx
	
	cmp 	$0,%eax
	jge		return
	neg 	%eax
	mov 	%eax,errno
	mov		$-1,%eax
	
return:
	pop		%esi
	pop		%ebx
	mov		%ebp,%esp
	pop    	%ebp
	ret


