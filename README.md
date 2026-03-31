# Kernel Module: proc_counter

Linux Kernel Module that exposes a '/proc' interface for interaction between kernel space and user space.

## Description

This module creates a '/proc/proc_counter' entry that allows:

- Reading internal state from the kernel
- Writing commands to modify behavior
- Tracking R/W operations
- Storing a user-defined integer value

It demonstrates a simple communication mechanism between user space and kernel space using the '/proc' filesystem.

---

## Features

- '/proc' file creation
- Read handler ('cat')
- Write handler ('echo')
- Internal state trancking:
  - Read count
  - Write count
  - Stored value
- Command parsing:
  - 'reset' -> resets all values
  - integer input -> updates stored value
- Mutex-based synchronization

---

## Build Instructions

Make sure you have the required kernel headers installed.
```bash
make
```

## Load and Unload Module

### Load module
```bash
sudo insmod proc_counter.ko
```

### Check if loaded
```bash
lsmod | grep proc_counter
```

### Unload module
```bash
sudo rmmod proc_counter
```

## Usage

### Read data
```bash
cat /proc/proc_counter
```

### Example output:
```bash
Read count: 1
Write count: 0
Stored value: 0
```

### Write a value
```bash
echo [integer_value] | sudo tee /proc/proc_counter
```

### Reset state
```bash
sudo reset | sudo tee /proc/proc_counter
```

### Invalid input
```bash
echo abc | sudo tee /proc/proc_counter
```
This will return an error and log a message in dmesg

## Kernel Logs
```bash
sudo dmesg | tail
```

## Notes

- This module is for educational purposes

# Author
Ícaro Ramos R. dos Santos
