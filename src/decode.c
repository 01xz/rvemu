#include "decode.h"

#include <assert.h>

#include "instr.h"
#include "reg.h"
#include "utils.h"

typedef struct {
  u32 opcode : 7;
  u32 rd     : 5;
  u32 funct3 : 3;
  u32 rs1    : 5;
  u32 rs2    : 5;
  u32 funct7 : 7;
} RvInstrR;

typedef struct {
  u32 opcode : 7;
  u32 rd     : 5;
  u32 funct3 : 3;
  u32 rs1    : 5;
  u32 rs2    : 5;
  u32 funct2 : 2;
  u32 rs3    : 5;
} RvInstrR4;

typedef struct {
  u32 opcode  : 7;
  u32 rd      : 5;
  u32 funct3  : 3;
  u32 rs1     : 5;
  u32 imm11_0 : 12;
} RvInstrI;

typedef struct {
  u32 opcode  : 7;
  u32 imm4_0  : 5;
  u32 funct3  : 3;
  u32 rs1     : 5;
  u32 rs2     : 5;
  u32 imm11_5 : 7;
} RvInstrS;

typedef struct {
  u32 opcode  : 7;
  u32 imm11   : 1;
  u32 imm4_1  : 4;
  u32 funct3  : 3;
  u32 rs1     : 5;
  u32 rs2     : 5;
  u32 imm10_5 : 6;
  u32 imm12   : 1;
} RvInstrB;

typedef struct {
  u32 opcode   : 7;
  u32 rd       : 5;
  u32 imm31_12 : 20;
} RvInstrU;

typedef struct {
  u32 opcode   : 7;
  u32 rd       : 5;
  u32 imm19_12 : 8;
  u32 imm11    : 1;
  u32 imm10_1  : 10;
  u32 imm20    : 1;
} RvInstrJ;

typedef struct {
  u32 op     : 2;
  u32 rs2    : 5;
  u32 rs1    : 5;
  u32 funct4 : 4;
  u32        : 16;
} RvInstrCR;

typedef struct {
  u32 op     : 2;
  u32 imm5   : 5;
  u32 rs1    : 5;
  u32 imm1   : 1;
  u32 funct3 : 3;
  u32        : 16;
} RvInstrCI;

typedef struct {
  u32 op     : 2;
  u32 rs2    : 5;
  u32 imm6   : 6;
  u32 funct3 : 3;
  u32        : 16;
} RvInstrCSS;

typedef struct {
  u32 op     : 2;
  u32 rd     : 3;
  u32 imm8   : 8;
  u32 funct3 : 3;
  u32        : 16;
} RvInstrCIW;

typedef struct {
  u32 op     : 2;
  u32 rd     : 3;
  u32 imm2   : 2;
  u32 rs1    : 3;
  u32 imm3   : 3;
  u32 funct3 : 3;
  u32        : 16;
} RvInstrCL;

typedef struct {
  u32 op     : 2;
  u32 rs2    : 3;
  u32 imm2   : 2;
  u32 rs1    : 3;
  u32 imm3   : 3;
  u32 funct3 : 3;
  u32        : 16;
} RvInstrCS;

typedef struct {
  u32 op     : 2;
  u32 rs2    : 3;
  u32 funct2 : 2;
  u32 rs1    : 3;
  u32 funct6 : 6;
  u32        : 16;
} RvInstrCA;

typedef struct {
  u32 op     : 2;
  u32 imm5   : 5;
  u32 rs1    : 3;
  u32 imm3   : 3;
  u32 funct3 : 3;
  u32        : 16;
} RvInstrCB;

typedef struct {
  u32 op     : 2;
  u32 imm11  : 11;
  u32 funct3 : 3;
  u32        : 16;
} RvInstrCJ;

