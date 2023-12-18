const std = @import("std");
const riscv = @import("riscv.zig");

const Mmu = @import("Mmu.zig");
const State = @import("State.zig");
const XReg = riscv.XReg;

pub const Exit = enum {
    none,
    direct_branch,
    indirect_branch,
    syscall,
};

const Self = @This();

state: State,
mmu: Mmu,

fn load(self: Self) void {
    self.state.pc = self.mmu.entry;
}

fn setup(self: Self) void {
    _ = self;
}

pub fn init() Self {}

pub fn deinit() void {}

pub fn getXReg(self: Self, xreg: XReg) u64 {
    return self.state.xregs[@intFromEnum(xreg)];
}

pub fn setXReg(self: Self, xreg: XReg, value: u64) void {
    self.state.xregs[@intFromEnum(xreg)] = value;
}

pub fn run(self: Self) Exit {
    while (true) {
        self.state.exit = .none;
        self.state.interpret();

        if (self.state.exit == .direct_branch or self.state.exit == .indirect_branch) {
            self.state.pc = self.state.re_enter_pc;
            continue;
        }

        break;
    }

    self.state.pc = self.state.re_enter_pc;
    return .syscall;
}

pub fn doSyscall(self: Self, syscall: u64) u64 {
    _ = self;
    _ = syscall;
}
