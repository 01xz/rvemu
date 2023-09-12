#include "decode.h"

#include <assert.h>

#include "utils.h"

typedef struct {
  u32 opcode : 7;
  u32 rd : 5;
  u32 funct3 : 3;
  u32 rs1 : 5;
  u32 rs2 : 5;
  u32 funct7 : 7;
} RvInstrR;

typedef struct {
  u32 opcode : 7;
  u32 rd : 5;
  u32 funct3 : 3;
  u32 rs1 : 5;
  u32 rs2 : 5;
  u32 funct2 : 2;
  u32 rs3 : 5;
} RvInstrR4;

typedef struct {
  u32 opcode : 7;
  u32 rd : 5;
  u32 funct3 : 3;
  u32 rs1 : 5;
  u32 imm11_0 : 12;
} RvInstrI;

typedef struct {
  u32 opcode : 7;
  u32 imm4_0 : 5;
  u32 funct3 : 3;
  u32 rs1 : 5;
  u32 rs2 : 5;
  u32 imm11_5 : 7;
} RvInstrS;

typedef struct {
  u32 opcode : 7;
  u32 imm11 : 1;
  u32 imm4_1 : 4;
  u32 funct3 : 3;
  u32 rs1 : 5;
  u32 rs2 : 5;
  u32 imm10_5 : 6;
  u32 imm12 : 1;
} RvInstrB;

typedef struct {
  u32 opcode : 7;
  u32 rd : 5;
  u32 imm31_12 : 20;
} RvInstrU;

typedef struct {
  u32 opcode : 7;
  u32 rd : 5;
  u32 imm19_12 : 8;
  u32 imm11 : 1;
  u32 imm10_1 : 10;
  u32 imm20 : 1;
} RvInstrJ;

typedef struct {
  u32 : 16;
  u32 funct4 : 4;
  u32 rs1 : 5;
  u32 rs2 : 5;
  u32 op : 2;
} RvInstrCR;

typedef struct {
  u32 : 16;
  u32 funct3 : 3;
  u32 imm1 : 1;
  u32 rs1 : 5;
  u32 imm5 : 5;
  u32 op : 2;
} RvInstrCI;

typedef struct {
  u32 : 16;
  u32 funct3 : 3;
  u32 imm : 6;
  u32 rs2 : 5;
  u32 op : 2;
} RvInstrCSS;

typedef struct {
  u32 : 16;
  u32 funct3 : 3;
  u32 imm : 8;
  u32 rd : 3;
  u32 op : 2;
} RvInstrCIW;

typedef struct {
  u32 : 16;
  u32 funct3 : 3;
  u32 imm3 : 3;
  u32 rs1 : 3;
  u32 imm2 : 2;
  u32 rd : 3;
  u32 op : 2;
} RvInstrCL;

typedef struct {
  u32 : 16;
  u32 funct3 : 3;
  u32 imm3 : 3;
  u32 rs1 : 3;
  u32 imm2 : 2;
  u32 rs2 : 3;
  u32 op : 2;
} RvInstrCS;

typedef struct {
  u32 : 16;
  u32 funct6 : 6;
  u32 rs1 : 3;
  u32 funct2 : 2;
  u32 rs2 : 3;
  u32 op : 2;
} RvInstrCA;

typedef struct {
  u32 : 16;
  u32 funct3 : 3;
  u32 offset3 : 3;
  u32 rs1 : 3;
  u32 offset5 : 5;
  u32 op : 2;
} RvInstrCB;

typedef struct {
  u32 : 16;
  u32 funct3 : 3;
  u32 target : 11;
  u32 op : 2;
} RvInstrCJ;

typedef struct {
  u32 quadrant : 2;
  u32 opcode : 5;
  u32 : 6;
  u32 instr15_13 : 3;
  u32 : 4;
  u32 instr24_20 : 5;
  u32 instr31_25 : 7;
} RvInstrG;

