const std = @import("std");
const utils = @import("utils.zig");

const tuple = utils.tuple;

pub const Instr = union(enum) {
    rv32i: union(enum) {
        lui: UType,
        auipc: UType,
        jal: JType,
        jalr: IType,
        beq: BType,
        bne: BType,
        blt: BType,
        bge: BType,
        bltu: BType,
        bgeu: BType,
        lb: IType,
        lh: IType,
        lw: IType,
        lbu: IType,
        lhu: IType,
        sb: SType,
        sh: SType,
        sw: SType,
        addi: IType,
        slti: IType,
        sltiu: IType,
        xori: IType,
        ori: IType,
        andi: IType,
        slli: IType,
        srli: IType,
        srai: IType,
        add: RType,
        sub: RType,
        sll: RType,
        slt: RType,
        sltu: RType,
        xor: RType,
        srl: RType,
        sra: RType,
        @"or": RType,
        @"and": RType,
        fence: IType,
        ecall: IType,
        ebreak: IType,

        const Self = @This();

        pub inline fn decode(raw: u32) ?Self {
            const opcode: u5 = @intCast((raw >> 2) & 0b1_1111);
            return switch (opcode) {
                0b0_1101 => .{ .lui = UType.decode(raw) },
                0b0_0101 => .{ .auipc = UType.decode(raw) },
                0b1_1011 => .{ .jal = JType.decode(raw) },
                0b1_1001 => .{ .jalr = IType.decode(raw) },
                0b1_1000 => decodeBranch(raw),
                0b0_0000 => decodeLoad(raw),
                0b0_1000 => decodeStore(raw),
                0b0_0100 => decodeImm(raw),
                0b0_1100 => decodeReg(raw),
                0b0_0011 => .{ .fence = IType.decode(raw) },
                0b1_1100 => decodeSystem(raw),
                else => null,
            };
        }

        inline fn decodeBranch(raw: u32) ?Self {
            const funct3: u3 = @intCast((raw >> 12) & 0b111);
            return switch (funct3) {
                0b000 => .{ .beq = BType.decode(raw) },
                0b001 => .{ .bne = BType.decode(raw) },
                0b100 => .{ .blt = BType.decode(raw) },
                0b101 => .{ .bge = BType.decode(raw) },
                0b110 => .{ .bltu = BType.decode(raw) },
                0b111 => .{ .bgeu = BType.decode(raw) },
                else => null,
            };
        }

        inline fn decodeLoad(raw: u32) ?Self {
            const funct3: u3 = @intCast((raw >> 12) & 0b111);
            return switch (funct3) {
                0b000 => .{ .lb = IType.decode(raw) },
                0b001 => .{ .lh = IType.decode(raw) },
                0b010 => .{ .lw = IType.decode(raw) },
                0b100 => .{ .lbu = IType.decode(raw) },
                0b101 => .{ .lhu = IType.decode(raw) },
                else => null,
            };
        }

        inline fn decodeStore(raw: u32) ?Self {
            const funct3: u3 = @intCast((raw >> 12) & 0b111);
            return switch (funct3) {
                0b000 => .{ .sb = SType.decode(raw) },
                0b001 => .{ .sh = SType.decode(raw) },
                0b010 => .{ .sw = SType.decode(raw) },
                else => null,
            };
        }

        inline fn decodeImm(raw: u32) ?Self {
            const funct3: u3 = @intCast((raw >> 12) & 0b0111);
            const funct7: u7 = @intCast(raw >> 25 & 0b111_1111);
            return switch (funct3) {
                0b000 => .{ .addi = IType.decode(raw) },
                0b010 => .{ .slti = IType.decode(raw) },
                0b011 => .{ .sltiu = IType.decode(raw) },
                0b100 => .{ .xori = IType.decode(raw) },
                0b110 => .{ .ori = IType.decode(raw) },
                0b111 => .{ .andi = IType.decode(raw) },

                0b001 => if (funct7 == 0b0000_0000) .{ .slli = IType.decode(raw) } else null,

                0b101 => switch (funct7) {
                    0b000_0000 => .{ .srli = IType.decode(raw) },
                    0b010_0000 => .{ .srai = IType.decode(raw) },
                    else => null,
                },
            };
        }

        inline fn decodeReg(raw: u32) ?Self {
            const funct3: u3 = @intCast((raw >> 12) & 0b111);
            const funct7: u7 = @intCast((raw >> 25) & 0b111_1111);
            return switch (tuple(funct7, funct3)) {
                tuple(0b000_0000, 0b000) => .{ .add = RType.decode(raw) },
                tuple(0b010_0000, 0b000) => .{ .sub = RType.decode(raw) },
                tuple(0b000_0000, 0b001) => .{ .sll = RType.decode(raw) },
                tuple(0b000_0000, 0b010) => .{ .slt = RType.decode(raw) },
                tuple(0b000_0000, 0b011) => .{ .sltu = RType.decode(raw) },
                tuple(0b000_0000, 0b100) => .{ .xor = RType.decode(raw) },
                tuple(0b000_0000, 0b101) => .{ .srl = RType.decode(raw) },
                tuple(0b010_0000, 0b101) => .{ .sra = RType.decode(raw) },
                tuple(0b000_0000, 0b110) => .{ .@"or" = RType.decode(raw) },
                tuple(0b000_0000, 0b111) => .{ .@"and" = RType.decode(raw) },
                else => null,
            };
        }

        inline fn decodeSystem(raw: u32) ?Self {
            const funct12: u12 = @intCast((raw >> 20) & 0b1111_1111_1111);
            return switch (funct12) {
                0b0000_0000_0000 => .{ .ecall = IType.decode(raw) },
                0b0000_0000_0001 => .{ .ebreak = IType.decode(raw) },
                else => null,
            };
        }
    },

    rv64i: union(enum) {
        lwu: IType,
        ld: IType,
        sd: SType,
        slli: IType,
        srli: IType,
        srai: IType,
        addiw: IType,
        slliw: IType,
        srliw: IType,
        sraiw: IType,
        addw: RType,
        subw: RType,
        sllw: RType,
        srlw: RType,
        sraw: RType,

        const Self = @This();

        pub inline fn decode(raw: u32) ?Self {
            const opcode: u5 = @intCast((raw >> 2) & 0b1_1111);
            return switch (opcode) {
                0b0_0000 => decodeLoad(raw),
                0b0_1000 => decodeStore(raw),
                0b0_0100 => decodeImm(raw),
                0b0_0110 => decodeImmW(raw),
                0b0_1110 => decodeRegW(raw),
            };
        }

        inline fn decodeLoad(raw: u32) ?Self {
            const funct3: u3 = @intCast((raw >> 12) & 0b111);
            return switch (funct3) {
                0b110 => .{ .lwu = IType.decode(raw) },
                0b011 => .{ .ld = IType.decode(raw) },
                else => null,
            };
        }

        inline fn decodeStore(raw: u32) ?Self {
            const funct3: u3 = @intCast((raw >> 12) & 0b111);
            return switch (funct3) {
                0b011 => .{ .sd = SType.decode(raw) },
                else => null,
            };
        }

        inline fn decodeImm(raw: u32) ?Self {
            const funct3: u3 = @intCast((raw >> 12) & 0b0111);
            const funct6: u6 = @intCast(raw >> 26 & 0b11_1111);
            return switch (tuple(funct6, funct3)) {
                tuple(0b00_0000, 0b001) => .{ .slli = IType.decode(raw) },
                tuple(0b00_0000, 0b101) => .{ .srli = IType.decode(raw) },
                tuple(0b01_0000, 0b101) => .{ .srai = IType.decode(raw) },
                else => null,
            };
        }

        inline fn decodeImmW(raw: u32) ?Self {
            const funct3: u3 = @intCast((raw >> 12) & 0b111);
            const funct7: u7 = @intCast((raw >> 25) & 0b111_1111);
            return if (funct3 == 0b000)
                .{ .addiw = IType.decode(raw) }
            else switch (tuple(funct7, funct3)) {
                tuple(0b000_0000, 0b001) => .{ .slliw = IType.decode(raw) },
                tuple(0b000_0000, 0b101) => .{ .srliw = IType.decode(raw) },
                tuple(0b010_0000, 0b101) => .{ .sraiw = IType.decode(raw) },
                else => null,
            };
        }

        inline fn decodeRegW(raw: u32) ?Self {
            const funct3: u3 = @intCast((raw >> 12) & 0b111);
            const funct7: u7 = @intCast((raw >> 25) & 0b111_1111);
            return switch (tuple(funct7, funct3)) {
                tuple(0b000_0000, 0b000) => .{ .addw = RType.decode(raw) },
                tuple(0b010_0000, 0b000) => .{ .subw = RType.decode(raw) },
                tuple(0b000_0000, 0b001) => .{ .sllw = RType.decode(raw) },
                tuple(0b000_0000, 0b101) => .{ .srlw = RType.decode(raw) },
                tuple(0b010_0000, 0b101) => .{ .sraw = RType.decode(raw) },
                else => null,
            };
        }
    },

    zifencei: union(enum) {
        fence_i: IType,

        const Self = @This();

        pub inline fn decode(raw: u32) ?Self {
            _ = raw;
            return .{};
        }
    },

    zicsr: union(enum) {
        csrrw: CSRType,
        csrrs: CSRType,
        csrrc: CSRType,
        csrrwi: CSRType,
        csrrsi: CSRType,
        csrrci: CSRType,

        const Self = @This();

        pub inline fn decode(raw: u32) ?Self {
            _ = raw;
            return .{};
        }
    },

    rv32m: union(enum) {
        mul: RType,
        mulh: RType,
        mulhsu: RType,
        mulhu: RType,
        div: RType,
        divu: RType,
        rem: RType,
        remu: RType,

        const Self = @This();

        pub inline fn decode(raw: u32) ?Self {
            _ = raw;
            return .{};
        }
    },

    rv64m: union(enum) {
        mulw: RType,
        divw: RType,
        divuw: RType,
        remw: RType,
        remuw: RType,

        const Self = @This();

        pub inline fn decode(raw: u32) ?Self {
            _ = raw;
            return .{};
        }
    },

    rv32a: union(enum) {
        lr_w: RType,
        sc_w: RType,
        amoswap_w: RType,
        amoadd_w: RType,
        amoxor_w: RType,
        amoand_w: RType,
        amoor_w: RType,
        amomin_w: RType,
        amomax_w: RType,
        amominu_w: RType,
        amomaxu_w: RType,

        const Self = @This();

        pub inline fn decode(raw: u32) ?Self {
            _ = raw;
            return .{};
        }
    },

    rv64a: union(enum) {
        lr_d: RType,
        sc_d: RType,
        amoswap_d: RType,
        amoadd_d: RType,
        amoxor_d: RType,
        amoand_d: RType,
        amoor_d: RType,
        amomin_d: RType,
        amomax_d: RType,
        amominu_d: RType,
        amomaxu_d: RType,

        const Self = @This();

        pub inline fn decode(raw: u32) ?Self {
            _ = raw;
            return .{};
        }
    },

    rv32f: union(enum) {
        flw: IType,
        fsw: SType,
        fmadd_s: R4Type,
        fmsub_s: R4Type,
        fnmsub_s: R4Type,
        fnmadd_s: R4Type,
        fadd_s: RType,
        fsub_s: RType,
        fmul_s: RType,
        fdiv_s: RType,
        fsqrt_s: RType,
        fsgnj_s: RType,
        fsgnjn_s: RType,
        fsgnjx_s: RType,
        fmin_s: RType,
        fmax_s: RType,
        fcvt_w_s: RType,
        fcvt_wu_s: RType,
        fmv_x_w: RType,
        feq_s: RType,
        flt_s: RType,
        fle_s: RType,
        fclass_s: RType,
        fcvt_s_w: RType,
        fcvt_s_wu: RType,
        fmv_w_x: RType,

        const Self = @This();

        pub inline fn decode(raw: u32) ?Self {
            _ = raw;
            return .{};
        }
    },

    rv64f: union(enum) {
        fcvt_l_s: RType,
        fcvt_lu_s: RType,
        fcvt_s_l: RType,
        fcvt_s_lu: RType,

        const Self = @This();

        pub inline fn decode(raw: u32) ?Self {
            _ = raw;
            return .{};
        }
    },

    rv32d: union(enum) {
        fld: IType,
        fsd: SType,
        fmadd_d: R4Type,
        fmsub_d: R4Type,
        fnmsub_d: R4Type,
        fnmadd_d: R4Type,
        fadd_d: RType,
        fsub_d: RType,
        fmul_d: RType,
        fdiv_d: RType,
        fsqrt_d: RType,
        fsgnj_d: RType,
        fsgnjn_d: RType,
        fsgnjx_d: RType,
        fmin_d: RType,
        fmax_d: RType,
        fcvt_s_d: RType,
        fcvt_d_s: RType,
        feq_d: RType,
        flt_d: RType,
        fle_d: RType,
        fclass_d: RType,
        fcvt_w_d: RType,
        fcvt_wu_d: RType,
        fcvt_d_w: RType,
        fcvt_d_wu: RType,

        const Self = @This();

        pub inline fn decode(raw: u32) ?Self {
            _ = raw;
            return .{};
        }
    },

    rv64d: union(enum) {
        fcvt_l_d: RType,
        fcvt_lu_d: RType,
        fmv_x_d: RType,
        fcvt_d_l: RType,
        fcvt_d_lu: RType,
        fmv_d_x: RType,

        const Self = @This();

        pub inline fn decode(raw: u32) ?Self {
            _ = raw;
            return .{};
        }
    },

    privileged: union(enum) {
        mret: IType,

        const Self = @This();

        pub inline fn decode(raw: u32) ?Self {
            _ = raw;
            return .{};
        }
    },

    pub fn decode(raw: u32) Instr {
        const quadrant: u2 = @intCast(raw & 0b11);
        return switch (quadrant) {
            0b11 => decodeNotRvc(raw),
            else => decodeRvc(raw),
        };
    }

    inline fn decodeNotRvc(raw: u32) Instr {
        _ = raw;
        return .{};
    }

    inline fn decodeRvc(raw: u16) Instr {
        _ = raw;
        return .{};
    }
};

