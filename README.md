# Linux /proc Counter

[![Checks](https://github.com/IcaroRamosDM/proc_counter/actions/workflows/checks.yml/badge.svg)](https://github.com/IcaroRamosDM/proc_counter/actions/workflows/checks.yml)

An educational kernel module exposing an integer and synchronized counters at
/proc/proc_counter. The canonical implementation is proc_counter.c;
[the Portuguese guide](docs/COMO_FUNCIONA.md) explains the code.

## Interface

- Read a snapshot containing Read count, Write count and Stored value.
- Write a decimal signed int to update the value and count an accepted write.
- Write reset to clear the state.
- Empty writes preserve the state; invalid commands, embedded NUL bytes,
  out-of-range integers and inputs of 128 bytes or more return EINVAL.

seq_file handles partial reads, offsets and EOF. Read count measures rendered
snapshots rather than every read syscall. A mutex protects the shared state.
Permissions are 0644: readers can inspect it and the owner can write.

## Build and use

~~~sh
make
sudo insmod proc_counter.ko
cat /proc/proc_counter
printf '42\n' | sudo tee /proc/proc_counter
printf 'reset\n' | sudo tee /proc/proc_counter
sudo rmmod proc_counter
~~~

KDIR selects another kernel-header tree. Use headers matching the target kernel
and its module-loading policy. make compiles but does not load the module.

## Integration checks

With the module already loaded in a development system or VM:

~~~sh
sudo python3 tests/check_counter.py
~~~

The script changes the demo state and checks one-byte reads, EOF, signed integer
limits, empty writes and malformed commands. It does not load a module.
CI compilation and a live-kernel integration run are separate checks.

## License

[MIT](LICENSE).

## Author

**Icaro Ramos Rodrigues dos Santos**<br>
Electronic Engineer

Embedded Systems | Firmware | C | C++ | Python | Dart | Assembly | Hardware Design | PCB Design | STM32 | ARM Cortex-M | Linux | Windows | RISC-V | Verilog | SystemVerilog | UVM | Design Verification | Instrumentation | Signal Processing | Qt | FLTK

GitHub: [IcaroRamosDM](https://github.com/IcaroRamosDM)<br>
LinkedIn: [icaro-ramos-r](https://www.linkedin.com/in/icaro-ramos-r/)<br>
[icaroelt@gmail.com](mailto:icaroelt@gmail.com)
