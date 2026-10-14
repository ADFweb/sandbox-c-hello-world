# SandboxTest

Small example programs for the ADFweb sandbox. The hello world example is
implemented in C++, Python and C#. Each one prints "Hello, World!", arms the
watchdog at `/sandbox/watchdog` with a 5 s timeout, then for 30 seconds kicks it
once a second and toggles LED 2 (`/sandbox/leds/led2`).

## Layout

| Folder    | Contents                                                        |
|-----------|-----------------------------------------------------------------|
| `cpp/`    | Hello world (`main.cpp`) and Modbus RTU master/slave (`main_modbus.cpp`) |
| `python/` | Hello world (`hello_world.py`)                                  |
| `csharp/` | Hello world (`HelloWorld/`, a .NET 8 console app)               |

## Running

**C++** (Linux toolchain, libmodbus needed for the Modbus example):

```bash
cmake -S cpp -B build && cmake --build build
```

This produces `sandbox-c-hello-world` and `sandbox-modbus`. For the Modbus
example, copy `libmodbus.so.5.1.0` to the sandbox and run with
`LD_PRELOAD=/home/sandbox/libmodbus.so.5.1.0`.

**Python** (standard library only):

```bash
python3 python/hello_world.py
```

**C#** (.NET 8 runtime on the target):

```bash
dotnet publish csharp/HelloWorld -c Release -r linux-arm --self-contained
```

Pick the runtime identifier that matches the sandbox (`linux-arm`, `linux-arm64`).

## License

MIT, see [LICENSE](LICENSE).
