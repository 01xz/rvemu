#include "interp.h"

#include <math.h>
#include <stdbool.h>

#include "decode.h"
#include "utils.h"

#define __HANDLER_LOAD(type)                               \
  u64 addr = state->gp_regs[instr->rs1] + (i64)instr->imm; \
  state->gp_regs[instr->rd] = *(type*)TO_HOST(addr);

static void handler_lb(State* state, const RvInstr* instr) {
  __HANDLER_LOAD(i8);
}

static void handler_lh(State* state, const RvInstr* instr) {
  __HANDLER_LOAD(i16);
}

static void handler_lw(State* state, const RvInstr* instr) {
  __HANDLER_LOAD(i32);
}

static void handler_ld(State* state, const RvInstr* instr) {
  __HANDLER_LOAD(i64);
}

static void handler_lbu(State* state, const RvInstr* instr) {
  __HANDLER_LOAD(u8);
}

static void handler_lhu(State* state, const RvInstr* instr) {
  __HANDLER_LOAD(u16);
}

static void handler_lwu(State* state, const RvInstr* instr) {
  __HANDLER_LOAD(u32);
}

#define __HANDLER_STORE(type)                              \
  u64 addr = state->gp_regs[instr->rs1] + (i64)instr->imm; \
  *(type*)TO_HOST(addr) = (type)state->gp_regs[instr->rs2];

static void handler_sb(State* state, const RvInstr* instr) {
  __HANDLER_STORE(i8);
}

static void handler_sh(State* state, const RvInstr* instr) {
  __HANDLER_STORE(i16);
}

static void handler_sw(State* state, const RvInstr* instr) {
  __HANDLER_STORE(i32);
}

static void handler_sd(State* state, const RvInstr* instr) {
  __HANDLER_STORE(i64);
}

#define __HANDLER_I_ARITHMETIC(expr)    \
  u64 rs1 = state->gp_regs[instr->rs1]; \
  i64 imm = (i64)instr->imm;            \
  state->gp_regs[instr->rd] = (expr);

static void handler_addi(State* state, const RvInstr* instr) {
  __HANDLER_I_ARITHMETIC(rs1 + imm);
}

static void handler_slli(State* state, const RvInstr* instr) {
  __HANDLER_I_ARITHMETIC(rs1 << (imm & 0x3f));
}

static void handler_slti(State* state, const RvInstr* instr) {
  __HANDLER_I_ARITHMETIC((i64)rs1 < (i64)imm);
}

static void handler_sltiu(State* state, const RvInstr* instr) {
  __HANDLER_I_ARITHMETIC((u64)rs1 < (u64)imm);
}

static void handler_xori(State* state, const RvInstr* instr) {
  __HANDLER_I_ARITHMETIC(rs1 ^ (u64)imm);
}

static void handler_srli(State* state, const RvInstr* instr) {
  __HANDLER_I_ARITHMETIC(rs1 >> (imm & 0x3f));
}

static void handler_srai(State* state, const RvInstr* instr) {
  __HANDLER_I_ARITHMETIC((i64)rs1 >> (imm & 0x3f));
}

static void handler_ori(State* state, const RvInstr* instr) {
  __HANDLER_I_ARITHMETIC(rs1 | (u64)imm);
}

static void handler_andi(State* state, const RvInstr* instr) {
  __HANDLER_I_ARITHMETIC(rs1 & (u64)imm);
}

static void handler_addiw(State* state, const RvInstr* instr) {
  __HANDLER_I_ARITHMETIC((i64)(i32)(rs1 + imm));
}

static void handler_slliw(State* state, const RvInstr* instr) {
  __HANDLER_I_ARITHMETIC((i64)(i32)(rs1 << (imm & 0x1f)));
}

static void handler_srliw(State* state, const RvInstr* instr) {
  __HANDLER_I_ARITHMETIC((i64)(i32)((u32)rs1 >> (imm & 0x1f)));
}

static void handler_sraiw(State* state, const RvInstr* instr) {
  __HANDLER_I_ARITHMETIC((i64)((i32)rs1 >> (imm & 0x1f)));
}

