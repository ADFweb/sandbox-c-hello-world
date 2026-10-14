#!/usr/bin/env python3
import fcntl
import os
import struct
import sys
import time

# ioctl request codes from <linux/watchdog.h> (WATCHDOG_IOCTL_BASE = 'W')
WDIOC_GETSUPPORT = 0x80285700   # _IOR('W', 0, struct watchdog_info)
WDIOC_SETOPTIONS = 0x40045704   # _IOW('W', 4, int), same as the C++ example
WDIOC_KEEPALIVE = 0x80045705    # _IOR('W', 5, int)
WDIOC_SETTIMEOUT = 0xC0045706   # _IOWR('W', 6, int)
WDIOC_GETTIMEOUT = 0x80045707   # _IOR('W', 7, int)
WDIOC_GETTIMELEFT = 0x8004570A  # _IOR('W', 10, int)

WDIOS_DISABLECARD = 0x0001
WDIOS_ENABLECARD = 0x0002

WATCHDOG_INFO_SIZE = 40  # __u32 options, __u32 firmware_version, __u8 identity[32]


def ioctl_int(fd, request, value=0):
    buf = bytearray(struct.pack("i", value))
    fcntl.ioctl(fd, request, buf)
    return struct.unpack("i", buf)[0]


def watchdog_start(timeout):
    try:
        fd = os.open("/sandbox/watchdog", os.O_WRONLY)
    except OSError:
        return -1

    try:
        fcntl.ioctl(fd, WDIOC_GETSUPPORT, bytearray(WATCHDOG_INFO_SIZE))
        ioctl_int(fd, WDIOC_SETTIMEOUT, timeout)
        set_timeout = ioctl_int(fd, WDIOC_GETTIMEOUT)
        print(f"Set watchdog timeout to {set_timeout}")
        ioctl_int(fd, WDIOC_SETOPTIONS, WDIOS_ENABLECARD)
    except OSError:
        os.close(fd)
        return -1

    return fd


def watchdog_keepalive(fd):
    if fd <= 0:
        return False
    try:
        fcntl.ioctl(fd, WDIOC_KEEPALIVE, 0)
        return True
    except OSError:
        return False


def watchdog_stop(fd):
    if fd > 0:
        try:
            ioctl_int(fd, WDIOC_SETOPTIONS, WDIOS_DISABLECARD)
        except OSError:
            return False
    os.close(fd)
    return True


def led_set(led_number, on):
    try:
        fd = os.open(f"/sandbox/leds/led{led_number}", os.O_WRONLY)
    except OSError:
        return False
    os.write(fd, b"1" if on else b"0")
    os.close(fd)
    return True


def main():
    print("Hello, World!", flush=True)
    watchdog_fd = watchdog_start(5)
    if watchdog_fd == -1:
        return 1

    led_on = False
    for _ in range(30):
        time_left = ioctl_int(watchdog_fd, WDIOC_GETTIMELEFT)
        print(f"Watchdog time left: {time_left}", flush=True)

        led_set(2, led_on)
        led_on = not led_on

        watchdog_keepalive(watchdog_fd)
        time.sleep(1)
    watchdog_stop(watchdog_fd)
    return 0


if __name__ == "__main__":
    sys.exit(main())
