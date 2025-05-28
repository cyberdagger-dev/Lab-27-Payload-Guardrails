#include <windows.h>
#include <iostream>

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
        return diskSizeGB >= 100;
        // return diskSizeGB >= 90;
    }

    CloseHandle(hDevice);
    return false;
}

int main() {
    if (isPhysicalSystem()) {
        std::cout << "Physical system detected. Continuing normal operations...\n";
    } else {
        std::cout << "Possibly running in a virtual machine or sandbox. Exiting...\n";
    }

    return 0;
}