#define __HANDLER_R_ARITHMETIC(expr)    \
  u64 rs1 = state->gp_regs[instr->rs1]; \
  u64 rs2 = state->gp_regs[instr->rs2]; \
  state->gp_regs[instr->rd] = (expr);

static void handler_add(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(rs1 + rs2);
}

static void handler_sub(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(rs1 - rs2);
}

static void handler_sll(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(rs1 << (rs2 & 0x3f));
}

static void handler_slt(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)rs1 < (i64)rs2);
}

static void handler_sltu(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((u64)rs1 < (u64)rs2);
}

static void handler_xor(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(rs1 ^ rs2);
}

static void handler_srl(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(rs1 >> (rs2 & 0x3f));
}

static void handler_sra(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)rs1 >> (rs2 & 0x3f));
}

static void handler_or(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(rs1 | rs2);
}

static void handler_and(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(rs1 & rs2);
}

static void handler_mul(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(rs1 * rs2);
}

static inline u64 __mulhu(u64 a, u64 b) {
  u64 a1 = a >> 32, a0 = (u32)a;
  u64 b1 = b >> 32, b0 = (u32)b;
  u64 w0 = a0 * b0;
  u64 t = a1 * b0 + (w0 >> 32);
  u64 w2 = t >> 32, w1 = (u32)t;
  w1 = a0 * b1 + w1;
  return a1 * b1 + w2 + (w1 >> 32);
}

static inline i64 __mulh(i64 a, i64 b) {
  u64 res = __mulhu((u64)a, (u64)b);
  return res - (b & (a >> 63)) - (a & (b >> 63));
}

static inline i64 __mulhsu(i64 a, u64 b) {
  u64 res = __mulhu((u64)a, b);
  return res - (b & (a >> 63));
}

static void handler_mulh(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(__mulh(rs1, rs2));
}

static void handler_mulhsu(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(__mulhsu(rs1, rs2));
}

static void handler_mulhu(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(__mulhu(rs1, rs2));
}

static inline i64 __div(i64 a, i64 b) {
  if (b == 0) {
    return UINT64_MAX;
  } else if (a == INT64_MIN && b == UINT64_MAX) {
    return INT64_MIN;
  } else {
    return a / b;
  }
}

static void handler_div(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(__div(rs1, rs2));
}

static inline u64 __divu(u64 a, u64 b) {
  if (b == 0) {
    return UINT64_MAX;
  } else {
    return a / b;
  }
}

static void handler_divu(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(__divu(rs1, rs2));
}

static inline i64 __rem(i64 a, i64 b) {
  if (b == 0) {
    return a;
  } else if (a == INT64_MIN && b == UINT64_MAX) {
    return 0;
  } else {
    return a % b;
  }
}

static void handler_rem(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(__rem(rs1, rs2));
}

static inline u64 __remu(u64 a, u64 b) {
  if (b == 0) {
    return a;
  } else {
    return a % b;
  }
}

static void handler_remu(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(__remu(rs1, rs2));
}

static void handler_addw(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)(rs1 + rs2));
}

static void handler_subw(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)(rs1 - rs2));
}

static void handler_sllw(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)(rs1 << (rs2 & 0x1f)));
}

static void handler_srlw(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)((u32)rs1 >> (rs2 & 0x1f)));
}

static void handler_sraw(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)((i32)rs1 >> (rs2 & 0x1f)));
}

static void handler_mulw(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)(rs1 * rs2));
}

static void handler_divw(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)__div((i64)(i32)rs1, (i64)(i32)rs2));
}

static void handler_divuw(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)__divu((u32)rs1, (u32)rs2));
}

static void handler_remw(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)__rem((i64)(i32)rs1, (i64)(i32)rs2));
}

static void handler_remuw(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)__remu((u32)rs1, (u32)rs2));
}

static void handler_auipc(State* state, const RvInstr* instr) {
  state->gp_regs[instr->rd] = state->pc + (i64)instr->imm;
}

