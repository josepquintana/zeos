#include <asm.h>
#include <segment.h>

ENTRY(write)
	push	%ebp
	mov    	%esp,%ebp
	push 	%ebx
	push 	%esi
	mov    	0x08(%ebp),%ebx
	mov    	0x0c(%ebp),%ecx
	mov    	0x10(%ebp),%edx
	
	mov 	$4,%eax
	
	push	%ecx
	push	%edx
	
	leal	retsysexit1,%esi
	push	%esi
	
	push 	%ebp
	mov    	%esp,%ebp
	
	sysenter
	
retsysexit1:
	pop		%ebp
	pop		%esi
	
	pop		%edx
	pop		%ecx
	
	cmp 	$0,%eax
	jge		return1
	neg 	%eax
	mov 	%eax,errno
	mov		$-1,%eax
	
return1:
	pop		%esi
	pop		%ebx
	mov		%ebp,%esp
	pop    	%ebp
	ret




ENTRY(gettime)
	push	%ebp
	mov    	%esp,%ebp
	push 	%esi
	
	mov 	$10,%eax
	
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
	mov		%ebp,%esp
	pop    	%ebp
	ret




ENTRY(getpid)
	push	%ebp
	mov    	%esp,%ebp
	push 	%esi
	
	mov 	$20,%eax
	
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




ENTRY(fork)
	push	%ebp
	mov    	%esp,%ebp
	push 	%esi
	
	mov 	$2,%eax
	
	push	%ecx
	push	%edx
	
	leal	retsysexit4,%esi
	push	%esi
	
	push 	%ebp
	mov    	%esp,%ebp
	
	sysenter
	
retsysexit4:
	pop		%ebp
	pop		%esi
	
	pop		%edx
	pop		%ecx
	
	cmp 	$0,%eax
	jge		return4
	neg 	%eax
	mov 	%eax,errno
	mov		$-1,%eax
	
return4:
	pop		%esi
	mov		%ebp,%esp
	pop    	%ebp
	ret



ENTRY(exit)
	push	%ebp
	mov    	%esp,%ebp
	push 	%esi
	
	mov 	$1,%eax
	
	push	%ecx
	push	%edx
	
	leal	retsysexit5,%esi
	push	%esi
	
	push 	%ebp
	mov    	%esp,%ebp
	
	sysenter
	
retsysexit5:
	pop		%ebp
	pop		%esi
	
	pop		%edx
	pop		%ecx
	
	cmp 	$0,%eax
	jge		return5
	neg 	%eax
	mov 	%eax,errno
	mov		$-1,%eax
	
return5:
	pop		%esi
	mov		%ebp,%esp
	pop    	%ebp
	ret



ENTRY(get_stats)
	push	%ebp
	mov    	%esp,%ebp
	push 	%ebx
	push 	%esi
	mov    	0x08(%ebp),%ebx
	mov    	0x0c(%ebp),%ecx
	
	mov 	$35,%eax
	
	push	%ecx
	push	%edx
	
	leal	retsysexit6,%esi
	push	%esi
	
	push 	%ebp
	mov    	%esp,%ebp
	
	sysenter
	
retsysexit6:
	pop		%ebp
	pop		%esi
	
	pop		%edx
	pop		%ecx
	
	cmp 	$0,%eax
	jge		return6
	neg 	%eax
	mov 	%eax,errno
	mov		$-1,%eax
	
return6:
	pop		%esi
	pop		%ebx
	mov		%ebp,%esp
	pop    	%ebp
	ret