const RType = struct {
    rs1: u5,
    rs2: u5,
    rd: u5,

    const Self = @This();

    pub fn decode(raw: u32) Self {
        const un: packed union {
            raw: u32,
            rtype: packed struct { opcode: u7, rd: u5, funct3: u3, rs1: u5, rs2: u5, funct7: u7 },
        } = .{ .raw = raw };
        return .{
            .rs1 = un.rtype.rs1,
            .rs2 = un.rtype.rs2,
            .rd = un.rtype.rd,
        };
    }
};

const R4Type = struct {
    rs1: u5,
    rs2: u5,
    rs3: u5,
    rd: u5,

    const Self = @This();

    pub fn decode(raw: u32) Self {
        const un: packed union {
            raw: u32,
            r4type: packed struct { opcode: u7, rd: u5, funct3: u3, rs1: u5, rs2: u5, funct2: u2, rs3: u5 },
        } = .{ .raw = raw };
        return .{
            .rs1 = un.r4type.rs1,
            .rs2 = un.r4type.rs2,
            .rs2 = un.r4type.rs2,
            .rd = un.r4type.rd,
        };
    }
};

const IType = struct {
    rs1: u5,
    rd: u5,
    imm: i32,

    const Self = @This();

    pub fn decode(raw: u32) Self {
        const un: packed union {
            raw: u32,
            itype: packed struct { opcode: u7, rd: u5, funct3: u3, rs1: u5, imm12_11_0: u12 },
        } = .{ .raw = raw };
        return .{
            .rs1 = un.itype.rs1,
            .rd = un.itype.rd,
            .imm = @intCast(@as(i12, @bitCast(un.itype.imm12_11_0))),
        };
    }
};