static void handler_lui(State* state, const RvInstr* instr) {
  state->gp_regs[instr->rd] = (i64)instr->imm;
}

#define __HANDLER_BRANCH(condi)         \
  u64 rs1 = state->gp_regs[instr->rs1]; \
  u64 rs2 = state->gp_regs[instr->rs2]; \
  if (condi) state->pc += (i64)instr->imm;

static void handler_beq(State* state, const RvInstr* instr) {
  __HANDLER_BRANCH(rs1 == rs2);
}

static void handler_bne(State* state, const RvInstr* instr) {
  __HANDLER_BRANCH(rs1 != rs2);
}

static void handler_blt(State* state, const RvInstr* instr) {
  __HANDLER_BRANCH((i64)rs1 < (i64)rs2);
}

static void handler_bge(State* state, const RvInstr* instr) {
  __HANDLER_BRANCH((i64)rs1 >= (i64)rs2);
}

static void handler_bltu(State* state, const RvInstr* instr) {
  __HANDLER_BRANCH(rs1 < rs2);
}

static void handler_bgeu(State* state, const RvInstr* instr) {
  __HANDLER_BRANCH(rs1 >= rs2);
}

static void handler_jalr(State* state, const RvInstr* instr) {
  state->gp_regs[instr->rd] = state->pc + (instr->rvc ? 2 : 4);
  state->re_enter_pc = (state->gp_regs[instr->rs1] + (i64)instr->imm) & ~(u64)1;
  state->exit_reason = kIndirectBranch;
}

static void handler_jal(State* state, const RvInstr* instr) {
  state->gp_regs[instr->rd] = state->pc + (instr->rvc ? 2 : 4);
  state->re_enter_pc = state->pc = state->pc + (i64)instr->imm;
  state->exit_reason = kDirectBranch;
}

static void handler_ecall(State* state, const RvInstr* instr) {
  state->re_enter_pc = state->pc + 4;
  state->exit_reason = kECall;
}

static void handler_fence(State* state, const RvInstr* instr) {}

static void handler_fencei(State* state, const RvInstr* instr) {}

#define __HANDLER_CSR()   \
  switch (instr->csr) {   \
    case kFflags:         \
    case kFrm:            \
    case kFcsr:           \
      break;              \
    default:              \
      FATAL("TODO: csr"); \
  }                       \
  state->gp_regs[instr->rd] = 0;

static void handler_csrrw(State* state, const RvInstr* instr) {
  __HANDLER_CSR();
}

static void handler_csrrs(State* state, const RvInstr* instr) {
  __HANDLER_CSR();
}

static void handler_csrrc(State* state, const RvInstr* instr) {
  __HANDLER_CSR();
}

static void handler_csrrwi(State* state, const RvInstr* instr) {
  __HANDLER_CSR();
}

static void handler_csrrsi(State* state, const RvInstr* instr) {
  __HANDLER_CSR();
}

static void handler_csrrci(State* state, const RvInstr* instr) {
  __HANDLER_CSR();
}

static void handler_flw(State* state, const RvInstr* instr) {
  u64 addr = state->gp_regs[instr->rs1] + (i64)instr->imm;
  state->fp_regs[instr->rd].wu = *(u32*)TO_HOST(addr);
}

static void handler_fld(State* state, const RvInstr* instr) {
  u64 addr = state->gp_regs[instr->rs1] + (i64)instr->imm;
  state->fp_regs[instr->rd].lu = *(u64*)TO_HOST(addr);
}

#define __HANDLER_STORE_F(type)                            \
  u64 addr = state->gp_regs[instr->rs1] + (i64)instr->imm; \
  *(type*)TO_HOST(addr) = (type)state->fp_regs[instr->rs2].lu;

static void handler_fsw(State* state, const RvInstr* instr) {
  __HANDLER_STORE_F(u32);
}

static void handler_fsd(State* state, const RvInstr* instr) {
  __HANDLER_STORE_F(u64);
}

