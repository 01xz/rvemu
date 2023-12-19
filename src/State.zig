const std = @import("std");

const Instr = @import("instr.zig").Instr;
const Machine = @import("Machine.zig");
const Exit = Machine.Exit;

xregs: [32]u64,
fregs: [32]u64,
pc: u64,
re_enter_pc: u64,
exit: Exit,
cont: bool,

const Self = @This();

pub fn init() Self {
    return .{
        .exit = .none,
        .cont = false,
    };
}

pub fn interpret(self: Self, memory: []u8) void {
    while (true) {
        // instruction fetch
        const instr_raw: u32 = memory[self.pc];

        // instruction decode
        const instr = Instr.decode(instr_raw);

        // instruction execute
        self.execute(instr, memory);

        // reset register `zero`
        self.xregs[0] = 0;

        if (self.cont) break;

        // pc
        self.pc += if (instr.rvc) 2 else 4;
    }
}

fn execute(self: *Self, instr: *const Instr, memory: []u8) void {
    switch (instr.rv_instr) {
        .rv32i => |rv32i| {
            switch (rv32i) {
                .lui => |i| {
                    self.xregs[i.rd] = i.imm;
                },
                .auipc => |i| {
                    self.xregs[i.rd] = self.pc + i.imm;
                },
                .jal => |i| {
                    self.xregs[i.rd] = self.pc + if (instr.rvc) 2 else 4;
                    self.re_enter_pc = self.pc + i.imm;
                    self.exit = .direct_branch;
                    self.cont = true;
                },
                .jalr => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    self.xregs[i.rd] = self.pc + if (instr.rvc) 2 else 4;
                    self.re_enter_pc = (rs1 + i.imm) & 0xffff_ffff_ffff_fffe;
                    self.exit = .indirect_branch;
                    self.cont = true;
                },
                .beq => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const rs2: u64 = self.xregs[i.rs2];
                    if (rs1 == rs2) {
                        self.re_enter_pc = self.pc + i.imm;
                        self.exit = .direct_branch;
                        instr.cont = true;
                    }
                },
                .bne => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const rs2: u64 = self.xregs[i.rs2];
                    if (rs1 != rs2) {
                        self.re_enter_pc = self.pc + i.imm;
                        self.exit = .direct_branch;
                        instr.cont = true;
                    }
                },
                .blt => |i| {
                    const rs1: i64 = @intCast(self.xregs[i.rs1]);
                    const rs2: i64 = @intCast(self.xregs[i.rs2]);
                    if (rs1 < rs2) {
                        self.re_enter_pc = self.pc + i.imm;
                        self.exit = .direct_branch;
                        instr.cont = true;
                    }
                },
                .bge => |i| {
                    const rs1: i64 = @intCast(self.xregs[i.rs1]);
                    const rs2: i64 = @intCast(self.xregs[i.rs2]);
                    if (rs1 >= rs2) {
                        self.re_enter_pc = self.pc + i.imm;
                        self.exit = .direct_branch;
                        instr.cont = true;
                    }
                },
                .bltu => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const rs2: u64 = self.xregs[i.rs2];
                    if (rs1 < rs2) {
                        self.re_enter_pc = self.pc + i.imm;
                        self.exit = .direct_branch;
                        instr.cont = true;
                    }
                },
                .bgeu => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const rs2: u64 = self.xregs[i.rs2];
                    if (rs1 >= rs2) {
                        self.re_enter_pc = self.pc + i.imm;
                        self.exit = .direct_branch;
                        instr.cont = true;
                    }
                },
                .lb => |i| {
                    const addr: u64 = self.xregs[i.rs1] + i.imm;
                    self.xregs[i.rd] = std.mem.bytesToValue(i8, memory[addr..][0..1]);
                },
                .lh => |i| {
                    const addr: u64 = self.xregs[i.rs1] + i.imm;
                    self.xregs[i.rd] = std.mem.bytesToValue(i16, memory[addr..][0..2]);
                },
                .lw => |i| {
                    const addr: u64 = self.xregs[i.rs1] + i.imm;
                    self.xregs[i.rd] = std.mem.bytesToValue(i32, memory[addr..][0..4]);
                },
                .lbu => |i| {
                    const addr: u64 = self.xregs[i.rs1] + i.imm;
                    self.xregs[i.rd] = std.mem.bytesToValue(u8, memory[addr..][0..1]);
                },
                .lhu => |i| {
                    const addr: u64 = self.xregs[i.rs1] + i.imm;
                    self.xregs[i.rd] = std.mem.bytesToValue(u16, memory[addr..][0..2]);
                },
                .sb => |i| {
                    const addr: u64 = self.xregs[i.rs1] + i.imm;
                    std.mem.writeIntSliceLittle(i8, memory[addr..], self.xregs[i.rs2]);
                },
                .sh => |i| {
                    const addr: u64 = self.xregs[i.rs1] + i.imm;
                    std.mem.writeIntSliceLittle(i16, memory[addr..], self.xregs[i.rs2]);
                },
                .sw => |i| {
                    const addr: u64 = self.xregs[i.rs1] + i.imm;
                    std.mem.writeIntSliceLittle(i32, memory[addr..], self.xregs[i.rs2]);
                },
                .addi => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const imm: i64 = @intCast(i.imm);
                    self.xregs[i.rd] = @bitCast(rs1 +% imm);
                },
                .slti => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const imm: i64 = @intCast(i.imm);
                    self.xregs[i.rd] = @as(i64, @bitCast(rs1)) < imm;
                },
                .sltiu => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const imm: i64 = @intCast(i.imm);
                    self.xregs[i.rd] = rs1 < @as(u64, @bitCast(imm));
                },
                .xori => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const imm: i64 = @intCast(i.imm);
                    self.xregs[i.rd] = rs1 ^ imm;
                },
                .ori => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const imm: i64 = @intCast(i.imm);
                    self.xregs[i.rd] = rs1 | imm;
                },
                .andi => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const imm: i64 = @intCast(i.imm);
                    self.xregs[i.rd] = rs1 & imm;
                },
                .slli => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const shamt: u5 = @truncate(i.imm);
                    self.xregs[i.rd] = rs1 << shamt;
                },
                .srli => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const shamt: u5 = @truncate(i.imm);
                    self.xregs[i.rd] = rs1 >> shamt;
                },
                .srai => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const shamt: u5 = @truncate(i.imm);
                    self.xregs[i.rd] = @bitCast(@as(i64, @bitCast(rs1)) >> shamt);
                },
                .add => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const rs2: u64 = self.xregs[i.rs2];
                    self.xregs[i.rd] = rs1 +% rs2;
                },
                .sub => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const rs2: u64 = self.xregs[i.rs2];
                    self.xregs[i.rd] = rs1 -% rs2;
                },
                .sll => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const rs2: u64 = self.xregs[i.rs2];
                    self.xregs[i.rd] = rs1 << @as(u6, @truncate(rs2));
                },
                .slt => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const rs2: u64 = self.xregs[i.rs2];
                    self.xregs[i.rd] = @as(i64, @bitCast(rs1)) < @as(i64, @bitCast(rs2));
                },
                .sltu => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const rs2: u64 = self.xregs[i.rs2];
                    self.xregs[i.rd] = rs1 < rs2;
                },
                .xor => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const rs2: u64 = self.xregs[i.rs2];
                    self.xregs[i.rd] = rs1 ^ rs2;
                },
                .srl => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const rs2: u64 = self.xregs[i.rs2];
                    self.xregs[i.rd] = rs1 >> @as(u6, @truncate(rs2));
                },
                .sra => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const rs2: u64 = self.xregs[i.rs2];
                    self.xregs[i.rd] = @bitCast(@as(i64, @bitCast(rs1)) >> @as(u6, @truncate(rs2)));
                },
                .@"or" => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const rs2: u64 = self.xregs[i.rs2];
                    self.xregs[i.rd] = rs1 | rs2;
                },
                .@"and" => |i| {
                    const rs1: u64 = self.xregs[i.rs1];
                    const rs2: u64 = self.xregs[i.rs2];
                    self.xregs[i.rd] = rs1 & rs2;
                },
                .fence => {},
                .ecall => {
                    self.re_enter_pc = self.pc + 4;
                    self.exit = .syscall;
                    self.cont = true;
                },
                .ebreak => {},
            }
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
