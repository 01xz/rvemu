const std = @import("std");
const instr = @import("instr.zig");

const Instr = instr.Instr;
const Machine = @import("Machine.zig");
const Exit = Machine.Exit;

xregs: [32]u64,
fregs: [32]u64,
pc: u64,
re_enter_pc: u64,
exit: Exit,
cont: bool,

const Self = @This();

pub fn interpret(self: Self) void {
    while (true) {
        // instruction fetch
        const rv_instr_raw: u32 = *self.pc;

        // instruction decode
        const rv_instr = Instr.decode(rv_instr_raw);

        // instruction execute
        self.execute(rv_instr);

        // reset register `zero`
        self.xregs[0] = 0;

        if (self.cont) break;

        // pc
        self.pc += if (rv_instr.rvc) 2 else 4;
    }
}

fn execute(self: Self, rv_instr: Instr) void {
    _ = self;
    switch (rv_instr) {
        .rv32i => |rv32i| {
            switch (rv32i) {}
        },
        .rv64i => |rv64i| {
            switch (rv64i) {}
        },
        .rv32m => |rv32m| {
            switch (rv32m) {}
        },
        .rv64m => |rv64m| {
            switch (rv64m) {}
        },
        .rv32f => |rv32f| {
            switch (rv32f) {}
        },
        .rv64f => |rv64f| {
            switch (rv64f) {}
        },
        .rv32d => |rv32d| {
            switch (rv32d) {}
        },
        .rv64d => |rv64d| {
            switch (rv64d) {}
        },
        else => {},
    }
}