#define __HANDLER_R_ARITHMETIC_S(expr)                            \
  f32 rs1 = state->fp_regs[instr->rs1].s;                         \
  __attribute__((unused)) f32 rs2 = state->fp_regs[instr->rs2].s; \
  state->fp_regs[instr->rd].s = (f32)(expr);

static void handler_fadd_s(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_S(rs1 + rs2);
}

static void handler_fsub_s(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_S(rs1 - rs2);
}

static void handler_fmul_s(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_S(rs1 * rs2);
}

static void handler_fdiv_s(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_S(rs1 / rs2);
}

static void handler_fsqrt_s(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_S(sqrtf(rs1));
}

static void handler_fmin_s(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_S(rs1 < rs2 ? rs1 : rs2);
}

static void handler_fmax_s(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_S(rs1 > rs2 ? rs1 : rs2);
}

static inline u32 __sgnj_s(u32 a, u32 b, bool n, bool x) {
  u32 sign = (u32)INT32_MIN;
  u32 t = x ? a : n ? sign : 0;
  return (a & ~sign) | ((t ^ b) & sign);
}

#define __HANDLER_SGNJ_S(n, x)             \
  u32 rs1 = state->fp_regs[instr->rs1].wu; \
  u32 rs2 = state->fp_regs[instr->rs2].wu; \
  state->fp_regs[instr->rd].wu = __sgnj_s(rs1, rs2, n, x);

static void handler_fsgnj_s(State* state, const RvInstr* instr) {
  __HANDLER_SGNJ_S(false, false);
}

static void handler_fsgnjn_s(State* state, const RvInstr* instr) {
  __HANDLER_SGNJ_S(true, false);
}

static void handler_fsgnjx_s(State* state, const RvInstr* instr) {
  __HANDLER_SGNJ_S(false, true);
}

#define __HANDLER_R_ARITHMETIC_D(expr)                            \
  f64 rs1 = state->fp_regs[instr->rs1].d;                         \
  __attribute__((unused)) f64 rs2 = state->fp_regs[instr->rs2].d; \
  state->fp_regs[instr->rd].d = (expr);

static void handler_fadd_d(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_D(rs1 + rs2);
}

static void handler_fsub_d(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_D(rs1 + rs2);
}

static void handler_fmul_d(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_D(rs1 + rs2);
}

static void handler_fdiv_d(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_D(rs1 + rs2);
}

static void handler_fsqrt_d(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_D(sqrt(rs1));
}

static void handler_fmin_d(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_D(rs1 < rs2 ? rs1 : rs2);
}

static void handler_fmax_d(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_D(rs1 > rs2 ? rs1 : rs2);
}

static inline u64 __sgnj_d(u64 a, u64 b, bool n, bool x) {
  u64 sign = (u64)INT64_MIN;
  u64 t = x ? a : n ? sign : 0;
  return (a & ~sign) | ((t ^ b) & sign);
}

#define __HANDLER_SGNJ_D(n, x)             \
  u64 rs1 = state->fp_regs[instr->rs1].lu; \
  u64 rs2 = state->fp_regs[instr->rs2].lu; \
  state->fp_regs[instr->rd].lu = __sgnj_d(rs1, rs2, n, x);

static void handler_fsgnj_d(State* state, const RvInstr* instr) {
  __HANDLER_SGNJ_D(false, false);
}

static void handler_fsgnjn_d(State* state, const RvInstr* instr) {
  __HANDLER_SGNJ_D(true, false);
}

static void handler_fsgnjx_d(State* state, const RvInstr* instr) {
  __HANDLER_SGNJ_D(false, true);
}

#define __HANDLER_R_ARITHMETIC_FUSED_S(expr) \
  f32 rs1 = state->fp_regs[instr->rs1].s;    \
  f32 rs2 = state->fp_regs[instr->rs2].s;    \
  f32 rs3 = state->fp_regs[instr->rs3].s;    \
  state->fp_regs[instr->rd].s = (f32)(expr);

static void handler_fmadd_s(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_FUSED_S(rs1 * rs2 + rs3);
}