const CSRType = struct {
    rs1: u5,
    rd: u5,
    csr: u12,

    const Self = @This();

    pub fn decode(raw: u32) Self {
        const un: packed union {
            raw: u32,
            csrtype: packed struct { opcode: u7, rd: u5, funct3: u3, rs1: u5, imm12_11_0: u12 },
        } = .{ .raw = raw };
        return .{
            .rs1 = un.csrtype.rs1,
            .rd = un.csrtype.rd,
            .imm = un.csrtype.imm12_11_0,
        };
    }
};

const SType = struct {
    rs1: u5,
    rs2: u5,
    imm: i32,

    const Self = @This();

    pub fn decode(raw: u32) Self {
        const un: packed union {
            raw: u32,
            stype: packed struct { opcode: u7, imm12_4_0: u5, funct3: u3, rs1: u5, rs2: u5, imm12_11_5: u7 },
        } = .{ .raw = raw };
        return .{
            .rs1 = un.stype.rs1,
            .rs2 = un.stype.rs1,
            .imm = @intCast(@as(i12, @intCast(un.stype.imm12_11_5)) << 5 | @as(i12, @intCast(un.stype.imm12_4_0))),
        };
    }
};

const BType = struct {
    rs1: u5,
    rs2: u5,
    imm: i32,

    const Self = @This();

    pub fn decode(raw: u32) Self {
        const un: packed union {
            raw: u32,
            btype: packed struct { opcode: u7, imm13_11_11: u1, imm13_4_1: u4, funct3: u3, rs1: u5, rs2: u5, imm13_10_5: u6, imm13_12_12: u1 },
        } = .{ .raw = raw };
        return .{
            .rs1 = un.btype.rs1,
            .rs2 = un.btype.rs1,
            .imm = @intCast(@as(i13, @intCast(un.btype.imm13_12_12)) << 12 |
                @as(i13, @intCast(un.btype.imm13_11_11)) << 11 |
                @as(i13, @intCast(un.btype.imm13_10_5)) << 5 |
                @as(i13, @intCast(un.btype.imm13_4_1)) << 1),
        };
    }
};

const UType = struct {
    rd: u5,
    imm: i32,

    const Self = @This();

    pub fn decode(raw: u32) Self {
        const un: packed union {
            raw: u32,
            utype: packed struct { opcode: u7, rd: u5, imm32_31_12: u20 },
        } = .{ .raw = raw };
        return .{
            .rd = un.utype.rd,
            .imm = @intCast(@as(i32, @intCast(un.utype.imm32_31_12)) << 12),
        };
    }
};

const JType = struct {
    rd: u5,
    imm: i32,

    const Self = @This();

    pub fn decode(raw: u32) Self {
        const un: packed union {
            raw: u32,
            jtype: packed struct { opcode: u7, rd: u5, imm21_19_12: u8, imm21_11_11: u1, imm21_10_1: u10, imm21_20_20: u1 },
        } = .{ .raw = raw };
        return .{
            .rd = un.jtype.rd,
            .imm = @intCast(@as(i21, @intCast(un.jtype.imm21_20_20)) << 20 |
                @as(i21, @intCast(un.jtype.imm21_19_12)) << 12 |
                @as(i21, @intCast(un.jtype.imm21_11_11)) << 11 |
                @as(i21, @intCast(un.jtype.imm21_10_1)) << 1),
        };
    }
};
