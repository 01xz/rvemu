const std = @import("std");

pub const XLEN = 64;

pub const FLEN = 64;

pub const STACK_SIZE = 32 * 1024 * 1024;

pub const XReg = enum(u5) {
    // zig fmt: off
    zero,                                     // zero
    ra,                                       // return address, caller saved
    sp,                                       // stack pointer, callee saved
    gp,                                       // global pointer
    tp,                                       // thread pointer
    t0, t1, t2,                               // temporaries, caller saved
    s0,                                       // s0/fp, callee saved
    s1,                                       // callee saved
    a0, a1,                                   // fn args/return values, caller saved
    a2, a3, a4, a5, a6, a7,                   // fn args, caller saved
    s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, // saved registers, callee saved
    t3, t4, t5, t6,                           // caller saved
    // zig fmt: on
};

pub const FReg = enum(u5) {
    // zig fmt: off
    ft0, ft1, ft2, ft3, ft4, ft5, ft6, ft7,
    fs0, fs1,
    fa0, fa1, fa2, fa3, fa4, fa5, fa6, fa7,
    fs2, fs3, fs4, fs5, fs6, fs7, fs8, fs9, fs10, fs11,
    ft8, ft9, ft10, ft11,
    // zig fmt: on
};