static void handler_fmsub_s(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_FUSED_S(rs1 * rs2 - rs3);
}

static void handler_fnmsub_s(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_FUSED_S(-(rs1 * rs2) - rs3);
}

static void handler_fnmadd_s(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_FUSED_S(-(rs1 * rs2) + rs3);
}

#define __HANDLER_R_ARITHMETIC_FUSED_D(expr) \
  f64 rs1 = state->fp_regs[instr->rs1].d;    \
  f64 rs2 = state->fp_regs[instr->rs2].d;    \
  f64 rs3 = state->fp_regs[instr->rs3].d;    \
  state->fp_regs[instr->rd].d = (expr);

static void handler_fmadd_d(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_FUSED_D(rs1 * rs2 + rs3);
}

static void handler_fmsub_d(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_FUSED_D(rs1 * rs2 - rs3);
}

static void handler_fnmsub_d(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_FUSED_D(-(rs1 * rs2) - rs3);
}

static void handler_fnmadd_d(State* state, const RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_FUSED_D(-(rs1 * rs2) + rs3);
}

#define __HANDLER_R_COMPARE_S(expr)       \
  f32 rs1 = state->fp_regs[instr->rs1].s; \
  f32 rs2 = state->fp_regs[instr->rs2].s; \
  state->gp_regs[instr->rd] = (u64)(expr);

static void handler_fle_s(State* state, const RvInstr* instr) {
  __HANDLER_R_COMPARE_S(rs1 <= rs2);
}

static void handler_flt_s(State* state, const RvInstr* instr) {
  __HANDLER_R_COMPARE_S(rs1 < rs2);
}

static void handler_feq_s(State* state, const RvInstr* instr) {
  __HANDLER_R_COMPARE_S(rs1 == rs2);
}

#define __HANDLER_R_COMPARE_D(expr)       \
  f64 rs1 = state->fp_regs[instr->rs1].d; \
  f64 rs2 = state->fp_regs[instr->rs2].d; \
  state->gp_regs[instr->rd] = (u64)(expr);

static void handler_fle_d(State* state, const RvInstr* instr) {
  __HANDLER_R_COMPARE_D(rs1 <= rs2);
}

static void handler_flt_d(State* state, const RvInstr* instr) {
  __HANDLER_R_COMPARE_D(rs1 < rs2);
}

static void handler_feq_d(State* state, const RvInstr* instr) {
  __HANDLER_R_COMPARE_D(rs1 == rs2);
}

static void handler_fcvt_s_d(State* state, const RvInstr* instr) {
  state->fp_regs[instr->rd].s = (f32)state->fp_regs[instr->rs1].d;
}

static void handler_fcvt_d_s(State* state, const RvInstr* instr) {
  state->fp_regs[instr->rd].d = (f64)state->fp_regs[instr->rs1].s;
}

static void handler_fcvt_w_s(State* state, const RvInstr* instr) {
  state->gp_regs[instr->rd] = (i64)(i32)llrintf(state->fp_regs[instr->rs1].s);
}

static void handler_fcvt_wu_s(State* state, const RvInstr* instr) {
  state->gp_regs[instr->rd] =
      (i64)(i32)(u32)llrintf(state->fp_regs[instr->rs1].s);
}

static void handler_fcvt_l_s(State* state, const RvInstr* instr) {
  state->gp_regs[instr->rd] = (i64)llrintf(state->fp_regs[instr->rs1].s);
}

static void handler_fcvt_lu_s(State* state, const RvInstr* instr) {
  state->gp_regs[instr->rd] = (u64)llrintf(state->fp_regs[instr->rs1].s);
}

static void handler_fcvt_s_w(State* state, const RvInstr* instr) {
  state->fp_regs[instr->rd].s = (f32)(i32)state->gp_regs[instr->rs1];
}

static void handler_fcvt_s_wu(State* state, const RvInstr* instr) {
  state->fp_regs[instr->rd].s = (f32)(u32)state->gp_regs[instr->rs1];
}

