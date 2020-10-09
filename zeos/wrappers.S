#include <asm.h>
#include <segment.h>

ENTRY(writeIDT)
	push	%ebp
	mov    	%esp,%ebp
	push 	%ebx
	mov    	0x08(%ebp),%ebx
	mov    	0x0c(%ebp),%ecx
	mov    	0x10(%ebp),%edx
	
	mov 	$0x04,%eax
	
	int 	$0x80
	
	cmp 	$0,%eax
	jge		return1
	neg 	%eax
	mov 	%eax,errno
	mov		$-1,%eax
	
return1:
	pop		%ebx
	mov		%ebp,%esp
	pop    	%ebp
	ret




ENTRY(write)
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
	
	leal	retsysexit2,%esi
	push	%esi
	
	push 	%ebp
	mov    	%esp,%ebp
	
	sysenter
	
retsysexit2:
	pop		%ebp
	pop		%esi
	
	pop		%edx
	pop		%ecx
	
	cmp 	$0,%eax
	jge		return2
	neg 	%eax
	mov 	%eax,errno
	mov		$-1,%eax
	
return2:
	pop		%esi
	pop		%ebx
	mov		%ebp,%esp
	pop    	%ebp
	ret




ENTRY(gettime)
	push	%ebp
	mov    	%esp,%ebp
	push 	%esi
	
	mov 	$0x0a,%eax
	
	push	%ecx
	push	%edx
	
	leal	retsysexit3,%esi
	push	%esi
	
	push 	%ebp
	mov    	%esp,%ebp
	
	sysenter
	
retsysexit3:
	pop		%ebp
	pop		%esi
	
	pop		%edx
	pop		%ecx
	
	cmp 	$0,%eax
	jge		return3
	neg 	%eax
	mov 	%eax,errno
	mov		$-1,%eax
	
return3:
	pop		%esi
	mov		%ebp,%esp
	pop    	%ebp
	ret




