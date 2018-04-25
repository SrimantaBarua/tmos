//! Memory manipulation functions that rust needs

/// Copy `n` bytes from `src` to `dst`. No checking for overlap
#[no_mangle]
pub unsafe extern "C" fn memcpy(dst: *mut u8, src: *const u8, n: usize) -> *mut u8 {
    let mut i = 0;
    if ((dst as usize) & 3) == 0 && ((src as usize) & 3) == 0 && (n & 3) == 0 {
        let dstptr: *mut u32 = dst as *mut u32;
        let srcptr: *const u32 = src as *const u32;
        let newn = n >> 2;
        while i < newn {
            *dstptr.offset(i as isize) = *srcptr.offset(i as isize);
            i += 1;
        }
        return dst;
    }
    while i < n {
        *dst.offset(i as isize) = *src.offset(i as isize);
        i += 1;
    }
    dst
}

/// Copy `n` bytes from `src` to `dst`. Checks for and handles overlap
#[no_mangle]
pub unsafe extern "C" fn memmove(dst: *mut u8, src: *const u8, n: usize) -> *mut u8 {
    if (dst as usize) < (src as usize) {
        return memcpy(dst, src, n);
    }
    if ((dst as usize) & 3) == 0 && ((src as usize) & 3) == 0 && (n & 3) == 0 {
        let dstptr: *mut u32 = dst as *mut u32;
        let srcptr: *const u32 = src as *const u32;
        let newn = n >> 2;
        let mut i = newn;
        while i > 0 {
            i -= 1;
            *dstptr.offset(i as isize) = *srcptr.offset(i as isize);
        }
        return dst;
    }
    let mut i = n;
    while i > 0 {
        i -= 1;
        *dst.offset(i as isize) = *src.offset(i as isize);
    }
    dst
}


/// Set `n` bytes of memory at `dst` to the lower byte of `val`
#[no_mangle]
pub unsafe extern "C" fn memset(dst: *mut u8, val: i32, n: usize) -> *mut u8 {
    let byte: u8 = (val & 0xff) as u8;
    let mut i = 0;
    if ((dst as usize) & 3) == 0 && (n & 3) == 0 {
        let dstptr: *mut u32 = dst as *mut u32;
        let newbyte: u32 = byte as u32;
        let newval: u32 = (newbyte << 24) | (newbyte << 16) | (newbyte << 8) | newbyte;
        let newn = n >> 2;
        while i < newn {
            *dstptr.offset(i as isize) = newval;
            i += 1;
        }
        return dst;
    }
    while i < n {
        *dst.offset(i as isize) = byte;
        i += 1;
    }
    dst
}

/// Compare `n` bytes of memory at `m1` and `m2`
#[no_mangle]
pub unsafe extern "C" fn memcmp(m1: *const u8, m2: *const u8, n: usize) -> i32 {
    let mut i = 0;
    while i <  n {
        let a = *m1.offset(i as isize);
        let b = *m2.offset(i as isize);
        if a != b {
            return  a as i32 - b as i32;
        }
        i += 1
    }
    0
}