static void handler_fcvt_s_l(State* state, const RvInstr* instr) {
  state->fp_regs[instr->rd].s = (f32)(i64)state->gp_regs[instr->rs1];
}

static void handler_fcvt_s_lu(State* state, const RvInstr* instr) {
  state->fp_regs[instr->rd].s = (f32)(u64)state->gp_regs[instr->rs1];
}

static void handler_fcvt_w_d(State* state, const RvInstr* instr) {
  state->gp_regs[instr->rd] = (i64)(i32)llrintf(state->fp_regs[instr->rs1].d);
}

static void handler_fcvt_wu_d(State* state, const RvInstr* instr) {
  state->gp_regs[instr->rd] =
      (i64)(i32)(u32)llrintf(state->fp_regs[instr->rs1].d);
}

static void handler_fcvt_l_d(State* state, const RvInstr* instr) {
  state->gp_regs[instr->rd] = (i64)llrintf(state->fp_regs[instr->rs1].d);
}

static void handler_fcvt_lu_d(State* state, const RvInstr* instr) {
  state->gp_regs[instr->rd] = (u64)llrintf(state->fp_regs[instr->rs1].d);
}

static void handler_fcvt_d_w(State* state, const RvInstr* instr) {
  state->fp_regs[instr->rd].d = (f64)(i32)state->gp_regs[instr->rs1];
}

static void handler_fcvt_d_wu(State* state, const RvInstr* instr) {
  state->fp_regs[instr->rd].d = (f64)(u32)state->gp_regs[instr->rs1];
}

static void handler_fcvt_d_l(State* state, const RvInstr* instr) {
  state->fp_regs[instr->rd].d = (f64)(i64)state->gp_regs[instr->rs1];
}

static void handler_fcvt_d_lu(State* state, const RvInstr* instr) {
  state->fp_regs[instr->rd].d = (f64)(u64)state->gp_regs[instr->rs1];
}

static void handler_fmv_x_w(State* state, const RvInstr* instr) {
  state->gp_regs[instr->rd] = (u64)(i64)(i32)state->fp_regs[instr->rs1].wu;
}

static void handler_fmv_w_x(State* state, const RvInstr* instr) {
  state->fp_regs[instr->rd].wu = (u32)state->gp_regs[instr->rs1];
}

static void handler_fmv_x_d(State* state, const RvInstr* instr) {
  state->gp_regs[instr->rd] = state->fp_regs[instr->rs1].lu;
}

static void handler_fmv_d_x(State* state, const RvInstr* instr) {
  state->fp_regs[instr->rd].lu = state->gp_regs[instr->rs1];
}

static inline u64 __classify_s(f32 in) {
  union {
    struct {
      bool sign : 1;
      u8 exp    : 8;
      u32 frac  : 23;
    } w;
    f32 s;
  } un = {.s = in};

  bool sign = un.w.sign;
  bool inf_or_nan = (un.w.exp == 0xff);
  bool sub_or_zero = (un.w.exp == 0);
  bool frac_zero = (un.w.frac == 0);
  bool is_nan = inf_or_nan && !frac_zero;
  bool is_signaling_nan = inf_or_nan && (un.w.frac & 0x3fffff);

  return (sign && inf_or_nan && frac_zero)      << 0 |
         (sign && !inf_or_nan && !sub_or_zero)  << 1 |
         (sign && sub_or_zero && !frac_zero)    << 2 |
         (sign && sub_or_zero && frac_zero)     << 3 |
         (!sign && inf_or_nan && frac_zero)     << 7 |
         (!sign && !inf_or_nan && !sub_or_zero) << 6 |
         (!sign && sub_or_zero && !frac_zero)   << 5 |
         (!sign && sub_or_zero && frac_zero)    << 4 |
         (is_nan && is_signaling_nan)           << 8 |
         (is_nan && !is_signaling_nan)          << 9;
}

static void handler_fclass_s(State* state, const RvInstr* instr) {
  state->gp_regs[instr->rd] = __classify_s(state->fp_regs[instr->rs1].s);
}

