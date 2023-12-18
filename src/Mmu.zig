const std = @import("std");

const Self = @This();

entry: u64,
host_alloc: u64,
alloc: u64,
base: u64,

pub fn loadElf() void {}

pub fn alloc(self: Self, addr: i64) u64 {
    _ = self;
    _ = addr;
}
