//! The stage 2 of the bootloader.
//! 
//! The current state of the system -
//!   - 32-bit protected mode enabled
//!   - A20 line enabled
//!   - Number of memory map regions stored in a uint64_t at 0x10000
//!   - Memory map stored in an array starting from 0x10008
//!   - 4 16-bit partition table entries from 0x7dbe to 0x7dfe
//!   - Stack pointer at 0x7c00

#![no_std]
#![feature(lang_items)]
#![cfg(target_arch = "x86")]

mod mem;
pub use mem::*;

#[no_mangle]
pub extern fn rust_main() {
    let buf: [u16; 2] = [ 0x2f4f, 0x2f4b ];
    let ptr = 0xb8000 as *mut _;
    unsafe { *ptr = buf };
    loop { }
}


// Language items required by Rust
#[lang = "eh_personality"]
#[no_mangle]
pub extern fn eh_personality() {}

#[lang = "panic_fmt"]
#[no_mangle]
pub extern fn panic_fmt() -> ! {
    loop {}
}
