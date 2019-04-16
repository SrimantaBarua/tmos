The tmos bootloader
--------------------

Why write a custom bootloader? Well, why write a custom OS at all?
For educational purposes.

That's why I'm writing this OS. To learn, myself, and also so as to act as a source for others to learn systems programming.

The bootloader is initially only going to support booting from MBR-based devices. Later, I might add in UEFI compatibility.

I'm planning to make the bootloader multiboot2 compliant, so that development on the OS can go on independent of the bootloader.