typedef union {
  RvInstrR rtype;
  RvInstrR4 r4type;
  RvInstrI itype;
  RvInstrS stype;
  RvInstrB btype;
  RvInstrU utype;
  RvInstrJ jtype;
  RvInstrG gtype;
  RvInstrCR crtype;
  RvInstrCI citype;
  RvInstrCSS csstype;
  RvInstrCIW ciwtype;
  RvInstrCL cltype;
  RvInstrCS cstype;
  RvInstrCA catype;
  RvInstrCB cbtype;
  RvInstrCJ cjtype;
  u32 raw;
} RvInstrUn;

static inline i32 get_i_type_imm(const RvInstrUn *un) {
  return (i32)un->raw >> 20;
}

static inline i32 get_s_type_imm(const RvInstrUn *un) {
  i32 imm = (un->stype.imm11_5 << 5) | un->stype.imm4_0;
  return (imm << 20) >> 20;
}

static inline i32 get_b_type_imm(const RvInstrUn *un) {
  i32 imm = (un->btype.imm12 << 12) | (un->btype.imm11 << 11) |
            (un->btype.imm10_5 << 5) | (un->btype.imm4_1 << 1);
  return (imm << 19) >> 19;
}

static inline i32 get_u_type_imm(const RvInstrUn *un) {
  return (i32)un->raw & 0xfffff000;
}

static inline i32 get_j_type_imm(const RvInstrUn *un) {
  i32 imm = (un->jtype.imm20 << 20) | (un->jtype.imm19_12 << 12) |
            (un->jtype.imm11 << 11) | (un->jtype.imm10_1 << 1);
  return (imm << 11) >> 11;
}

