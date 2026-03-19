#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <cstring>
#include <thread>
#include <chrono>

// Helper function to send a click event
void emit(int fd, int type, int code, int val)
{
    struct input_event ie;
    memset(&ie, 0, sizeof(ie));
    ie.type = type;
    ie.code = code;
    ie.value = val;

    // Check the return value to satisfy the compiler and ensure safety
    if (write(fd, &ie, sizeof(ie)) < 0)
    {
        std::cerr << "Fatal: Failed to write to uinput device." << std::endl;
        exit(1);
    }
}

int main()
{
    // 1. Open the uinput device
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0)
    {
        std::cerr << "Error: Could not open /dev/uinput. Run with sudo." << std::endl;
        return 1;
    }

    // 2. Setup the device to support key/button events
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);

    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;  // Mock Vendor ID
    usetup.id.product = 0x5678; // Mock Product ID
    strcpy(usetup.name, "StellarClicker-Virtual-Mouse");

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    // Give the OS a moment to register the new device
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Auto-clicker started. Press Ctrl+C to stop." << std::endl;

    double interval_seconds = 120; // Your preferred interval

    while (true)
    {
        // Press Left Click
        emit(fd, EV_KEY, BTN_LEFT, 1);
        emit(fd, EV_SYN, SYN_REPORT, 0);

        // Release Left Click
        emit(fd, EV_KEY, BTN_LEFT, 0);
        emit(fd, EV_SYN, SYN_REPORT, 0);

        // Wait for the interval
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(interval_seconds * 1000)));
    }

    // Cleanup (not reached in this infinite loop)
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);

    return 0;
}