#include <iostream>
#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>

#define	WDIOC_SETOPTIONS	_IOW(WATCHDOG_IOCTL_BASE, 4, int)

int watchdog_start(const int timeout) {
    const int fd = open("/sandbox/watchdog", O_WRONLY);
    if (fd == -1) {
        return -1;
    }

    watchdog_info info{};
    if (ioctl(fd, WDIOC_GETSUPPORT, &info) == -1) {
        close(fd);
        return -1;
    }

    if (ioctl(fd, WDIOC_SETTIMEOUT, &timeout) == -1) {
        close(fd);
        return -1;
    }

    int set_timeout;
    if (ioctl(fd, WDIOC_GETTIMEOUT, &set_timeout) == -1) {
        close(fd);
        return -1;
    }
    std::cout << "Set watchdog timeout to " << set_timeout << std::endl;

    int flags = WDIOS_ENABLECARD;
    if (ioctl(fd, WDIOC_SETOPTIONS, &flags) == -1) {
        close(fd);
        return -1;
    }

    return fd;
}

bool watchdog_keepalive(const int fd) {
    return fd > 0 && ioctl(fd, WDIOC_KEEPALIVE, 0) == 0;
}

bool watchdog_stop(const int fd) {
    if (fd > 0) {
        int flags = WDIOS_DISABLECARD;
        if (ioctl(fd, WDIOC_SETOPTIONS, &flags) == -1) {
            return false;
        }
    }
    close(fd);
    return true;
}

bool led_set(const int led_number, const bool on) {
    const std::string path = "/sandbox/leds/led" + std::to_string(led_number);
    const int fd = open(path.c_str(), O_WRONLY);
    if (!fd) {
        return false;
    }
    char value;
    if (on) {
        value = '1';
    } else {
        value = '0';
    }
    write(fd, &value, 1);
    close(fd);
    return true;
}

int main() {
    std::cout << "Hello, World!" << std::endl;
    const int watchdog_fd = watchdog_start(5);
    if (watchdog_fd == -1) {
        return 1;
    }

    bool led_on = false;
    for (int i = 0; i < 30; i++) {
        int time_left;
        ioctl(watchdog_fd, WDIOC_GETTIMELEFT, &time_left);
        std::cout << "Watchdog time left: " << time_left << std::endl;

        led_set(2, led_on);
        led_on = !led_on;

        watchdog_keepalive(watchdog_fd);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    watchdog_stop(watchdog_fd);
    return 0;
}
