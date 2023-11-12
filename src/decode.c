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

static inline u16 __get_i_type_csr(const RvInstrUn *un) {
  return un->raw >> 20;
}

static inline RvInstr decode_i_type_with_csr(const RvInstrUn *un) {
  return (RvInstr){
      .csr = __get_i_type_csr(un),
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

static inline i32 __get_ci_type_imm_slli(const RvInstrUn *un) {
  u32 imm6_5_5 = un->citype.imm1;
  u32 imm6_4_0 = un->citype.imm5;
  i32 imm = (imm6_5_5 << 5) | imm6_4_0;
  return (imm << 26) >> 26;
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
        case 0x0:  // CIW-format: C.ADDI4SPN
          *instr = decode_ciw_type(&un);
          instr->type = U_RV32I_ADDI;
          instr->rs1 = XREG_SP;
          assert(instr->imm != 0);
          return;
        case 0x1:  // CL-format: C.FLD
          *instr = decode_cl_type(&un);
          instr->type = U_RV32D_FLD;
          instr->imm = __get_cl_type_imm_scaled_8(&un);
          return;
        case 0x2:  // CL-format: C.LW
          *instr = decode_cl_type(&un);
          instr->type = U_RV32I_LW;
          instr->imm = __get_cl_type_imm_scaled_4(&un);
          return;
        case 0x3:  // CL-format: C.LD
          *instr = decode_cl_type(&un);
          instr->type = U_RV64I_LD;
          instr->imm = __get_cl_type_imm_scaled_8(&un);
          return;
        case 0x5:  // CS-format: C.FSD
          *instr = decode_cs_type(&un);
          instr->type = U_RV32D_FSD;
          instr->imm = __get_cs_type_imm_scaled_8(&un);
          return;
        case 0x6:  // CS-format: C.SW
          *instr = decode_cs_type(&un);
          instr->type = U_RV32I_SW;
          instr->imm = __get_cs_type_imm_scaled_4(&un);
          return;
        case 0x7:  // CS-format: C.SD
          *instr = decode_cs_type(&un);
          instr->type = U_RV64I_SD;
          instr->imm = __get_cs_type_imm_scaled_8(&un);
          return;
        default:
          __builtin_unreachable();
      }
      __builtin_unreachable();
    }  // quadrant case 0x0

    case 0x1: {
      switch (un.gtype.instr15_13) {
        case 0x0:  // CI-format: C.ADDI
          *instr = decode_ci_type(&un);
          instr->type = U_RV32I_ADDI;
          instr->rs1 = instr->rd;
          instr->imm = __get_ci_type_imm_sign_extended(&un);
          return;
        case 0x1:  // CI-format: C.ADDIW
          *instr = decode_ci_type(&un);
          instr->type = U_RV64I_ADDIW;
          instr->rs1 = instr->rd;
          instr->imm = __get_ci_type_imm_sign_extended(&un);
          return;
        case 0x2:  // CI-format: C.LI
          *instr = decode_ci_type(&un);
          instr->type = U_RV32I_ADDI;
          instr->rs1 = XREG_ZERO;
          instr->imm = __get_ci_type_imm_sign_extended(&un);
          return;
        case 0x3: {  // CI-format
          *instr = decode_ci_type(&un);
          if (instr->rd == 2) {  // C.ADDI16SP
            instr->type = U_RV32I_ADDI;
            instr->rs1 = instr->rd;
            instr->imm = __get_ci_type_imm_addi16sp(&un);
          } else {  // C.LUI
            instr->type = U_RV32I_LUI;
            instr->imm = __get_ci_type_imm_lui(&un);
          }
          return;
        }
        case 0x4: {
          u32 funct2 = (instr_raw >> 10) & 0x3;
          if (funct2 != 0x3) {  // CB-format
            *instr = decode_cb_type(&un);
            instr->imm = __get_cb_type_imm_ic(&un);
            instr->rd = instr->rs1;
            if (funct2 == 0x0) {  // C.SRLI
              instr->type = U_RV64I_SRLI;
            } else if (funct2 == 0x1) {  // C.SRAI
              instr->type = U_RV64I_SRAI;
            } else {  // C.ANDI
              instr->type = U_RV32I_ANDI;
            }
            return;
          } else {  // CA-format
            *instr = decode_ca_type(&un);
            u32 funct1 = (instr_raw >> 12) & 0x1;
            u32 funct2_low = (instr_raw >> 5) & 0x3;
            if (funct1 == 0x0) {
              if (funct2_low == 0x0) {  // C.SUB
                instr->type = U_RV32I_SUB;
              } else if (funct2_low == 0x1) {  // C.XOR
                instr->type = U_RV32I_XOR;
              } else if (funct2_low == 0x2) {  // C.OR
                instr->type = U_RV32I_OR;
              } else {  // C.AND
                instr->type = U_RV32I_AND;
              }
              return;
            } else {
              if ((funct2_low & 0x1) == 0x0) {  // C.SUBW
                instr->type = U_RV64I_SUBW;
              } else {  // C.ADDW
                instr->type = U_RV64I_ADDW;
              }
              return;
            }
          }
        }
        case 0x5:  // CJ-format: C.J
          *instr = decode_cj_type(&un);
          instr->type = U_RV32I_JAL;
          instr->rs1 = XREG_ZERO;
          instr->cont = true;
          return;
        case 0x6:  // CB-format: C.BEQZ
          *instr = decode_cb_type(&un);
          instr->type = U_RV32I_BEQ;
          instr->imm = __get_cb_type_imm(&un);
          instr->rs2 = XREG_ZERO;
          return;
        case 0x7:  // CB-format: C.BNEZ
          *instr = decode_cb_type(&un);
          instr->type = U_RV32I_BNE;
          instr->imm = __get_cb_type_imm(&un);
          instr->rs2 = XREG_ZERO;
          return;
        default:
          __builtin_unreachable();
      }
      __builtin_unreachable();
    }  // quadrant case 0x1

    case 0x2: {
      switch (un.gtype.instr15_13) {
        case 0x0:  // CI-format: C.SLLI
          *instr = decode_ci_type(&un);
          instr->type = U_RV64I_SLLI;
          instr->imm = __get_ci_type_imm_slli(&un);
          instr->rs1 = instr->rd;
          return;
        case 0x1:  // CI-format: C.FLDSP
          *instr = decode_ci_type(&un);
          instr->type = U_RV32D_FLD;
          instr->imm = __get_ci_type_imm_scaled_8(&un);
          instr->rs1 = XREG_SP;
          return;
        case 0x2:  // CI-format: C.LWSP
          *instr = decode_ci_type(&un);
          instr->type = U_RV32I_LW;
          instr->imm = __get_ci_type_imm_scaled_4(&un);
          instr->rs1 = XREG_SP;
          return;
        case 0x3:  // CI-format: C.LDSP
          *instr = decode_ci_type(&un);
          instr->type = U_RV64I_LD;
          instr->imm = __get_ci_type_imm_scaled_8(&un);
          instr->rs1 = XREG_SP;
          return;
        case 0x4: {  // CR-format
          *instr = decode_cr_type(&un);
          if (un.crtype.funct4 == 0x8) {
            if (instr->rs2 == 0) {  // C.JR
              instr->type = U_RV32I_JALR;
              instr->rd = XREG_ZERO;
              instr->cont = true;
            } else {  // C.MV
              instr->type = U_RV32I_ADD;
              instr->rd = instr->rs1;
              instr->rs1 = XREG_ZERO;
            }
            return;
          } else {
            if (instr->rs1 == 0) {  // C.EBREAK
              instr->type = U_RV32I_EBREAK;
            } else if (instr->rs2 == 0) {  // C.JALR
              instr->type = U_RV32I_JALR;
              instr->rd = XREG_RA;
              instr->cont = true;
            } else {  // C.ADD
              instr->type = U_RV32I_ADD;
              instr->rd = instr->rs1;
            }
            return;
          }
        }
        case 0x5:  // CSS-format: C.FSDSP
          *instr = decode_css_type(&un);
          instr->type = U_RV32D_FSD;
          instr->imm = __get_css_type_imm_scaled_8(&un);
          instr->rs1 = XREG_SP;
          return;
        case 0x6:  // CSS-format: C.SWSP
          *instr = decode_css_type(&un);
          instr->type = U_RV32I_SW;
          instr->imm = __get_css_type_imm_scaled_4(&un);
          instr->rs1 = XREG_SP;
          return;
        case 0x7:  // CSS-format: C.SDSP
          *instr = decode_css_type(&un);
          instr->type = U_RV64I_SD;
          instr->imm = __get_css_type_imm_scaled_8(&un);
          instr->rs1 = XREG_SP;
          return;
        default:
          __builtin_unreachable();
      }
      __builtin_unreachable();
    }  // quadrant case 0x2

    case 0x3: {  // not rvc
      switch (un.gtype.opcode) {
        case 0x0: {  // I-type
          *instr = decode_i_type(&un);
          switch (un.itype.funct3) {
            case 0x0:  // LB
              instr->type = U_RV32I_LB;
              return;
            case 0x1:  // LH
              instr->type = U_RV32I_LH;
              return;
            case 0x2:  // LW
              instr->type = U_RV32I_LW;
              return;
            case 0x3:  // LD
              instr->type = U_RV64I_LD;
              return;
            case 0x4:  // LBU
              instr->type = U_RV32I_LBU;
              return;
            case 0x5:  // LHU
              instr->type = U_RV32I_LHU;
              return;
            case 0x6:  // LWU
              instr->type = U_RV64I_LWU;
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
              instr->type = U_RV32F_FLW;
              return;
            case 0x3:  // FLD
              instr->type = U_RV32D_FLD;
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
              instr->type = U_RV32I_FENCE;
              return;
            case 0x1:  // FENCE.I
              instr->type = U_ZIFENCEI_FENCE_I;
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
              instr->type = U_RV32I_ADDI;
              return;
            case 0x1:
              if ((un.gtype.instr31_25 >> 1) == 0x0) {  // SLLI
                instr->type = U_RV64I_SLLI;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x2:  // SLTI
              instr->type = U_RV32I_SLTI;
              return;
            case 0x3:  // SLTIU
              instr->type = U_RV32I_SLTIU;
              return;
            case 0x4:  // XORI
              instr->type = U_RV32I_XORI;
              return;
            case 0x5:
              if ((un.gtype.instr31_25 >> 1) == 0x0) {  // SRLI
                instr->type = U_RV64I_SRLI;
              } else if ((un.gtype.instr31_25 >> 1) == 0x10) {  // SRAI
                instr->type = U_RV64I_SRAI;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x6:  // ORI
              instr->type = U_RV32I_ORI;
              return;
            case 0x7:  // ANDI
              instr->type = U_RV32I_ANDI;
              return;
            default:
              __builtin_unreachable();
          }  // switch funct3
          __builtin_unreachable();
        }  // opcode case 0x4

        case 0x5: {  // U-type: AUIPC
          *instr = decode_u_type(&un);
          instr->type = U_RV32I_AUIPC;
          return;
        }  // opcode case 0x5

        case 0x6: {  // I-type
          *instr = decode_i_type(&un);
          switch (un.itype.funct3) {
            case 0x0:  // ADDIW
              instr->type = U_RV64I_ADDIW;
              return;
            case 0x1:  // SLLIW
              assert(un.gtype.instr31_25 == 0x0);
              instr->type = U_RV64I_SLLIW;
              return;
            case 0x5:
              if (un.gtype.instr31_25 == 0x0) {  // SRLIW
                instr->type = U_RV64I_SRLIW;
              } else if (un.gtype.instr31_25 == 0x20) {  // SRAIW
                instr->type = U_RV64I_SRAIW;
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
              instr->type = U_RV32I_SB;
              return;
            case 0x1:  // SH
              instr->type = U_RV32I_SH;
              return;
            case 0x2:  // SW
              instr->type = U_RV32I_SW;
              return;
            case 0x3:  // SD
              instr->type = U_RV64I_SD;
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
              instr->type = U_RV32F_FSW;
              return;
            case 0x3:  // FSD
              instr->type = U_RV32D_FSD;
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
                instr->type = U_RV32I_ADD;
              } else if (un.rtype.funct7 == 0x1) {  // MUL
                instr->type = U_RV32M_MUL;
              } else if (un.rtype.funct7 == 0x20) {  // SUB
                instr->type = U_RV32I_SUB;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x1:
              if (un.rtype.funct7 == 0x0) {  // SLL
                instr->type = U_RV32I_SLL;
              } else if (un.rtype.funct7 == 0x1) {  // MULH
                instr->type = U_RV32M_MULH;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x2:
              if (un.rtype.funct7 == 0x0) {  // SLT
                instr->type = U_RV32I_SLT;
              } else if (un.rtype.funct7 == 0x1) {  // MULHSU
                instr->type = U_RV32M_MULHSU;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x3:
              if (un.rtype.funct7 == 0x0) {  // SLTU
                instr->type = U_RV32I_SLTU;
              } else if (un.rtype.funct7 == 0x1) {  // MULHU
                instr->type = U_RV32M_MULHU;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x4:
              if (un.rtype.funct7 == 0x0) {  // XOR
                instr->type = U_RV32I_XOR;
              } else if (un.rtype.funct7 == 0x1) {  // DIV
                instr->type = U_RV32M_DIV;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x5:
              if (un.rtype.funct7 == 0x0) {  // SRL
                instr->type = U_RV32I_SRL;
              } else if (un.rtype.funct7 == 0x1) {  // DIVU
                instr->type = U_RV32M_DIVU;
              } else if (un.rtype.funct7 == 0x20) {  // SRA
                instr->type = U_RV32I_SRA;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x6:
              if (un.rtype.funct7 == 0x0) {  // OR
                instr->type = U_RV32I_OR;
              } else if (un.rtype.funct7 == 0x1) {  // REM
                instr->type = U_RV32M_REM;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x7:
              if (un.rtype.funct7 == 0x0) {  // AND
                instr->type = U_RV32I_AND;
              } else if (un.rtype.funct7 == 0x1) {  // REMU
                instr->type = U_RV32M_REMU;
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
          instr->type = U_RV32I_LUI;
          return;
        }  // case 0xd

        case 0xe: {  // R-type
          *instr = decode_r_type(&un);
          switch (un.rtype.funct3) {
            case 0x0:
              if (un.rtype.funct7 == 0x0) {  // ADDW
                instr->type = U_RV64I_ADDW;
              } else if (un.rtype.funct7 == 0x1) {  // MULW
                instr->type = U_RV64M_MULW;
              } else if (un.rtype.funct7 == 0x20) {  // SUBW
                instr->type = U_RV64I_SUBW;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x1:
              if (un.rtype.funct7 == 0x0) {  // SLLW
                instr->type = U_RV64I_SLLW;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x4:
              if (un.rtype.funct7 == 0x1) {  // DIVW
                instr->type = U_RV64M_DIVW;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x5:
              if (un.rtype.funct7 == 0x0) {  // SRLW
                instr->type = U_RV64I_SRLW;
              } else if (un.rtype.funct7 == 0x1) {  // DIVUW
                instr->type = U_RV64M_DIVUW;
              } else if (un.rtype.funct7 == 0x20) {  // SRAW
                instr->type = U_RV64I_SRAW;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x6:
              if (un.rtype.funct7 == 0x1) {  // REMW
                instr->type = U_RV64M_REMW;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x7:
              if (un.rtype.funct7 == 0x1) {  // REMUW
                instr->type = U_RV64M_REMUW;
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
              instr->type = U_RV32F_FMADD_S;
              return;
            case 0x1:  // FMADD.D
              instr->type = U_RV32D_FMADD_D;
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
              instr->type = U_RV32F_FMSUB_S;
              return;
            case 0x1:  // FMSUB.D
              instr->type = U_RV32D_FMSUB_D;
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
              instr->type = U_RV32F_FNMSUB_S;
              return;
            case 0x1:  // FNMSUB.D
              instr->type = U_RV32D_FNMSUB_D;
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
              instr->type = U_RV32F_FNMADD_S;
              return;
            case 0x1:  // FNMADD.D
              instr->type = U_RV32D_FNMADD_D;
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
              instr->type = U_RV32F_FADD_S;
              return;
            case 0x1:  // FADD.D
              instr->type = U_RV32D_FADD_D;
              return;
            case 0x4:  // FSUB.S
              instr->type = U_RV32F_FSUB_S;
              return;
            case 0x5:  // FSUB.D
              instr->type = U_RV32D_FSUB_D;
              return;
            case 0x8:  // FMUL.S
              instr->type = U_RV32F_FMUL_S;
              return;
            case 0x9:  // FMUL.D
              instr->type = U_RV32D_FMUL_D;
              return;
            case 0xc:  // FDIV.S
              instr->type = U_RV32F_FDIV_S;
              return;
            case 0xd:  // FDIV.D
              instr->type = U_RV32D_FDIV_D;
              return;
            case 0x10:
              if (un.rtype.funct3 == 0x0) {  // FSGNJ.S
                instr->type = U_RV32F_FSGNJ_S;
              } else if (un.rtype.funct3 == 0x1) {  // FSNGJN.S
                instr->type = U_RV32F_FSGNJN_S;
              } else if (un.rtype.funct3 == 0x2) {  // FSNGJX.S
                instr->type = U_RV32F_FSGNJX_S;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x11:
              if (un.rtype.funct3 == 0x0) {  // FSGNJ.D
                instr->type = U_RV32D_FSGNJ_D;
              } else if (un.rtype.funct3 == 0x1) {  // FSNGJN.D
                instr->type = U_RV32D_FSGNJN_D;
              } else if (un.rtype.funct3 == 0x2) {  // FSNGJX.D
                instr->type = U_RV32D_FSGNJX_D;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x14:
              if (un.rtype.funct3 == 0x0) {  // FMIN.S
                instr->type = U_RV32F_FMIN_S;
              } else if (un.rtype.funct3 == 0x1) {  // FMAX.S
                instr->type = U_RV32F_FMAX_S;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x15:
              if (un.rtype.funct3 == 0x0) {  // FMIN.D
                instr->type = U_RV32D_FMIN_D;
              } else if (un.rtype.funct3 == 0x1) {  // FMAX.D
                instr->type = U_RV32D_FMAX_D;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x20:  // FCVT.S.D
              assert(un.rtype.rs2 == 0x1);
              instr->type = U_RV32D_FCVT_S_D;
              return;
            case 0x21:  // FCVT.D.S
              assert(un.rtype.rs2 == 0x0);
              instr->type = U_RV32D_FCVT_D_S;
              return;
            case 0x2c:  // FSQRT.S
              assert(un.rtype.rs2 == 0x0);
              instr->type = U_RV32F_FSQRT_S;
              return;
            case 0x2d:  // FSQRT.D
              assert(un.rtype.rs2 == 0x0);
              instr->type = U_RV32D_FSQRT_D;
              return;
            case 0x50:
              if (un.rtype.funct3 == 0x0) {  // FLE.S
                instr->type = U_RV32F_FLE_S;
              } else if (un.rtype.funct3 == 0x1) {  // FLT.S
                instr->type = U_RV32F_FLT_S;
              } else if (un.rtype.funct3 == 0x2) {  // FEQ.S
                instr->type = U_RV32F_FEQ_S;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x51:
              if (un.rtype.funct3 == 0x0) {  // FLE.D
                instr->type = U_RV32D_FLE_D;
              } else if (un.rtype.funct3 == 0x1) {  // FLT.D
                instr->type = U_RV32D_FLT_D;
              } else if (un.rtype.funct3 == 0x2) {  // FEQ.D
                instr->type = U_RV32D_FEQ_D;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x60:
              if (un.rtype.rs2 == 0x0) {  // FCVT.W.S
                instr->type = U_RV32F_FCVT_W_S;
              } else if (un.rtype.rs2 == 0x1) {  // FCVT.WU.S
                instr->type = U_RV32F_FCVT_WU_S;
              } else if (un.rtype.rs2 == 0x2) {  // FCVT.L.S
                instr->type = U_RV64F_FCVT_L_S;
              } else if (un.rtype.rs2 == 0x3) {  // FCVT.LU.S
                instr->type = U_RV64F_FCVT_LU_S;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x61:
              if (un.rtype.rs2 == 0x0) {  // FCVT.W.D
                instr->type = U_RV32D_FCVT_W_D;
              } else if (un.rtype.rs2 == 0x1) {  // FCVT.WU.D
                instr->type = U_RV32D_FCVT_WU_D;
              } else if (un.rtype.rs2 == 0x2) {  // FCVT.L.D
                instr->type = U_RV64D_FCVT_L_D;
              } else if (un.rtype.rs2 == 0x3) {  // FCVT.LU.D
                instr->type = U_RV64D_FCVT_LU_D;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x68:
              if (un.rtype.rs2 == 0x0) {  // FCVT.S.W
                instr->type = U_RV32F_FCVT_S_W;
              } else if (un.rtype.rs2 == 0x1) {  // FCVT.S.WU
                instr->type = U_RV32F_FCVT_S_WU;
              } else if (un.rtype.rs2 == 0x2) {  // FCVT.S.L
                instr->type = U_RV64F_FCVT_S_L;
              } else if (un.rtype.rs2 == 0x3) {  // FCVT.S.LU
                instr->type = U_RV64F_FCVT_S_LU;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x69:
              if (un.rtype.rs2 == 0x0) {  // FCVT.D.W
                instr->type = U_RV32D_FCVT_D_W;
              } else if (un.rtype.rs2 == 0x1) {  // FCVT.D.WU
                instr->type = U_RV32D_FCVT_D_WU;
              } else if (un.rtype.rs2 == 0x2) {  // FCVT.D.L
                instr->type = U_RV64D_FCVT_D_L;
              } else if (un.rtype.rs2 == 0x3) {  // FCVT.D.LU
                instr->type = U_RV64D_FCVT_D_LU;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x70:
              assert(un.rtype.rs2 == 0x0);
              if (un.rtype.funct3 == 0x0) {  // FMV.X.W
                instr->type = U_RV32F_FMV_X_W;
              } else if (un.rtype.funct3 == 0x1) {  // FCLASS.S
                instr->type = U_RV32F_FCLASS_S;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x71:
              assert(un.rtype.rs2 == 0x0);
              if (un.rtype.funct3 == 0x0) {  // FMV.X.D
                instr->type = U_RV64D_FMV_X_D;
              } else if (un.rtype.funct3 == 0x1) {  // FCLASS.D
                instr->type = U_RV32D_FCLASS_D;
              } else {
                __builtin_unreachable();
              }
              return;
            case 0x78:  // FMV.W.X
              assert(un.rtype.rs2 == 0x0 && un.rtype.funct3 == 0x0);
              instr->type = U_RV32F_FMV_W_X;
              return;
            case 0x79:  // FMV.D.X
              assert(un.rtype.rs2 == 0x0 && un.rtype.funct3 == 0x0);
              instr->type = U_RV64D_FMV_D_X;
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
              instr->type = U_RV32I_BEQ;
              return;
            case 0x1:  // BNE
              instr->type = U_RV32I_BNE;
              return;
            case 0x4:  // BLT
              instr->type = U_RV32I_BLT;
              return;
            case 0x5:  // BGE
              instr->type = U_RV32I_BGE;
              return;
            case 0x6:  // BLTU
              instr->type = U_RV32I_BLTU;
              return;
            case 0x7:  // BGEU
              instr->type = U_RV32I_BGEU;
              return;
            default:
              __builtin_unreachable();
          }  // switch funct3
          __builtin_unreachable();
        }  // case 0x18

        case 0x19: {  // I-type: JALR
          *instr = decode_i_type(&un);
          instr->type = U_RV32I_JALR;
          instr->cont = true;
          return;
        }  // case 0x19

        case 0x1b: {  // J-type: JAL
          *instr = decode_j_type(&un);
          instr->type = U_RV32I_JAL;
          instr->cont = true;
          return;
        }  // case 0x1b

        case 0x1c: {  // I-type
          if (un.itype.funct3 == 0x0) {
            switch (un.itype.imm11_0) {
              case 0x000:  // ECALL
                instr->type = U_RV32I_ECALL;
                instr->cont = true;
                return;
              case 0x001:  // EBREAK
                instr->type = U_RV32I_EBREAK;
                return;
              case 0x102:  // SRET
                instr->type = P_SRET;
                instr->cont = true;
                return;
              case 0x302:  // MRET
                instr->type = P_MRET;
                instr->cont = true;
                return;
              default:
                __builtin_unreachable();
            }
          } else {  // I-type: Zicsr
            *instr = decode_i_type_with_csr(&un);
            switch (un.itype.funct3) {
              case 0x1:  // CSRRW
                instr->type = U_ZICSR_CSRRW;
                return;
              case 0x2:  // CSRRS
                instr->type = U_ZICSR_CSRRS;
                return;
              case 0x3:  // CSRRC
                instr->type = U_ZICSR_CSRRC;
                return;
              case 0x5:  // CSRRWI
                instr->type = U_ZICSR_CSRRWI;
                return;
              case 0x6:  // CSRRSI
                instr->type = U_ZICSR_CSRRSI;
                return;
              case 0x7:  // CSRRCI
                instr->type = U_ZICSR_CSRRCI;
                return;
              default:
                __builtin_unreachable();
            }  // switch funct3
          }
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
