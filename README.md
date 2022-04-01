# sy42-tp

My solutions to the assignements in the SY42 class.
These were made using [`adri326/no-stm32cube`](https://github.com/adri326/no-stm32cube), see instructions there if you want to make heavy modifications to the code or are having trouble compiling it.

Note that I only post here the assignements that were already due.
For that reason, I'm fine with anyone re-using the code of these previous assignements in future assignements.
The [license](LICENSE.txt) has to stick around, though.

## Installation

Following is a summary of the packages you will need to install (on Arch Linux).
On other package repos, the names should be similar.

If you are on windows, then... good luck. I'm not sure if you can do anything without stm32cube.

- `arm-none-eabi-gcc`, for the compiler itself
- `arm-none-eabi-newlib`, for the `nano.specs` specification file, used by the compiler
- `arm-none-eabi-gdb`, for debugging
- `stm32f4-headers-git` (AUR), for the header files
- `stlink`, for interacting with the chip
- `openocd`, for debugging the chip
- `base-devel`, for `git` and `make`
- `screen` (optional), for interacting with the chip through UART; any tool will do

First, clone this repository:

```sh
git clone https://github.com/adri326/sy42-tp/
cd sy42-tp
```

Then, choose which assignement you want to execute, and navigate into it (`cd tp3`, for instance).

Finally, run `make` to compile the code that's in the `src/` directory and `make flash` to flash the binary on the chip.

To interface with the chip through UART, run `screen /dev/TTYASM0` (or whatever device in `/dev` the chip was attached to).

To debug the code, run `make openocd` on one terminal, and `make gdb` on another.
`make gdb` requires that `make openocd` is running in the background.

If you only have/want one terminal for debugging, then you can run `make openocd` in a detached instance of `screen` (`Ctrl+A, D` to detach and `screen -R` to reattach).
