
#define SECURITY_WIN32

#include <stdio.h>
#include <Windows.h>
#include <Security.h>
#include <secext.h>

BOOL DomainCheck(CHAR *domain) {
	BOOL Result = FALSE;
	CHAR buffer[512];
	DWORD dwSize = 512;

	//a buffer just points to the beginning of the string
	GetUserNameExA(NameSamCompatible, buffer, &dwSize);

	//we need to extract just the domain, not the user.
	//use strstr to find the first occurence of '\' within the buffer. returns a pointer
	//we need to escape the '\' because it's a special character
	CHAR *position = strstr(buffer, "\\");
	
	//print both pointers, should be very similar memory addresses
	printf("%p\n%p\n", buffer, position);
    
	//assign position to null
	position[0] = 0x00;
    printf("%s\n", buffer);
	
	//our if statement comparing what's in buffer to our actual target domain
	if(strcmp(domain, buffer) == 0) {
		Result = TRUE;
		
	} 

}

int main(){
	if(!DomainCheck("domaingoeshere")) {
		printf("this user is not within the target domain.\n");
	} else {
		// dump LSASS, kerberoast, pwn everything in the world
	}
	
	return 0;
}
