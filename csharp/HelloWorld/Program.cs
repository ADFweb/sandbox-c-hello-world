using System.Runtime.InteropServices;

internal static class Native
{
    public const int O_WRONLY = 1;

    [DllImport("libc", SetLastError = true)]
    public static extern int open(string path, int flags);

    [DllImport("libc", SetLastError = true)]
    public static extern int close(int fd);

    [DllImport("libc", SetLastError = true)]
    public static extern nint write(int fd, byte[] buf, nuint count);

    [DllImport("libc", SetLastError = true)]
    public static extern int ioctl(int fd, nuint request, ref int arg);

    [DllImport("libc", SetLastError = true)]
    public static extern int ioctl(int fd, nuint request, byte[] arg);

    [DllImport("libc", SetLastError = true)]
    public static extern int ioctl(int fd, nuint request, nint arg);
}

internal static class Program
{
    // ioctl request codes from <linux/watchdog.h> (WATCHDOG_IOCTL_BASE = 'W')
    private const uint WDIOC_GETSUPPORT = 0x80285700;  // _IOR('W', 0, struct watchdog_info)
    private const uint WDIOC_SETOPTIONS = 0x40045704;  // _IOW('W', 4, int), same as the C++ example
    private const uint WDIOC_KEEPALIVE = 0x80045705;   // _IOR('W', 5, int)
    private const uint WDIOC_SETTIMEOUT = 0xC0045706;  // _IOWR('W', 6, int)
    private const uint WDIOC_GETTIMEOUT = 0x80045707;  // _IOR('W', 7, int)
    private const uint WDIOC_GETTIMELEFT = 0x8004570A; // _IOR('W', 10, int)

    private const int WDIOS_DISABLECARD = 0x0001;
    private const int WDIOS_ENABLECARD = 0x0002;

    private const int WatchdogInfoSize = 40; // __u32 options, __u32 firmware_version, __u8 identity[32]

    private static int WatchdogStart(int timeout)
    {
        int fd = Native.open("/sandbox/watchdog", Native.O_WRONLY);
        if (fd == -1)
        {
            return -1;
        }

        var info = new byte[WatchdogInfoSize];
        if (Native.ioctl(fd, WDIOC_GETSUPPORT, info) == -1)
        {
            Native.close(fd);
            return -1;
        }

        if (Native.ioctl(fd, WDIOC_SETTIMEOUT, ref timeout) == -1)
        {
            Native.close(fd);
            return -1;
        }

        int setTimeout = 0;
        if (Native.ioctl(fd, WDIOC_GETTIMEOUT, ref setTimeout) == -1)
        {
            Native.close(fd);
            return -1;
        }
        Console.WriteLine($"Set watchdog timeout to {setTimeout}");

        int flags = WDIOS_ENABLECARD;
        if (Native.ioctl(fd, WDIOC_SETOPTIONS, ref flags) == -1)
        {
            Native.close(fd);
            return -1;
        }

        return fd;
    }

    private static bool WatchdogKeepalive(int fd)
    {
        return fd > 0 && Native.ioctl(fd, WDIOC_KEEPALIVE, 0) == 0;
    }

    private static bool WatchdogStop(int fd)
    {
        if (fd > 0)
        {
            int flags = WDIOS_DISABLECARD;
            if (Native.ioctl(fd, WDIOC_SETOPTIONS, ref flags) == -1)
            {
                return false;
            }
        }
        Native.close(fd);
        return true;
    }

    private static bool LedSet(int ledNumber, bool on)
    {
        int fd = Native.open($"/sandbox/leds/led{ledNumber}", Native.O_WRONLY);
        if (fd == -1)
        {
            return false;
        }
        var value = new[] { (byte)(on ? '1' : '0') };
        Native.write(fd, value, 1);
        Native.close(fd);
        return true;
    }

    private static int Main()
    {
        Console.WriteLine("Hello, World!");
        int watchdogFd = WatchdogStart(5);
        if (watchdogFd == -1)
        {
            return 1;
        }

        bool ledOn = false;
        for (int i = 0; i < 30; i++)
        {
            int timeLeft = 0;
            Native.ioctl(watchdogFd, WDIOC_GETTIMELEFT, ref timeLeft);
            Console.WriteLine($"Watchdog time left: {timeLeft}");

            LedSet(2, ledOn);
            ledOn = !ledOn;

            WatchdogKeepalive(watchdogFd);
            Thread.Sleep(TimeSpan.FromSeconds(1));
        }
        WatchdogStop(watchdogFd);
        return 0;
    }
}