void rv_instr_decode(RvInstr* instr, u32 instr_raw) {
  instr->raw = instr_raw;
  switch (instr->gtype.quadrant) {
    case 0x0:
      FATAL("TODO");
      break;
    case 0x1:
      FATAL("TODO");
      break;
    case 0x2:
      FATAL("TODO");
      break;
    case 0x3: {  // not rvc
      instr->rvc = false;
      switch (instr->gtype.opcode) {
        case 0x0: {  // I-type
          instr->imm = get_i_type_imm(instr);
          switch (instr->itype.funct3) {
            case 0x0:  // LB
              instr->type = kLb;
              return;
            case 0x1:  // LH
              instr->type = kLh;
              return;
            case 0x2:  // LW
              instr->type = kLw;
              return;
            case 0x3:  // LD
              instr->type = kLd;
              return;
            case 0x4:  // LBU
              instr->type = kLbu;
              return;
            case 0x5:  // LHU
              instr->type = kLhu;
              return;
            case 0x6:  // LWU
              instr->type = kLwu;
              return;
            default:
              UNREACHABLE();
          }  // switch funct3
          UNREACHABLE();
        }  // opcode case 0x0

        case 0x1: {  // I-type
          instr->imm = get_i_type_imm(instr);
          switch (instr->itype.funct3) {
            case 0x2:  // FLW
              instr->type = kFlw;
              return;
            case 0x3:  // FLD
              instr->type = kFld;
              return;
            default:
              UNREACHABLE();
          }  // switch funct3
          UNREACHABLE();
        }  // opcode case 0x1

        case 0x3: {  // I-type
          instr->imm = get_i_type_imm(instr);
          switch (instr->itype.funct3) {
            case 0x0:  // FENCE
              instr->type = kFence;
              return;
            case 0x1:  // FENCE.I
              instr->type = kFenceI;
              return;
            default:
              UNREACHABLE();
          }  // switch funct3
          UNREACHABLE();
        }  // opcode case 0x3

        case 0x4: {  // I-type
          instr->imm = get_i_type_imm(instr);
          switch (instr->itype.funct3) {
            case 0x0:  // ADDI
              instr->type = kAddi;
              return;
            case 0x1:
              if ((instr->gtype.instr31_25 >> 1) == 0x0) {  // SLLI
                instr->type = kSlli;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x2:  // SLTI
              instr->type = kSlti;
              return;
            case 0x3:  // SLTIU
              instr->type = kSltiu;
              return;
            case 0x4:  // XORI
              instr->type = kXori;
              return;
            case 0x5:
              if ((instr->gtype.instr31_25 >> 1) == 0x0) {  // SRLI
                instr->type = kSrli;
              } else if ((instr->gtype.instr31_25 >> 1) == 0x10) {  // SRAI
                instr->type = kSrai;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x6:  // ORI
              instr->type = kOri;
              return;
            case 0x7:  // ANDI
              instr->type = kAndi;
              return;
            default:
              UNREACHABLE();
          }  // switch funct3
          UNREACHABLE();
        }  // opcode case 0x4

        case 0x5: {  // U-type: AUIPC
          instr->imm = get_u_type_imm(instr);
          instr->type = kAuipc;
          return;
        }  // opcode case 0x5

        case 0x6: {  // I-type
          instr->imm = get_i_type_imm(instr);
          switch (instr->itype.funct3) {
            case 0x0:  // ADDIW
              instr->type = kAddiw;
              return;
            case 0x1:  // SLLIW
              assert(instr->gtype.instr31_25 == 0x0);
              instr->type = kSlliw;
              return;
            case 0x5:
              if (instr->gtype.instr31_25 == 0x0) {  // SRLIW
                instr->type = kSrliw;
              } else if (instr->gtype.instr31_25 == 0x20) {  // SRAIW
                instr->type = kSraiw;
              } else {
                UNREACHABLE();
              }
              return;
            default:
              UNREACHABLE();
          }  // switch funct3
          UNREACHABLE();
        }  // opcode case 0x6

        case 0x8: {  // S-type
          instr->imm = get_s_type_imm(instr);
          switch (instr->stype.funct3) {
            case 0x0:  // SB
              instr->type = kSb;
              return;
            case 0x1:  // SH
              instr->type = kSh;
              return;
            case 0x2:  // SW
              instr->type = kSw;
              return;
            case 0x3:  // SD
              instr->type = kSd;
              return;
            default:
              UNREACHABLE();
          }  // switch funct3
          UNREACHABLE();
        }  // opcode case 0x8

        case 0x9: {  // S-type
          instr->imm = get_s_type_imm(instr);
          switch (instr->stype.funct3) {
            case 0x2:  // FSW
              instr->type = kFsw;
              return;
            case 0x3:  // FSD
              instr->type = kFsd;
              return;
            default:
              UNREACHABLE();
          }  // switch funct3
          UNREACHABLE();
        }  // case 0x9

        case 0xc: {  // R-type
          switch (instr->rtype.funct3) {
            case 0x0:
              if (instr->rtype.funct7 == 0x0) {  // ADD
                instr->type = kAdd;
              } else if (instr->rtype.funct7 == 0x1) {  // MUL
                instr->type = kMul;
              } else if (instr->rtype.funct7 == 0x20) {  // SUB
                instr->type = kSub;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x1:
              if (instr->rtype.funct7 == 0x0) {  // SLL
                instr->type = kSll;
              } else if (instr->rtype.funct7 == 0x1) {  // MULH
                instr->type = kMulh;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x2:
              if (instr->rtype.funct7 == 0x0) {  // SLT
                instr->type = kSlt;
              } else if (instr->rtype.funct7 == 0x1) {  // MULHSU
                instr->type = kMulhsu;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x3:
              if (instr->rtype.funct7 == 0x0) {  // SLTU
                instr->type = kSltu;
              } else if (instr->rtype.funct7 == 0x1) {  // MULHU
                instr->type = kMulhu;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x4:
              if (instr->rtype.funct7 == 0x0) {  // XOR
                instr->type = kXor;
              } else if (instr->rtype.funct7 == 0x1) {  // DIV
                instr->type = kDiv;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x5:
              if (instr->rtype.funct7 == 0x0) {  // SRL
                instr->type = kSrl;
              } else if (instr->rtype.funct7 == 0x1) {  // DIVU
                instr->type = kDivu;
              } else if (instr->rtype.funct7 == 0x20) {  // SRA
                instr->type = kSra;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x6:
              if (instr->rtype.funct7 == 0x0) {  // OR
                instr->type = kOr;
              } else if (instr->rtype.funct7 == 0x1) {  // REM
                instr->type = kRem;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x7:
              if (instr->rtype.funct7 == 0x0) {  // AND
                instr->type = kAnd;
              } else if (instr->rtype.funct7 == 0x1) {  // REMU
                instr->type = kRemu;
              } else {
                UNREACHABLE();
              }
              return;
            default:
              UNREACHABLE();
          }  // switch funct3
          UNREACHABLE();
        }  // case 0xc

        case 0xd: {  // U-type: LUI
          instr->imm = get_u_type_imm(instr);
          instr->type = kLui;
          return;
        }  // case 0xd

        case 0xe: {  // R-type
          switch (instr->rtype.funct3) {
            case 0x0:
              if (instr->rtype.funct7 == 0x0) {  // ADDW
                instr->type = kAddw;
              } else if (instr->rtype.funct7 == 0x1) {  // MULW
                instr->type = kMulw;
              } else if (instr->rtype.funct7 == 0x20) {  // SUBW
                instr->type = kSubw;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x1:
              if (instr->rtype.funct7 == 0x0) {  // SLLW
                instr->type = kSllw;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x4:
              if (instr->rtype.funct7 == 0x1) {  // DIVW
                instr->type = kDivw;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x5:
              if (instr->rtype.funct7 == 0x0) {  // SRLW
                instr->type = kSrlw;
              } else if (instr->rtype.funct7 == 0x1) {  // DIVUW
                instr->type = kDivuw;
              } else if (instr->rtype.funct7 == 0x20) {  // SRAW
                instr->type = kSraw;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x6:
              if (instr->rtype.funct7 == 0x1) {  // REMW
                instr->type = kRemw;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x7:
              if (instr->rtype.funct7 == 0x1) {  // REMUW
                instr->type = kRemuw;
              } else {
                UNREACHABLE();
              }
              return;
            default:
              UNREACHABLE();
          }  // switch funct3
          UNREACHABLE();
        }  // case 0xe

        case 0x10: {  // R4-type
          switch (instr->r4type.funct2) {
            case 0x0:  // FMADD.S
              instr->type = kFmaddS;
              return;
            case 0x1:  // FMADD.D
              instr->type = kFmaddD;
              return;
            default:
              UNREACHABLE();
          }  // switch funct2
          UNREACHABLE();
        }  // case 0x10

        case 0x11: {  // R4-type
          switch (instr->r4type.funct2) {
            case 0x0:  // FMSUB.S
              instr->type = kFmsubS;
              return;
            case 0x1:  // FMSUB.D
              instr->type = kFmsubD;
              return;
            default:
              UNREACHABLE();
          }  // switch funct2
          UNREACHABLE();
        }  // case 0x11

        case 0x12: {  // R4-type
          switch (instr->r4type.funct2) {
            case 0x0:  // FNMSUB.S
              instr->type = kFnmsubS;
              return;
            case 0x1:  // FNMSUB.D
              instr->type = kFnmsubD;
              return;
            default:
              UNREACHABLE();
          }  // switch funct2
          UNREACHABLE();
        }  // case 0x12

        case 0x13: {  // R4-type
          switch (instr->r4type.funct2) {
            case 0x0:  // FNMADD.S
              instr->type = kFnmaddS;
              return;
            case 0x1:  // FNMADD.D
              instr->type = kFnmaddD;
              return;
            default:
              UNREACHABLE();
          }  // switch funct2
          UNREACHABLE();
        }  // case 0x13

        case 0x14: {
          switch (instr->rtype.funct7) {
            case 0x0:  // FADD.S
              instr->type = kFaddS;
              return;
            case 0x1:  // FADD.D
              instr->type = kFaddD;
              return;
            case 0x4:  // FSUB.S
              instr->type = kFsubS;
              return;
            case 0x5:  // FSUB.D
              instr->type = kFsubD;
              return;
            case 0x8:  // FMUL.S
              instr->type = kFmulS;
              return;
            case 0x9:  // FMUL.D
              instr->type = kFmulD;
              return;
            case 0xc:  // FDIV.S
              instr->type = kFdivS;
              return;
            case 0xd:  // FDIV.D
              instr->type = kFdivD;
              return;
            case 0x10:
              if (instr->rtype.funct3 == 0x0) {  // FSGNJ.S
                instr->type = kFsgnjS;
              } else if (instr->rtype.funct3 == 0x1) {  // FSNGJN.S
                instr->type = kFsgnjnS;
              } else if (instr->rtype.funct3 == 0x2) {  // FSNGJX.S
                instr->type = kFsgnjxS;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x11:
              if (instr->rtype.funct3 == 0x0) {  // FSGNJ.D
                instr->type = kFsgnjD;
              } else if (instr->rtype.funct3 == 0x1) {  // FSNGJN.D
                instr->type = kFsgnjnD;
              } else if (instr->rtype.funct3 == 0x2) {  // FSNGJX.D
                instr->type = kFsgnjxD;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x14:
              if (instr->rtype.funct3 == 0x0) {  // FMIN.S
                instr->type = kFminS;
              } else if (instr->rtype.funct3 == 0x1) {  // FMAX.S
                instr->type = kFmaxS;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x15:
              if (instr->rtype.funct3 == 0x0) {  // FMIN.D
                instr->type = kFminD;
              } else if (instr->rtype.funct3 == 0x1) {  // FMAX.D
                instr->type = kFmaxD;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x20:  // FCVT.S.D
              assert(instr->rtype.rs2 == 0x1);
              instr->type = kFcvtSD;
              return;
            case 0x21:  // FCVT.D.S
              assert(instr->rtype.rs2 == 0x0);
              instr->type = kFcvtDS;
              return;
            case 0x2c:  // FSQRT.S
              assert(instr->rtype.rs2 == 0x0);
              instr->type = kFsqrtS;
              return;
            case 0x2d:  // FSQRT.D
              assert(instr->rtype.rs2 == 0x0);
              instr->type = kFsqrtD;
              return;
            case 0x50:
              if (instr->rtype.funct3 == 0x0) {  // FLE.S
                instr->type = kFleS;
              } else if (instr->rtype.funct3 == 0x1) {  // FLT.S
                instr->type = kFltS;
              } else if (instr->rtype.funct3 == 0x2) {  // FEQ.S
                instr->type = kFeqS;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x51:
              if (instr->rtype.funct3 == 0x0) {  // FLE.D
                instr->type = kFleD;
              } else if (instr->rtype.funct3 == 0x1) {  // FLT.D
                instr->type = kFltD;
              } else if (instr->rtype.funct3 == 0x2) {  // FEQ.D
                instr->type = kFeqD;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x60:
              if (instr->rtype.rs2 == 0x0) {  // FCVT.W.S
                instr->type = kFcvtWS;
              } else if (instr->rtype.rs2 == 0x1) {  // FCVT.WU.S
                instr->type = kFcvtWuS;
              } else if (instr->rtype.rs2 == 0x2) {  // FCVT.L.S
                instr->type = kFcvtLS;
              } else if (instr->rtype.rs2 == 0x3) {  // FCVT.LU.S
                instr->type = kFcvtLuS;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x61:
              if (instr->rtype.rs2 == 0x0) {  // FCVT.W.D
                instr->type = kFcvtWD;
              } else if (instr->rtype.rs2 == 0x1) {  // FCVT.WU.D
                instr->type = kFcvtWuD;
              } else if (instr->rtype.rs2 == 0x2) {  // FCVT.L.D
                instr->type = kFcvtLD;
              } else if (instr->rtype.rs2 == 0x3) {  // FCVT.LU.D
                instr->type = kFcvtLuD;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x68:
              if (instr->rtype.rs2 == 0x0) {  // FCVT.S.W
                instr->type = kFcvtSW;
              } else if (instr->rtype.rs2 == 0x1) {  // FCVT.S.WU
                instr->type = kFcvtSWu;
              } else if (instr->rtype.rs2 == 0x2) {  // FCVT.S.L
                instr->type = kFcvtSL;
              } else if (instr->rtype.rs2 == 0x3) {  // FCVT.S.LU
                instr->type = kFcvtSLu;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x69:
              if (instr->rtype.rs2 == 0x0) {  // FCVT.D.W
                instr->type = kFcvtDW;
              } else if (instr->rtype.rs2 == 0x1) {  // FCVT.D.WU
                instr->type = kFcvtDWu;
              } else if (instr->rtype.rs2 == 0x2) {  // FCVT.D.L
                instr->type = kFcvtDL;
              } else if (instr->rtype.rs2 == 0x3) {  // FCVT.D.LU
                instr->type = kFcvtDLu;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x70:
              assert(instr->rtype.rs2 == 0x0);
              if (instr->rtype.funct3 == 0x0) {  // FMV.X.W
                instr->type = kFmvXW;
              } else if (instr->rtype.funct3 == 0x1) {  // FCLASS.S
                instr->type = kFclassS;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x71:
              assert(instr->rtype.rs2 == 0x0);
              if (instr->rtype.funct3 == 0x0) {  // FMV.X.D
                instr->type = kFmvXD;
              } else if (instr->rtype.funct3 == 0x1) {  // FCLASS.D
                instr->type = kFclassD;
              } else {
                UNREACHABLE();
              }
              return;
            case 0x78:  // FMV.W.X
              assert(instr->rtype.rs2 == 0x0 && instr->rtype.funct3 == 0x0);
              instr->type = kFmvWX;
              return;
            case 0x79:  // FMV.D.X
              assert(instr->rtype.rs2 == 0x0 && instr->rtype.funct3 == 0x0);
              instr->type = kFmvDX;
              return;
            default:
              UNREACHABLE();
          }  // switch funct7
          UNREACHABLE();
        }  // case 0x14

        case 0x18: {  // B-type
          instr->imm = get_b_type_imm(instr);
          switch (instr->btype.funct3) {
            case 0x0:  // BEQ
              instr->type = kBeq;
              return;
            case 0x1:  // BNE
              instr->type = kBne;
              return;
            case 0x4:  // BLT
              instr->type = kBlt;
              return;
            case 0x5:  // BGE
              instr->type = kBge;
              return;
            case 0x6:  // BLTU
              instr->type = kBltu;
              return;
            case 0x7:  // BGEU
              instr->type = kBgeu;
              return;
            default:
              UNREACHABLE();
          }  // switch funct3
          UNREACHABLE();
        }  // case 0x18

        case 0x19: {  // I-type: JALR
          instr->imm = get_i_type_imm(instr);
          instr->type = kJalr;
          return;
        }  // case 0x19

        case 0x1b: {  // J-type: JAL
          instr->imm = get_j_type_imm(instr);
          instr->type = kJal;
          return;
        }  // case 0x1b

        case 0x1c: {  // I-type
          instr->imm = get_i_type_imm(instr);
          switch (instr->itype.funct3) {
            case 0x0:  // ECALL
              instr->type = kEcall;
              return;
            case 0x1:  // CSRRW
              instr->type = kCsrrw;
              return;
            case 0x2:  // CSRRS
              instr->type = kCsrrs;
              return;
            case 0x3:  // CSRRC
              instr->type = kCsrrc;
              return;
            case 0x5:  // CSRRWI
              instr->type = kCsrrwi;
              return;
            case 0x6:  // CSRRSI
              instr->type = kCsrrsi;
              return;
            case 0x7:  // CSRRCI
              instr->type = kCsrrci;
              return;
            default:
              UNREACHABLE();
          }  // switch funct3
          UNREACHABLE();
        }  // case 0x1c

        default:
          UNREACHABLE();
      }  // switch opcode
      UNREACHABLE();
    }  // quadrant case 0x3

    default:
      UNREACHABLE();
  }
}
