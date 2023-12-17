const std = @import("std");

pub fn tuple(a: u32, b: u32) u64 {
    return @as(u64, @intCast(a)) << 32 | @as(u64, @intCast(b));
}