static inline u64 __classify_d(f64 in) {
  union {
    struct {
      bool sign : 1;
      u16 exp   : 11;
      u64 frac  : 52;
    } l;
    f64 d;
  } un = {.d = in};

  bool sign = un.l.sign;
  bool inf_or_nan = (un.l.exp == 0x7ff);
  bool sub_or_zero = (un.l.exp == 0);
  bool frac_zero = (un.l.frac == 0);
  bool is_nan = inf_or_nan && !frac_zero;
  bool is_signaling_nan = inf_or_nan && (un.l.frac & 0x7ffffffffffff);

  return (sign && inf_or_nan && frac_zero)      << 0 |
         (sign && !inf_or_nan && !sub_or_zero)  << 1 |
         (sign && sub_or_zero && !frac_zero)    << 2 |
         (sign && sub_or_zero && frac_zero)     << 3 |
         (!sign && inf_or_nan && frac_zero)     << 7 |
         (!sign && !inf_or_nan && !sub_or_zero) << 6 |
         (!sign && sub_or_zero && !frac_zero)   << 5 |
         (!sign && sub_or_zero && frac_zero)    << 4 |
         (is_nan && is_signaling_nan)           << 8 |
         (is_nan && !is_signaling_nan)          << 9;
}

static void handler_fclass_d(State* state, const RvInstr* instr) {
  state->gp_regs[instr->rd] = __classify_d(state->fp_regs[instr->rs1].d);
}

