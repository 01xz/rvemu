const std = @import("std");
const riscv = @import("riscv.zig");

const Machine = @import("Machine.zig");
const XReg = riscv.XReg;

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();

    const allocator = gpa.allocator();

    var args = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    var machine = Machine.init(args[1]);
    defer machine.deinit();

    while (true) {
        machine.run();
        const syscall = machine.getXReg(.a7);
        const ret = machine.doSyscall(syscall);
        machine.setXReg(.a0, ret);
    }
}