typedef struct {
  u32 quadrant   : 2;
  u32 opcode     : 5;
  u32            : 6;
  u32 instr15_13 : 3;
  u32            : 4;
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

static inline RvInstr decode_r_type(const RvInstrUn *un) {
  return (RvInstr){
      .rs1 = un->rtype.rs1,
      .rs2 = un->rtype.rs2,
      .rd = un->rtype.rd,
  };
}

static inline RvInstr decode_r4_type(const RvInstrUn *un) {
  return (RvInstr){
      .rs1 = un->r4type.rs1,
      .rs2 = un->r4type.rs2,
      .rs3 = un->r4type.rs3,
      .rd = un->r4type.rd,
  };
}

static inline i32 __get_i_type_imm(const RvInstrUn *un) {
  return (i32)un->raw >> 20;
}

static inline RvInstr decode_i_type(const RvInstrUn *un) {
  return (RvInstr){
      .imm = __get_i_type_imm(un),
      .rs1 = un->itype.rs1,
      .rd = un->itype.rd,
  };
}

static inline i32 __get_s_type_imm(const RvInstrUn *un) {
  i32 imm = (un->stype.imm11_5 << 5) | un->stype.imm4_0;
  return (imm << 20) >> 20;
}

static inline RvInstr decode_s_type(const RvInstrUn *un) {
  return (RvInstr){
      .imm = __get_s_type_imm(un),
      .rs1 = un->stype.rs1,
      .rs2 = un->stype.rs2,
  };
}

static inline i32 __get_b_type_imm(const RvInstrUn *un) {
  i32 imm = (un->btype.imm12 << 12) | (un->btype.imm11 << 11) |
            (un->btype.imm10_5 << 5) | (un->btype.imm4_1 << 1);
  return (imm << 19) >> 19;
}

static inline RvInstr decode_b_type(const RvInstrUn *un) {
  return (RvInstr){
      .imm = __get_b_type_imm(un),
      .rs1 = un->btype.rs1,
      .rs2 = un->btype.rs2,
  };
}

static inline i32 __get_u_type_imm(const RvInstrUn *un) {
  return (i32)un->raw & 0xfffff000;
}

static inline RvInstr decode_u_type(const RvInstrUn *un) {
  return (RvInstr){
      .imm = __get_u_type_imm(un),
      .rd = un->utype.rd,
  };
}

static inline i32 __get_j_type_imm(const RvInstrUn *un) {
  i32 imm = (un->jtype.imm20 << 20) | (un->jtype.imm19_12 << 12) |
            (un->jtype.imm11 << 11) | (un->jtype.imm10_1 << 1);
  return (imm << 11) >> 11;
}

static inline RvInstr decode_j_type(const RvInstrUn *un) {
  return (RvInstr){
      .imm = __get_j_type_imm(un),
      .rd = un->jtype.rd,
  };
}

static inline RvInstr decode_cr_type(const RvInstrUn *un) {
  return (RvInstr){
      .rs1 = un->crtype.rs1,
      .rs2 = un->crtype.rs2,
      .rvc = true,
  };
}

static inline i32 __get_ci_type_imm_scaled_4(const RvInstrUn *un) {
  u32 imm8_7_6 = un->citype.imm5 & 0x3;
  u32 imm8_4_2 = (un->citype.imm5 >> 2) & 0x7;
  u32 imm8_5_5 = un->citype.imm1;
  return (imm8_7_6 << 6) | (imm8_5_5 << 5) | (imm8_4_2 << 2);
}

static inline i32 __get_ci_type_imm_scaled_8(const RvInstrUn *un) {
  u32 imm9_8_6 = un->citype.imm5 & 0x7;
  u32 imm9_4_3 = (un->citype.imm5 >> 3) & 0x3;
  u32 imm9_5_5 = un->citype.imm1;
  return (imm9_8_6 << 6) | (imm9_5_5 << 5) | (imm9_4_3 << 3);
}

static inline i32 __get_ci_type_imm_sign_extended(const RvInstrUn *un) {
  u32 imm6_4_0 = un->citype.imm5;
  u32 imm6_5_5 = un->citype.imm1;
  i32 imm = (imm6_5_5 << 5) | imm6_4_0;
  return (imm << 26) >> 26;
}

static inline i32 __get_ci_type_imm_addi16sp(const RvInstrUn *un) {
  u32 imm10_9_9 = un->citype.imm1;
  u32 imm10_4_4 = (un->citype.imm5 >> 4) & 0x1;
  u32 imm10_6_6 = (un->citype.imm5 >> 3) & 0x1;
  u32 imm10_8_7 = (un->citype.imm5 >> 1) & 0x3;
  u32 imm10_5_5 = un->citype.imm5 & 0x1;

  i32 imm = (imm10_9_9 << 9) | (imm10_4_4 << 4) | (imm10_6_6 << 6) |
            (imm10_8_7 << 7) | (imm10_5_5 << 5);

  return (imm << 22) >> 22;
}

static inline i32 __get_ci_type_imm_lui(const RvInstrUn *un) {
  u32 imm18_17_17 = un->citype.imm1;
  u32 imm18_16_12 = un->citype.imm5;
  i32 imm = (imm18_17_17 << 17) | (imm18_16_12 << 12);
  return (imm << 14) >> 14;
}

static inline RvInstr decode_ci_type(const RvInstrUn *un) {
  return (RvInstr){
      .rd = un->citype.rs1,
      .rvc = true,
  };
}

static inline i32 __get_css_type_imm_scaled_4(const RvInstrUn *un) {
  u32 imm8_7_6 = un->csstype.imm6 & 0x3;
  u32 imm8_5_2 = (un->csstype.imm6 >> 2) & 0xf;
  return (imm8_7_6 << 6) | (imm8_5_2 << 2);
}

static inline i32 __get_css_type_imm_scaled_8(const RvInstrUn *un) {
  u32 imm9_8_6 = un->csstype.imm6 & 0x7;
  u32 imm9_5_3 = (un->csstype.imm6 >> 3) & 0x7;
  return (imm9_8_6 << 6) | (imm9_5_3 << 3);
}

static inline RvInstr decode_css_type(const RvInstrUn *un) {
  return (RvInstr){
      .rs2 = un->csstype.rs2,
      .rvc = true,
  };
}

static inline i32 __get_ciw_type_imm(const RvInstrUn *un) {
  u32 imm10_5_4 = (un->ciwtype.imm8 >> 6) & 0x3;
  u32 imm10_9_6 = (un->ciwtype.imm8 >> 2) & 0xf;
  u32 imm10_2_2 = (un->ciwtype.imm8 >> 1) & 0x1;
  u32 imm10_3_3 = un->ciwtype.imm8 & 0x1;
  return (imm10_9_6 << 6) | (imm10_5_4 << 4) | (imm10_3_3 << 3) |
         (imm10_2_2 << 2);
}

static inline RvInstr decode_ciw_type(const RvInstrUn *un) {
  return (RvInstr){
      .imm = __get_ciw_type_imm(un),
      .rd = un->ciwtype.rd + 8,
      .rvc = true,
  };
}

static inline i32 __get_cl_type_imm_scaled_4(const RvInstrUn *un) {
  u32 imm7_5_3 = un->cltype.imm3;
  u32 imm7_6_6 = un->cltype.imm2 & 0x1;
  u32 imm7_2_2 = (un->cltype.imm2 >> 1) & 0x1;
  return (imm7_6_6 << 6) | (imm7_5_3 << 3) | (imm7_2_2 << 2);
}

static inline i32 __get_cl_type_imm_scaled_8(const RvInstrUn *un) {
  u32 imm8_5_3 = un->cltype.imm3;
  u32 imm8_7_6 = un->cltype.imm2;
  return (imm8_7_6 << 6) | (imm8_5_3 << 3);
}

static inline RvInstr decode_cl_type(const RvInstrUn *un) {
  return (RvInstr){
      .rs1 = un->cltype.rs1 + 8,
      .rd = un->cltype.rd + 8,
      .rvc = true,
  };
}

static inline i32 __get_cs_type_imm_scaled_4(const RvInstrUn *un) {
  u32 imm7_5_3 = un->cstype.imm3;
  u32 imm7_6_6 = un->cstype.imm2 & 0x1;
  u32 imm7_2_2 = (un->cstype.imm2 >> 1) & 0x1;
  return (imm7_6_6 << 6) | (imm7_5_3 << 3) | (imm7_2_2 << 2);
}

static inline i32 __get_cs_type_imm_scaled_8(const RvInstrUn *un) {
  u32 imm8_5_3 = un->cstype.imm3;
  u32 imm8_7_6 = un->cstype.imm2;
  return (imm8_7_6 << 6) | (imm8_5_3 << 3);
}

static inline RvInstr decode_cs_type(const RvInstrUn *un) {
  return (RvInstr){
      .rs1 = un->cstype.rs1 + 8,
      .rs2 = un->cstype.rs2 + 8,
      .rvc = true,
  };
}

static inline RvInstr decode_ca_type(const RvInstrUn *un) {
  return (RvInstr){
      .rs1 = un->catype.rs1 + 8,
      .rs2 = un->catype.rs2 + 8,
      .rd = un->catype.rs1 + 8,
      .rvc = true,
  };
}

static inline i32 __get_cb_type_imm(const RvInstrUn *un) {
  u32 imm9_8_8 = (un->cbtype.imm3 >> 2) & 0x1;
  u32 imm9_4_3 = un->cbtype.imm3 & 0x3;
  u32 imm9_7_6 = (un->cbtype.imm5 >> 3) & 0x3;
  u32 imm9_2_1 = (un->cbtype.imm5 >> 1) & 0x3;
  u32 imm9_5_5 = un->cbtype.imm5 & 0x1;

  i32 imm = (imm9_8_8 << 8) | (imm9_7_6 << 6) | (imm9_5_5 << 5) |
            (imm9_4_3 << 3) | (imm9_2_1 << 1);

  return (imm << 23) >> 23;
}

static inline i32 __get_cb_type_imm_ic(const RvInstrUn *un) {
  u32 imm6_5_5 = (un->cbtype.imm3 >> 2) & 0x1;
  u32 imm6_4_0 = un->cbtype.imm5;
  i32 imm = (imm6_5_5 << 5) | imm6_4_0;
  return (imm << 26) >> 26;
}

static inline RvInstr decode_cb_type(const RvInstrUn *un) {
  return (RvInstr){
      .rs1 = un->cbtype.rs1 + 8,
      .rvc = true,
  };
}

static inline i32 __get_cj_type_imm(const RvInstrUn *un) {
  u32 imm12_11_11 = (un->cjtype.imm11 >> 10) & 0x1;
  u32 imm12_4_4 = (un->cjtype.imm11 >> 9) & 0x1;
  u32 imm12_9_8 = (un->cjtype.imm11 >> 7) & 0x3;
  u32 imm12_10_10 = (un->cjtype.imm11 >> 6) & 0x1;
  u32 imm12_6_6 = (un->cjtype.imm11 >> 5) & 0x1;
  u32 imm12_7_7 = (un->cjtype.imm11 >> 4) & 0x1;
  u32 imm12_3_1 = (un->cjtype.imm11 >> 1) & 0x7;
  u32 imm12_5_5 = un->cjtype.imm11 & 0x1;

  i32 imm12 = (imm12_11_11 << 11) | (imm12_10_10 << 10) | (imm12_9_8 << 8) |
              (imm12_7_7 << 7) | (imm12_6_6 << 6) | (imm12_5_5 << 5) |
              (imm12_4_4 << 4) | (imm12_3_1 << 1);

  return (imm12 << 20) >> 20;
}

static inline RvInstr decode_cj_type(const RvInstrUn *un) {
  return (RvInstr){
      .imm = __get_cj_type_imm(un),
      .rvc = true,
  };
}

void rv_instr_decode(RvInstr *instr, u32 instr_raw) {
  RvInstrUn un = {.raw = instr_raw};
  switch (un.gtype.quadrant) {
    case 0x0: {
      switch (un.gtype.instr15_13) {
        case 0x0:  // CIW-type: C.ADDI4SPN
          *instr = decode_ciw_type(&un);
          instr->type = kAddi;
          instr->rs1 = kSp;
          assert(instr->imm != 0);
          return;
        case 0x1:  // C.FLD
          *instr = decode_cl_type(&un);
          instr->type = kFld;
          instr->imm = __get_cl_type_imm_scaled_8(&un);
          return;
        case 0x2:  // C.LW
          *instr = decode_cl_type(&un);
          instr->type = kLw;
          instr->imm = __get_cl_type_imm_scaled_4(&un);
          return;
        case 0x3:  // C.LD
          *instr = decode_cl_type(&un);
          instr->type = kLd;
          instr->imm = __get_cl_type_imm_scaled_8(&un);
          return;
        case 0x5:  // C.FSD
          *instr = decode_cs_type(&un);
          instr->type = kFsd;
          instr->imm = __get_cs_type_imm_scaled_8(&un);
          return;
        case 0x6:  // C.SW
          *instr = decode_cs_type(&un);
          instr->type = kSw;
          instr->imm = __get_cs_type_imm_scaled_4(&un);
          return;
        case 0x7:  // C.SD
          *instr = decode_cs_type(&un);
          instr->type = kSd;
          instr->imm = __get_cs_type_imm_scaled_8(&un);
          return;
        default:
          __builtin_unreachable();
      }
      __builtin_unreachable();
    }

    case 0x1: {
      switch (un.gtype.instr15_13) {
        case 0x0:  // C.ADDI
          *instr = decode_ci_type(&un);
          instr->type = kAddi;
          instr->rs1 = instr->rd;
          instr->imm = __get_ci_type_imm_sign_extended(&un);
          return;
        case 0x1:  // C.ADDIW
          *instr = decode_ci_type(&un);
          instr->type = kAddiw;
          instr->rs1 = instr->rd;
          instr->imm = __get_ci_type_imm_sign_extended(&un);
          return;
        case 0x2:  // C.LI
          *instr = decode_ci_type(&un);
          instr->type = kAddi;
          instr->rs1 = kZero;
          instr->imm = __get_ci_type_imm_sign_extended(&un);
          return;
        case 0x3: {
          *instr = decode_ci_type(&un);
          if (instr->rd == 2) {  // C.ADDI16SP
            instr->type = kAddi;
            instr->rs1 = instr->rd;
            instr->imm = __get_ci_type_imm_addi16sp(&un);
          } else {  // C.LUI
            instr->type = kLui;
            instr->imm = __get_ci_type_imm_lui(&un);
          }
          return;
        }
        case 0x4: {
          u32 funct2 = (instr_raw >> 10) & 0x3;
          if (funct2 != 0x3) {
            *instr = decode_cb_type(&un);
            instr->imm = __get_cb_type_imm_ic(&un);
            instr->rd = instr->rs1;
            if (funct2 == 0x0) {  // C.SRLI
              instr->type = kSrli;
            } else if (funct2 == 0x1) {  // C.SRAI
              instr->type = kSrai;
            } else {  // C.ANDI
              instr->type = kAndi;
            }
            return;
          } else {
            *instr = decode_ca_type(&un);
            u32 funct1 = (instr_raw >> 12) & 0x1;
            u32 funct2_low = (instr_raw >> 5) & 0x3;
            if (funct1 == 0x0) {
              if (funct2_low == 0x0) {  // C.SUB
                instr->type = kSub;
              } else if (funct2_low == 0x1) {  // C.XOR
                instr->type = kXor;
              } else if (funct2_low == 0x2) {  // C.OR
                instr->type = kOr;
              } else {  // C.AND
                instr->type = kAnd;
              }
              return;
            } else {
              if ((funct2_low & 0x1) == 0x0) {  // C.SUBW
                instr->type = kSubw;
              } else {  // C.ADDW
                instr->type = kAddw;
              }
              return;
            }
          }
        }
        case 0x5:  // C.J
          *instr = decode_cj_type(&un);
          instr->type = kJal;
          instr->rs1 = kZero;
          return;
        case 0x6:  // C.BEQZ
          *instr = decode_cb_type(&un);
          instr->type = kBeq;
          instr->imm = __get_cb_type_imm(&un);
          instr->rs2 = kZero;
          return;
        case 0x7:  // C.BNEZ
          *instr = decode_cb_type(&un);
          instr->type = kBne;
          instr->imm = __get_cb_type_imm(&un);
          instr->rs2 = kZero;
          return;
        default:
          __builtin_unreachable();
      }
      __builtin_unreachable();
    }

    case 0x2: {
      instr->rvc = true;
      switch (un.gtype.instr15_13) {
        case 0x0:  // C.SLLI
          instr->type = kSlli;
          return;
        case 0x1:  // C.FLDSP
          return;
        case 0x2:  // C.LWSP
          return;
        case 0x3:  // C.LDSP
          return;
        case 0x4: {
        }
        case 0x5:  // C.FSDSP
          return;
        case 0x6:  // C.SWSP
          return;
        case 0x7:  // C.SDSP
          return;
        default:
          __builtin_unreachable();
      }
      __builtin_unreachable();
    }

    case 0x3: {  // not rvc
      switch (un.gtype.opcode) {
        case 0x0: {  // I-type
          *instr = decode_i_type(&un);
          switch (un.itype.funct3) {
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
              __builtin_unreachable();
          }  // switch funct3
          __builtin_unreachable();
        }  // opcode case 0x0

        case 0x1: {  // I-type
          *instr = decode_i_type(&un);
          switch (un.itype.funct3) {
            case 0x2:  // FLW
              instr->type = kFlw;
              return;
            case 0x3:  // FLD
              instr->type = kFld;
              return;
            default:
              __builtin_unreachable();
          }  // switch funct3
          __builtin_unreachable();
        }  // opcode case 0x1

        case 0x3: {  // I-type
          *instr = decode_i_type(&un);
          switch (un.itype.funct3) {
            case 0x0:  // FENCE
              instr->type = kFence;
              return;
            case 0x1:  // FENCE.I
              instr->type = kFenceI;
              return;
            default:
              __builtin_unreachable();
          }  // switch funct3
          __builtin_unreachable();
        }  // opcode case 0x3

        case 0x4: {  // I-type
          *instr = decode_i_type(&un);
          switch (un.itype.funct3) {
            case 0x0:  // ADDI
              instr->type = kAddi;
              return;
            case 0x1:
              if ((un.gtype.instr31_25 >> 1) == 0x0) {  // SLLI
                instr->type = kSlli;
              } else {
                __builtin_unreachable();
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
              if ((un.gtype.instr31_25 >> 1) == 0x0) {  // SRLI
                instr->type = kSrli;
              } else if ((un.gtype.instr31_25 >> 1) == 0x10) {  // SRAI
                instr->type = kSrai;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x6:  // ORI
              instr->type = kOri;
              return;
            case 0x7:  // ANDI
              instr->type = kAndi;
              return;
            default:
              __builtin_unreachable();
          }  // switch funct3
          __builtin_unreachable();
        }  // opcode case 0x4

        case 0x5: {  // U-type: AUIPC
          *instr = decode_u_type(&un);
          instr->type = kAuipc;
          return;
        }  // opcode case 0x5

        case 0x6: {  // I-type
          *instr = decode_i_type(&un);
          switch (un.itype.funct3) {
            case 0x0:  // ADDIW
              instr->type = kAddiw;
              return;
            case 0x1:  // SLLIW
              assert(un.gtype.instr31_25 == 0x0);
              instr->type = kSlliw;
              return;
            case 0x5:
              if (un.gtype.instr31_25 == 0x0) {  // SRLIW
                instr->type = kSrliw;
              } else if (un.gtype.instr31_25 == 0x20) {  // SRAIW
                instr->type = kSraiw;
              } else {
                __builtin_unreachable();
              }
              return;
            default:
              __builtin_unreachable();
          }  // switch funct3
          __builtin_unreachable();
        }  // opcode case 0x6

        case 0x8: {  // S-type
          *instr = decode_s_type(&un);
          switch (un.stype.funct3) {
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
              __builtin_unreachable();
          }  // switch funct3
          __builtin_unreachable();
        }  // opcode case 0x8

        case 0x9: {  // S-type
          *instr = decode_s_type(&un);
          switch (un.stype.funct3) {
            case 0x2:  // FSW
              instr->type = kFsw;
              return;
            case 0x3:  // FSD
              instr->type = kFsd;
              return;
            default:
              __builtin_unreachable();
          }  // switch funct3
          __builtin_unreachable();
        }  // case 0x9

        case 0xc: {  // R-type
          *instr = decode_r_type(&un);
          switch (un.rtype.funct3) {
            case 0x0:
              if (un.rtype.funct7 == 0x0) {  // ADD
                instr->type = kAdd;
              } else if (un.rtype.funct7 == 0x1) {  // MUL
                instr->type = kMul;
              } else if (un.rtype.funct7 == 0x20) {  // SUB
                instr->type = kSub;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x1:
              if (un.rtype.funct7 == 0x0) {  // SLL
                instr->type = kSll;
              } else if (un.rtype.funct7 == 0x1) {  // MULH
                instr->type = kMulh;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x2:
              if (un.rtype.funct7 == 0x0) {  // SLT
                instr->type = kSlt;
              } else if (un.rtype.funct7 == 0x1) {  // MULHSU
                instr->type = kMulhsu;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x3:
              if (un.rtype.funct7 == 0x0) {  // SLTU
                instr->type = kSltu;
              } else if (un.rtype.funct7 == 0x1) {  // MULHU
                instr->type = kMulhu;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x4:
              if (un.rtype.funct7 == 0x0) {  // XOR
                instr->type = kXor;
              } else if (un.rtype.funct7 == 0x1) {  // DIV
                instr->type = kDiv;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x5:
              if (un.rtype.funct7 == 0x0) {  // SRL
                instr->type = kSrl;
              } else if (un.rtype.funct7 == 0x1) {  // DIVU
                instr->type = kDivu;
              } else if (un.rtype.funct7 == 0x20) {  // SRA
                instr->type = kSra;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x6:
              if (un.rtype.funct7 == 0x0) {  // OR
                instr->type = kOr;
              } else if (un.rtype.funct7 == 0x1) {  // REM
                instr->type = kRem;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x7:
              if (un.rtype.funct7 == 0x0) {  // AND
                instr->type = kAnd;
              } else if (un.rtype.funct7 == 0x1) {  // REMU
                instr->type = kRemu;
              } else {
                __builtin_unreachable();
              }
              return;
            default:
              __builtin_unreachable();
          }  // switch funct3
          __builtin_unreachable();
        }  // case 0xc

        case 0xd: {  // U-type: LUI
          *instr = decode_u_type(&un);
          instr->type = kLui;
          return;
        }  // case 0xd

        case 0xe: {  // R-type
          *instr = decode_r_type(&un);
          switch (un.rtype.funct3) {
            case 0x0:
              if (un.rtype.funct7 == 0x0) {  // ADDW
                instr->type = kAddw;
              } else if (un.rtype.funct7 == 0x1) {  // MULW
                instr->type = kMulw;
              } else if (un.rtype.funct7 == 0x20) {  // SUBW
                instr->type = kSubw;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x1:
              if (un.rtype.funct7 == 0x0) {  // SLLW
                instr->type = kSllw;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x4:
              if (un.rtype.funct7 == 0x1) {  // DIVW
                instr->type = kDivw;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x5:
              if (un.rtype.funct7 == 0x0) {  // SRLW
                instr->type = kSrlw;
              } else if (un.rtype.funct7 == 0x1) {  // DIVUW
                instr->type = kDivuw;
              } else if (un.rtype.funct7 == 0x20) {  // SRAW
                instr->type = kSraw;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x6:
              if (un.rtype.funct7 == 0x1) {  // REMW
                instr->type = kRemw;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x7:
              if (un.rtype.funct7 == 0x1) {  // REMUW
                instr->type = kRemuw;
              } else {
                __builtin_unreachable();
              }
              return;
            default:
              __builtin_unreachable();
          }  // switch funct3
          __builtin_unreachable();
        }  // case 0xe

        case 0x10: {  // R4-type
          *instr = decode_r4_type(&un);
          switch (un.r4type.funct2) {
            case 0x0:  // FMADD.S
              instr->type = kFmaddS;
              return;
            case 0x1:  // FMADD.D
              instr->type = kFmaddD;
              return;
            default:
              __builtin_unreachable();
          }  // switch funct2
          __builtin_unreachable();
        }  // case 0x10

        case 0x11: {  // R4-type
          *instr = decode_r4_type(&un);
          switch (un.r4type.funct2) {
            case 0x0:  // FMSUB.S
              instr->type = kFmsubS;
              return;
            case 0x1:  // FMSUB.D
              instr->type = kFmsubD;
              return;
            default:
              __builtin_unreachable();
          }  // switch funct2
          __builtin_unreachable();
        }  // case 0x11

        case 0x12: {  // R4-type
          *instr = decode_r4_type(&un);
          switch (un.r4type.funct2) {
            case 0x0:  // FNMSUB.S
              instr->type = kFnmsubS;
              return;
            case 0x1:  // FNMSUB.D
              instr->type = kFnmsubD;
              return;
            default:
              __builtin_unreachable();
          }  // switch funct2
          __builtin_unreachable();
        }  // case 0x12

        case 0x13: {  // R4-type
          *instr = decode_r4_type(&un);
          switch (un.r4type.funct2) {
            case 0x0:  // FNMADD.S
              instr->type = kFnmaddS;
              return;
            case 0x1:  // FNMADD.D
              instr->type = kFnmaddD;
              return;
            default:
              __builtin_unreachable();
          }  // switch funct2
          __builtin_unreachable();
        }  // case 0x13

        case 0x14: {  // R-type
          *instr = decode_r_type(&un);
          switch (un.rtype.funct7) {
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
              if (un.rtype.funct3 == 0x0) {  // FSGNJ.S
                instr->type = kFsgnjS;
              } else if (un.rtype.funct3 == 0x1) {  // FSNGJN.S
                instr->type = kFsgnjnS;
              } else if (un.rtype.funct3 == 0x2) {  // FSNGJX.S
                instr->type = kFsgnjxS;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x11:
              if (un.rtype.funct3 == 0x0) {  // FSGNJ.D
                instr->type = kFsgnjD;
              } else if (un.rtype.funct3 == 0x1) {  // FSNGJN.D
                instr->type = kFsgnjnD;
              } else if (un.rtype.funct3 == 0x2) {  // FSNGJX.D
                instr->type = kFsgnjxD;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x14:
              if (un.rtype.funct3 == 0x0) {  // FMIN.S
                instr->type = kFminS;
              } else if (un.rtype.funct3 == 0x1) {  // FMAX.S
                instr->type = kFmaxS;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x15:
              if (un.rtype.funct3 == 0x0) {  // FMIN.D
                instr->type = kFminD;
              } else if (un.rtype.funct3 == 0x1) {  // FMAX.D
                instr->type = kFmaxD;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x20:  // FCVT.S.D
              assert(un.rtype.rs2 == 0x1);
              instr->type = kFcvtSD;
              return;
            case 0x21:  // FCVT.D.S
              assert(un.rtype.rs2 == 0x0);
              instr->type = kFcvtDS;
              return;
            case 0x2c:  // FSQRT.S
              assert(un.rtype.rs2 == 0x0);
              instr->type = kFsqrtS;
              return;
            case 0x2d:  // FSQRT.D
              assert(un.rtype.rs2 == 0x0);
              instr->type = kFsqrtD;
              return;
            case 0x50:
              if (un.rtype.funct3 == 0x0) {  // FLE.S
                instr->type = kFleS;
              } else if (un.rtype.funct3 == 0x1) {  // FLT.S
                instr->type = kFltS;
              } else if (un.rtype.funct3 == 0x2) {  // FEQ.S
                instr->type = kFeqS;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x51:
              if (un.rtype.funct3 == 0x0) {  // FLE.D
                instr->type = kFleD;
              } else if (un.rtype.funct3 == 0x1) {  // FLT.D
                instr->type = kFltD;
              } else if (un.rtype.funct3 == 0x2) {  // FEQ.D
                instr->type = kFeqD;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x60:
              if (un.rtype.rs2 == 0x0) {  // FCVT.W.S
                instr->type = kFcvtWS;
              } else if (un.rtype.rs2 == 0x1) {  // FCVT.WU.S
                instr->type = kFcvtWuS;
              } else if (un.rtype.rs2 == 0x2) {  // FCVT.L.S
                instr->type = kFcvtLS;
              } else if (un.rtype.rs2 == 0x3) {  // FCVT.LU.S
                instr->type = kFcvtLuS;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x61:
              if (un.rtype.rs2 == 0x0) {  // FCVT.W.D
                instr->type = kFcvtWD;
              } else if (un.rtype.rs2 == 0x1) {  // FCVT.WU.D
                instr->type = kFcvtWuD;
              } else if (un.rtype.rs2 == 0x2) {  // FCVT.L.D
                instr->type = kFcvtLD;
              } else if (un.rtype.rs2 == 0x3) {  // FCVT.LU.D
                instr->type = kFcvtLuD;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x68:
              if (un.rtype.rs2 == 0x0) {  // FCVT.S.W
                instr->type = kFcvtSW;
              } else if (un.rtype.rs2 == 0x1) {  // FCVT.S.WU
                instr->type = kFcvtSWu;
              } else if (un.rtype.rs2 == 0x2) {  // FCVT.S.L
                instr->type = kFcvtSL;
              } else if (un.rtype.rs2 == 0x3) {  // FCVT.S.LU
                instr->type = kFcvtSLu;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x69:
              if (un.rtype.rs2 == 0x0) {  // FCVT.D.W
                instr->type = kFcvtDW;
              } else if (un.rtype.rs2 == 0x1) {  // FCVT.D.WU
                instr->type = kFcvtDWu;
              } else if (un.rtype.rs2 == 0x2) {  // FCVT.D.L
                instr->type = kFcvtDL;
              } else if (un.rtype.rs2 == 0x3) {  // FCVT.D.LU
                instr->type = kFcvtDLu;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x70:
              assert(un.rtype.rs2 == 0x0);
              if (un.rtype.funct3 == 0x0) {  // FMV.X.W
                instr->type = kFmvXW;
              } else if (un.rtype.funct3 == 0x1) {  // FCLASS.S
                instr->type = kFclassS;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x71:
              assert(un.rtype.rs2 == 0x0);
              if (un.rtype.funct3 == 0x0) {  // FMV.X.D
                instr->type = kFmvXD;
              } else if (un.rtype.funct3 == 0x1) {  // FCLASS.D
                instr->type = kFclassD;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x78:  // FMV.W.X
              assert(un.rtype.rs2 == 0x0 && un.rtype.funct3 == 0x0);
              instr->type = kFmvWX;
              return;
            case 0x79:  // FMV.D.X
              assert(un.rtype.rs2 == 0x0 && un.rtype.funct3 == 0x0);
              instr->type = kFmvDX;
              return;
            default:
              __builtin_unreachable();
          }  // switch funct7
          __builtin_unreachable();
        }  // case 0x14

        case 0x18: {  // B-type
          *instr = decode_b_type(&un);
          switch (un.btype.funct3) {
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
              __builtin_unreachable();
          }  // switch funct3
          __builtin_unreachable();
        }  // case 0x18

        case 0x19: {  // I-type: JALR
          *instr = decode_i_type(&un);
          instr->type = kJalr;
          return;
        }  // case 0x19

        case 0x1b: {  // J-type: JAL
          *instr = decode_j_type(&un);
          instr->type = kJal;
          return;
        }  // case 0x1b

        case 0x1c: {  // I-type
          *instr = decode_i_type(&un);
          switch (un.itype.funct3) {
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
              __builtin_unreachable();
          }  // switch funct3
          __builtin_unreachable();
        }  // case 0x1c

        default:
          __builtin_unreachable();
      }  // switch opcode
      __builtin_unreachable();
    }  // quadrant case 0x3

    default:
      __builtin_unreachable();
  }
}