static void (*rv_instr_handler[kRvInstrNum])(State*, const RvInstr*) = {
    [kLb] = handler_lb,
    [kLh] = handler_lh,
    [kLw] = handler_lw,
    [kLd] = handler_ld,
    [kLbu] = handler_lbu,
    [kLhu] = handler_lhu,
    [kLwu] = handler_lwu,
    [kFence] = handler_fence,
    [kFenceI] = handler_fencei,
    [kAddi] = handler_addi,
    [kSlli] = handler_slli,
    [kSlti] = handler_slti,
    [kSltiu] = handler_sltiu,
    [kXori] = handler_xori,
    [kSrli] = handler_srli,
    [kSrai] = handler_srai,
    [kOri] = handler_ori,
    [kAndi] = handler_andi,
    [kAuipc] = handler_auipc,
    [kAddiw] = handler_addiw,
    [kSlliw] = handler_slliw,
    [kSrliw] = handler_srliw,
    [kSraiw] = handler_sraiw,
    [kSb] = handler_sb,
    [kSh] = handler_sh,
    [kSw] = handler_sw,
    [kSd] = handler_sd,
    [kAdd] = handler_add,
    [kSub] = handler_sub,
    [kSll] = handler_sll,
    [kSlt] = handler_slt,
    [kSltu] = handler_sltu,
    [kXor] = handler_xor,
    [kSrl] = handler_srl,
    [kSra] = handler_sra,
    [kOr] = handler_or,
    [kAnd] = handler_and,
    [kMul] = handler_mul,
    [kMulh] = handler_mulh,
    [kMulhsu] = handler_mulhsu,
    [kMulhu] = handler_mulhu,
    [kDiv] = handler_div,
    [kDivu] = handler_divu,
    [kRem] = handler_rem,
    [kRemu] = handler_remu,
    [kLui] = handler_lui,
    [kAddw] = handler_addw,
    [kSubw] = handler_subw,
    [kSllw] = handler_sllw,
    [kSrlw] = handler_srlw,
    [kSraw] = handler_sraw,
    [kMulw] = handler_mulw,
    [kDivw] = handler_divw,
    [kDivuw] = handler_divuw,
    [kRemw] = handler_remw,
    [kRemuw] = handler_remuw,
    [kBeq] = handler_beq,
    [kBne] = handler_bne,
    [kBlt] = handler_blt,
    [kBge] = handler_bge,
    [kBltu] = handler_bltu,
    [kBgeu] = handler_bgeu,
    [kJalr] = handler_jalr,
    [kJal] = handler_jal,
    [kEcall] = handler_ecall,
    [kCsrrw] = handler_csrrw,
    [kCsrrs] = handler_csrrs,
    [kCsrrc] = handler_csrrc,
    [kCsrrwi] = handler_csrrwi,
    [kCsrrsi] = handler_csrrsi,
    [kCsrrci] = handler_csrrci,
    [kFlw] = handler_flw,
    [kFsw] = handler_fsw,
    [kFmaddS] = handler_fmadd_s,
    [kFmsubS] = handler_fmsub_s,
    [kFnmsubS] = handler_fnmsub_s,
    [kFnmaddS] = handler_fnmadd_s,
    [kFaddS] = handler_fadd_s,
    [kFsubS] = handler_fsub_s,
    [kFmulS] = handler_fmul_s,
    [kFdivS] = handler_fdiv_s,
    [kFsgnjS] = handler_fsgnj_s,
    [kFsgnjnS] = handler_fsgnjn_s,
    [kFsgnjxS] = handler_fsgnjx_s,
    [kFminS] = handler_fmin_s,
    [kFmaxS] = handler_fmax_s,
    [kFsqrtS] = handler_fsqrt_s,
    [kFleS] = handler_fle_s,
    [kFltS] = handler_flt_s,
    [kFeqS] = handler_feq_s,
    [kFcvtWS] = handler_fcvt_w_s,
    [kFcvtWuS] = handler_fcvt_wu_s,
    [kFcvtLS] = handler_fcvt_l_s,
    [kFcvtLuS] = handler_fcvt_lu_s,
    [kFcvtSW] = handler_fcvt_s_w,
    [kFcvtSWu] = handler_fcvt_s_wu,
    [kFcvtSL] = handler_fcvt_s_l,
    [kFcvtSLu] = handler_fcvt_s_lu,
    [kFmvXW] = handler_fmv_x_w,
    [kFclassS] = handler_fclass_s,
    [kFmvWX] = handler_fmv_w_x,
    [kFld] = handler_fld,
    [kFsd] = handler_fsd,
    [kFmaddD] = handler_fmadd_d,
    [kFmsubD] = handler_fmsub_d,
    [kFnmsubD] = handler_fnmsub_d,
    [kFnmaddD] = handler_fnmadd_d,
    [kFaddD] = handler_fadd_d,
    [kFsubD] = handler_fsub_d,
    [kFmulD] = handler_fmul_d,
    [kFdivD] = handler_fdiv_d,
    [kFsgnjD] = handler_fsgnj_d,
    [kFsgnjnD] = handler_fsgnjn_d,
    [kFsgnjxD] = handler_fsgnjx_d,
    [kFminD] = handler_fmin_d,
    [kFmaxD] = handler_fmax_d,
    [kFcvtSD] = handler_fcvt_s_d,
    [kFcvtDS] = handler_fcvt_d_s,
    [kFsqrtD] = handler_fsqrt_d,
    [kFleD] = handler_fle_d,
    [kFltD] = handler_flt_d,
    [kFeqD] = handler_feq_d,
    [kFcvtWD] = handler_fcvt_w_d,
    [kFcvtWuD] = handler_fcvt_wu_d,
    [kFcvtLD] = handler_fcvt_l_d,
    [kFcvtLuD] = handler_fcvt_lu_d,
    [kFcvtDW] = handler_fcvt_d_w,
    [kFcvtDWu] = handler_fcvt_d_wu,
    [kFcvtDL] = handler_fcvt_d_l,
    [kFcvtDLu] = handler_fcvt_d_lu,
    [kFmvXD] = handler_fmv_x_d,
    [kFclassD] = handler_fclass_d,
    [kFmvDX] = handler_fmv_d_x,
};

void exec_block_interp(State* state) {
  static RvInstr instr = {0};
  while (true) {
    u32 instr_raw = *(u32*)TO_HOST(state->pc);
    rv_instr_decode(&instr, instr_raw);
    rv_instr_handler[instr.type](state, &instr);

    state->gp_regs[kZero] = 0;

    if (instr.cont) break;

    state->pc += instr.rvc ? 2 : 4;
  }
}
