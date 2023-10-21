#include "interp.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "decode.h"
#include "utils.h"

static void handler_lui(State* state, RvInstr* instr) {
  state->xregs[instr->rd] = (i64)instr->imm;
}

static void handler_auipc(State* state, RvInstr* instr) {
  state->xregs[instr->rd] = state->pc + (i64)instr->imm;
}

static void handler_jal(State* state, RvInstr* instr) {
  state->xregs[instr->rd] = state->pc + (instr->rvc ? 2 : 4);
  state->re_enter_pc = state->pc + (i64)instr->imm;
  state->exit_reason = kDirectBranch;
}

static void handler_jalr(State* state, RvInstr* instr) {
  u64 xreg_rs1 = state->xregs[instr->rs1];
  state->xregs[instr->rd] = state->pc + (instr->rvc ? 2 : 4);
  state->re_enter_pc = (xreg_rs1 + (i64)instr->imm) & ~(u64)1;
  state->exit_reason = kIndirectBranch;
}

#define __HANDLER_BRANCH(condi)                       \
  u64 rs1 = state->xregs[instr->rs1];                 \
  u64 rs2 = state->xregs[instr->rs2];                 \
  if (condi) {                                        \
    state->re_enter_pc = state->pc + (i64)instr->imm; \
    state->exit_reason = kDirectBranch;               \
    instr->cont = true;                               \
  }

static void handler_beq(State* state, RvInstr* instr) {
  __HANDLER_BRANCH(rs1 == rs2);
}

static void handler_bne(State* state, RvInstr* instr) {
  __HANDLER_BRANCH(rs1 != rs2);
}

static void handler_blt(State* state, RvInstr* instr) {
  __HANDLER_BRANCH((i64)rs1 < (i64)rs2);
}

static void handler_bge(State* state, RvInstr* instr) {
  __HANDLER_BRANCH((i64)rs1 >= (i64)rs2);
}

static void handler_bltu(State* state, RvInstr* instr) {
  __HANDLER_BRANCH(rs1 < rs2);
}

static void handler_bgeu(State* state, RvInstr* instr) {
  __HANDLER_BRANCH(rs1 >= rs2);
}

#define __HANDLER_LOAD(type)                             \
  u64 addr = state->xregs[instr->rs1] + (i64)instr->imm; \
  state->xregs[instr->rd] = *(type*)TO_HOST(addr);

static void handler_lb(State* state, RvInstr* instr) { __HANDLER_LOAD(i8); }

static void handler_lh(State* state, RvInstr* instr) { __HANDLER_LOAD(i16); }

static void handler_lw(State* state, RvInstr* instr) { __HANDLER_LOAD(i32); }

static void handler_ld(State* state, RvInstr* instr) { __HANDLER_LOAD(i64); }

static void handler_lbu(State* state, RvInstr* instr) { __HANDLER_LOAD(u8); }

static void handler_lhu(State* state, RvInstr* instr) { __HANDLER_LOAD(u16); }

static void handler_lwu(State* state, RvInstr* instr) { __HANDLER_LOAD(u32); }

#define __HANDLER_STORE(type)                            \
  u64 addr = state->xregs[instr->rs1] + (i64)instr->imm; \
  *(type*)TO_HOST(addr) = (type)state->xregs[instr->rs2];

static void handler_sb(State* state, RvInstr* instr) { __HANDLER_STORE(i8); }

static void handler_sh(State* state, RvInstr* instr) { __HANDLER_STORE(i16); }

static void handler_sw(State* state, RvInstr* instr) { __HANDLER_STORE(i32); }

static void handler_sd(State* state, RvInstr* instr) { __HANDLER_STORE(i64); }

#define __HANDLER_I_ARITHMETIC(expr)  \
  u64 rs1 = state->xregs[instr->rs1]; \
  i64 imm = (i64)instr->imm;          \
  state->xregs[instr->rd] = (expr);

static void handler_addi(State* state, RvInstr* instr) {
  __HANDLER_I_ARITHMETIC(rs1 + imm);
}

static void handler_slti(State* state, RvInstr* instr) {
  __HANDLER_I_ARITHMETIC((i64)rs1 < (i64)imm);
}

static void handler_sltiu(State* state, RvInstr* instr) {
  __HANDLER_I_ARITHMETIC((u64)rs1 < (u64)imm);
}

static void handler_xori(State* state, RvInstr* instr) {
  __HANDLER_I_ARITHMETIC(rs1 ^ (u64)imm);
}

static void handler_ori(State* state, RvInstr* instr) {
  __HANDLER_I_ARITHMETIC(rs1 | (u64)imm);
}

static void handler_andi(State* state, RvInstr* instr) {
  __HANDLER_I_ARITHMETIC(rs1 & (u64)imm);
}

static void handler_slli(State* state, RvInstr* instr) {
  __HANDLER_I_ARITHMETIC(rs1 << (imm & 0x3f));
}

static void handler_srli(State* state, RvInstr* instr) {
  __HANDLER_I_ARITHMETIC(rs1 >> (imm & 0x3f));
}

static void handler_srai(State* state, RvInstr* instr) {
  __HANDLER_I_ARITHMETIC((i64)rs1 >> (imm & 0x3f));
}

static void handler_addiw(State* state, RvInstr* instr) {
  __HANDLER_I_ARITHMETIC((i64)(i32)(rs1 + imm));
}

static void handler_slliw(State* state, RvInstr* instr) {
  __HANDLER_I_ARITHMETIC((i64)(i32)(rs1 << (imm & 0x1f)));
}

static void handler_srliw(State* state, RvInstr* instr) {
  __HANDLER_I_ARITHMETIC((i64)(i32)((u32)rs1 >> (imm & 0x1f)));
}

static void handler_sraiw(State* state, RvInstr* instr) {
  __HANDLER_I_ARITHMETIC((i64)((i32)rs1 >> (imm & 0x1f)));
}

#define __HANDLER_R_ARITHMETIC(expr)  \
  u64 rs1 = state->xregs[instr->rs1]; \
  u64 rs2 = state->xregs[instr->rs2]; \
  state->xregs[instr->rd] = (expr);

static void handler_add(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(rs1 + rs2);
}

static void handler_sub(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(rs1 - rs2);
}

static void handler_sll(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(rs1 << (rs2 & 0x3f));
}

static void handler_slt(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)rs1 < (i64)rs2);
}

static void handler_sltu(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((u64)rs1 < (u64)rs2);
}

static void handler_xor(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(rs1 ^ rs2);
}

static void handler_srl(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(rs1 >> (rs2 & 0x3f));
}

static void handler_sra(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)rs1 >> (rs2 & 0x3f));
}

static void handler_or(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(rs1 | rs2);
}

static void handler_and(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(rs1 & rs2);
}

static void handler_addw(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)(rs1 + rs2));
}

static void handler_subw(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)(rs1 - rs2));
}

static void handler_sllw(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)(rs1 << (rs2 & 0x1f)));
}

static void handler_srlw(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)((u32)rs1 >> (rs2 & 0x1f)));
}

static void handler_sraw(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)((i32)rs1 >> (rs2 & 0x1f)));
}

static void handler_mul(State* state, RvInstr* instr) {
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

static void handler_mulh(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(__mulh(rs1, rs2));
}

static void handler_mulhsu(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(__mulhsu(rs1, rs2));
}

static void handler_mulhu(State* state, RvInstr* instr) {
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

static void handler_div(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(__div(rs1, rs2));
}

static inline u64 __divu(u64 a, u64 b) {
  if (b == 0) {
    return UINT64_MAX;
  } else {
    return a / b;
  }
}

static void handler_divu(State* state, RvInstr* instr) {
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

static void handler_rem(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(__rem(rs1, rs2));
}

static inline u64 __remu(u64 a, u64 b) {
  if (b == 0) {
    return a;
  } else {
    return a % b;
  }
}

static void handler_remu(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC(__remu(rs1, rs2));
}

static void handler_mulw(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)(rs1 * rs2));
}

static void handler_divw(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)__div((i64)(i32)rs1, (i64)(i32)rs2));
}

static void handler_divuw(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)__divu((u32)rs1, (u32)rs2));
}

static void handler_remw(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)__rem((i64)(i32)rs1, (i64)(i32)rs2));
}

static void handler_remuw(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC((i64)(i32)__remu((u32)rs1, (u32)rs2));
}

static void handler_ecall(State* state, RvInstr* instr) {
  state->re_enter_pc = state->pc + 4;
  state->exit_reason = kECall;
}

static u64 load_csr(const State* state, u16 addr) {
  if (addr == CSR_SIE) {
    return state->csrs[CSR_MIE] & state->csrs[CSR_MIDELEG];
  }
  return state->csrs[addr];
}

static void store_csr(State* state, u16 addr, u64 value) {
  if (addr == CSR_SIE) {
    state->csrs[addr] = (state->csrs[CSR_MIE] & ~(state->csrs[CSR_MIDELEG])) |
                        (value & state->csrs[CSR_MIDELEG]);
    return;
  }
  state->csrs[addr] = value;
}

static void update_paging(State* state, u16 addr) {
  state->page_table =
      (load_csr(state, CSR_SATP) & (((u64)1 << 44) - 1)) * PAGE_SIZE;
  u64 mode = load_csr(state, CSR_SATP) >> 60;

  if (mode == 8) {
    state->enable_paging = true;
  } else {
    state->enable_paging = false;
  }
}

#define __HANDLER_CSR(expr)            \
  u64 t = load_csr(state, instr->csr); \
  store_csr(state, instr->csr, expr);  \
  state->xregs[instr->rd] = t;         \
  if (instr->csr == CSR_SATP) {        \
    update_paging(state, instr->csr);  \
  }

static void handler_csrrw(State* state, RvInstr* instr) {
  __HANDLER_CSR(state->xregs[instr->rs1]);
}

static void handler_csrrs(State* state, RvInstr* instr) {
  __HANDLER_CSR(t | state->xregs[instr->rs1]);
}

static void handler_csrrc(State* state, RvInstr* instr) {
  __HANDLER_CSR(t & ~(state->xregs[instr->rs1]));
}

static void handler_csrrwi(State* state, RvInstr* instr) {
  __HANDLER_CSR(instr->rs1);
}

static void handler_csrrsi(State* state, RvInstr* instr) {
  __HANDLER_CSR(t | instr->rs1);
}

static void handler_csrrci(State* state, RvInstr* instr) {
  __HANDLER_CSR(t & ~(instr->rs1));
}

static void handler_flw(State* state, RvInstr* instr) {
  u64 addr = state->xregs[instr->rs1] + (i64)instr->imm;
  state->fregs[instr->rd].lu = *(u32*)TO_HOST(addr) | (UINT64_MAX << 32);
}

static void handler_fld(State* state, RvInstr* instr) {
  u64 addr = state->xregs[instr->rs1] + (i64)instr->imm;
  state->fregs[instr->rd].lu = *(u64*)TO_HOST(addr);
}

#define __HANDLER_STORE_F(type)                          \
  u64 addr = state->xregs[instr->rs1] + (i64)instr->imm; \
  *(type*)TO_HOST(addr) = (type)state->fregs[instr->rs2].lu;

static void handler_fsw(State* state, RvInstr* instr) {
  __HANDLER_STORE_F(u32);
}

static void handler_fsd(State* state, RvInstr* instr) {
  __HANDLER_STORE_F(u64);
}

#define __HANDLER_R_ARITHMETIC_S(expr)                          \
  f32 rs1 = state->fregs[instr->rs1].s;                         \
  __attribute__((unused)) f32 rs2 = state->fregs[instr->rs2].s; \
  state->fregs[instr->rd].s = (f32)(expr);

static void handler_fadd_s(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_S(rs1 + rs2);
}

static void handler_fsub_s(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_S(rs1 - rs2);
}

static void handler_fmul_s(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_S(rs1 * rs2);
}

static void handler_fdiv_s(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_S(rs1 / rs2);
}

static void handler_fsqrt_s(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_S(sqrtf(rs1));
}

static void handler_fmin_s(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_S(rs1 < rs2 ? rs1 : rs2);
}

static void handler_fmax_s(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_S(rs1 > rs2 ? rs1 : rs2);
}

static inline u32 __sgnj_s(u32 a, u32 b, bool n, bool x) {
  u32 sign = (u32)INT32_MIN;
  u32 t = x ? a : n ? sign : 0;
  return (a & ~sign) | ((t ^ b) & sign);
}

#define __HANDLER_SGNJ_S(n, x)           \
  u32 rs1 = state->fregs[instr->rs1].wu; \
  u32 rs2 = state->fregs[instr->rs2].wu; \
  state->fregs[instr->rd].lu =           \
      (u64)__sgnj_s(rs1, rs2, n, x) | (UINT64_MAX << 32);

static void handler_fsgnj_s(State* state, RvInstr* instr) {
  __HANDLER_SGNJ_S(false, false);
}

static void handler_fsgnjn_s(State* state, RvInstr* instr) {
  __HANDLER_SGNJ_S(true, false);
}

static void handler_fsgnjx_s(State* state, RvInstr* instr) {
  __HANDLER_SGNJ_S(false, true);
}

#define __HANDLER_R_ARITHMETIC_D(expr)                          \
  f64 rs1 = state->fregs[instr->rs1].d;                         \
  __attribute__((unused)) f64 rs2 = state->fregs[instr->rs2].d; \
  state->fregs[instr->rd].d = (expr);

static void handler_fadd_d(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_D(rs1 + rs2);
}

static void handler_fsub_d(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_D(rs1 - rs2);
}

static void handler_fmul_d(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_D(rs1 * rs2);
}

static void handler_fdiv_d(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_D(rs1 / rs2);
}

static void handler_fsqrt_d(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_D(sqrt(rs1));
}

static void handler_fmin_d(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_D(rs1 < rs2 ? rs1 : rs2);
}

static void handler_fmax_d(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_D(rs1 > rs2 ? rs1 : rs2);
}

static inline u64 __sgnj_d(u64 a, u64 b, bool n, bool x) {
  u64 sign = (u64)INT64_MIN;
  u64 t = x ? a : n ? sign : 0;
  return (a & ~sign) | ((t ^ b) & sign);
}

#define __HANDLER_SGNJ_D(n, x)           \
  u64 rs1 = state->fregs[instr->rs1].lu; \
  u64 rs2 = state->fregs[instr->rs2].lu; \
  state->fregs[instr->rd].lu = __sgnj_d(rs1, rs2, n, x);

static void handler_fsgnj_d(State* state, RvInstr* instr) {
  __HANDLER_SGNJ_D(false, false);
}

static void handler_fsgnjn_d(State* state, RvInstr* instr) {
  __HANDLER_SGNJ_D(true, false);
}

static void handler_fsgnjx_d(State* state, RvInstr* instr) {
  __HANDLER_SGNJ_D(false, true);
}

#define __HANDLER_R_ARITHMETIC_FUSED_S(expr) \
  f32 rs1 = state->fregs[instr->rs1].s;      \
  f32 rs2 = state->fregs[instr->rs2].s;      \
  f32 rs3 = state->fregs[instr->rs3].s;      \
  state->fregs[instr->rd].s = (f32)(expr);

static void handler_fmadd_s(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_FUSED_S(rs1 * rs2 + rs3);
}

static void handler_fmsub_s(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_FUSED_S(rs1 * rs2 - rs3);
}

static void handler_fnmsub_s(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_FUSED_S(-(rs1 * rs2) + rs3);
}

static void handler_fnmadd_s(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_FUSED_S(-(rs1 * rs2) - rs3);
}

#define __HANDLER_R_ARITHMETIC_FUSED_D(expr) \
  f64 rs1 = state->fregs[instr->rs1].d;      \
  f64 rs2 = state->fregs[instr->rs2].d;      \
  f64 rs3 = state->fregs[instr->rs3].d;      \
  state->fregs[instr->rd].d = (expr);

static void handler_fmadd_d(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_FUSED_D(rs1 * rs2 + rs3);
}

static void handler_fmsub_d(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_FUSED_D(rs1 * rs2 - rs3);
}

static void handler_fnmsub_d(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_FUSED_D(-(rs1 * rs2) + rs3);
}

static void handler_fnmadd_d(State* state, RvInstr* instr) {
  __HANDLER_R_ARITHMETIC_FUSED_D(-(rs1 * rs2) - rs3);
}

#define __HANDLER_R_COMPARE_S(expr)     \
  f32 rs1 = state->fregs[instr->rs1].s; \
  f32 rs2 = state->fregs[instr->rs2].s; \
  state->xregs[instr->rd] = (u64)(expr);

static void handler_fle_s(State* state, RvInstr* instr) {
  __HANDLER_R_COMPARE_S(rs1 <= rs2);
}

static void handler_flt_s(State* state, RvInstr* instr) {
  __HANDLER_R_COMPARE_S(rs1 < rs2);
}

static void handler_feq_s(State* state, RvInstr* instr) {
  __HANDLER_R_COMPARE_S(rs1 == rs2);
}

#define __HANDLER_R_COMPARE_D(expr)     \
  f64 rs1 = state->fregs[instr->rs1].d; \
  f64 rs2 = state->fregs[instr->rs2].d; \
  state->xregs[instr->rd] = (u64)(expr);

static void handler_fle_d(State* state, RvInstr* instr) {
  __HANDLER_R_COMPARE_D(rs1 <= rs2);
}

static void handler_flt_d(State* state, RvInstr* instr) {
  __HANDLER_R_COMPARE_D(rs1 < rs2);
}

static void handler_feq_d(State* state, RvInstr* instr) {
  __HANDLER_R_COMPARE_D(rs1 == rs2);
}

static void handler_fcvt_s_d(State* state, RvInstr* instr) {
  state->fregs[instr->rd].s = (f32)state->fregs[instr->rs1].d;
}

static void handler_fcvt_d_s(State* state, RvInstr* instr) {
  state->fregs[instr->rd].d = (f64)state->fregs[instr->rs1].s;
}

static void handler_fcvt_w_s(State* state, RvInstr* instr) {
  state->xregs[instr->rd] = (i64)(i32)llrintf(state->fregs[instr->rs1].s);
}

static void handler_fcvt_wu_s(State* state, RvInstr* instr) {
  state->xregs[instr->rd] = (i64)(i32)(u32)llrintf(state->fregs[instr->rs1].s);
}

static void handler_fcvt_l_s(State* state, RvInstr* instr) {
  state->xregs[instr->rd] = (i64)llrintf(state->fregs[instr->rs1].s);
}

static void handler_fcvt_lu_s(State* state, RvInstr* instr) {
  state->xregs[instr->rd] = (u64)llrintf(state->fregs[instr->rs1].s);
}

static void handler_fcvt_s_w(State* state, RvInstr* instr) {
  state->fregs[instr->rd].s = (f32)(i32)state->xregs[instr->rs1];
}

static void handler_fcvt_s_wu(State* state, RvInstr* instr) {
  state->fregs[instr->rd].s = (f32)(u32)state->xregs[instr->rs1];
}

static void handler_fcvt_s_l(State* state, RvInstr* instr) {
  state->fregs[instr->rd].s = (f32)(i64)state->xregs[instr->rs1];
}

static void handler_fcvt_s_lu(State* state, RvInstr* instr) {
  state->fregs[instr->rd].s = (f32)(u64)state->xregs[instr->rs1];
}

static void handler_fcvt_w_d(State* state, RvInstr* instr) {
  state->xregs[instr->rd] = (i64)(i32)llrint(state->fregs[instr->rs1].d);
}

static void handler_fcvt_wu_d(State* state, RvInstr* instr) {
  state->xregs[instr->rd] = (i64)(i32)(u32)llrint(state->fregs[instr->rs1].d);
}

static void handler_fcvt_l_d(State* state, RvInstr* instr) {
  state->xregs[instr->rd] = (i64)llrint(state->fregs[instr->rs1].d);
}

static void handler_fcvt_lu_d(State* state, RvInstr* instr) {
  state->xregs[instr->rd] = (u64)llrint(state->fregs[instr->rs1].d);
}

static void handler_fcvt_d_w(State* state, RvInstr* instr) {
  state->fregs[instr->rd].d = (f64)(i32)state->xregs[instr->rs1];
}

static void handler_fcvt_d_wu(State* state, RvInstr* instr) {
  state->fregs[instr->rd].d = (f64)(u32)state->xregs[instr->rs1];
}

static void handler_fcvt_d_l(State* state, RvInstr* instr) {
  state->fregs[instr->rd].d = (f64)(i64)state->xregs[instr->rs1];
}

static void handler_fcvt_d_lu(State* state, RvInstr* instr) {
  state->fregs[instr->rd].d = (f64)(u64)state->xregs[instr->rs1];
}

static void handler_fmv_x_w(State* state, RvInstr* instr) {
  state->xregs[instr->rd] = (u64)(i64)(i32)state->fregs[instr->rs1].wu;
}

static void handler_fmv_w_x(State* state, RvInstr* instr) {
  state->fregs[instr->rd].wu = (u32)state->xregs[instr->rs1];
}

static void handler_fmv_x_d(State* state, RvInstr* instr) {
  state->xregs[instr->rd] = state->fregs[instr->rs1].lu;
}

static void handler_fmv_d_x(State* state, RvInstr* instr) {
  state->fregs[instr->rd].lu = state->xregs[instr->rs1];
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

  return (sign && inf_or_nan && frac_zero) << 0 |
         (sign && !inf_or_nan && !sub_or_zero) << 1 |
         (sign && sub_or_zero && !frac_zero) << 2 |
         (sign && sub_or_zero && frac_zero) << 3 |
         (!sign && inf_or_nan && frac_zero) << 7 |
         (!sign && !inf_or_nan && !sub_or_zero) << 6 |
         (!sign && sub_or_zero && !frac_zero) << 5 |
         (!sign && sub_or_zero && frac_zero) << 4 |
         (is_nan && is_signaling_nan) << 8 | (is_nan && !is_signaling_nan) << 9;
}

static void handler_fclass_s(State* state, RvInstr* instr) {
  state->xregs[instr->rd] = __classify_s(state->fregs[instr->rs1].s);
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

  return (sign && inf_or_nan && frac_zero) << 0 |
         (sign && !inf_or_nan && !sub_or_zero) << 1 |
         (sign && sub_or_zero && !frac_zero) << 2 |
         (sign && sub_or_zero && frac_zero) << 3 |
         (!sign && inf_or_nan && frac_zero) << 7 |
         (!sign && !inf_or_nan && !sub_or_zero) << 6 |
         (!sign && sub_or_zero && !frac_zero) << 5 |
         (!sign && sub_or_zero && frac_zero) << 4 |
         (is_nan && is_signaling_nan) << 8 | (is_nan && !is_signaling_nan) << 9;
}

static void handler_fclass_d(State* state, RvInstr* instr) {
  state->xregs[instr->rd] = __classify_d(state->fregs[instr->rs1].d);
}

static void handler_ni(State* state, RvInstr* instr) {}

static void (*rv_instr_handler[RV_INSTR_NUM])(State*, RvInstr*) = {
    // unprivileged: rv32i
    [U_RV32I_LUI] = handler_lui,
    [U_RV32I_AUIPC] = handler_auipc,
    [U_RV32I_JAL] = handler_jal,
    [U_RV32I_JALR] = handler_jalr,
    [U_RV32I_BEQ] = handler_beq,
    [U_RV32I_BNE] = handler_bne,
    [U_RV32I_BLT] = handler_blt,
    [U_RV32I_BGE] = handler_bge,
    [U_RV32I_BLTU] = handler_bltu,
    [U_RV32I_BGEU] = handler_bgeu,
    [U_RV32I_LB] = handler_lb,
    [U_RV32I_LH] = handler_lh,
    [U_RV32I_LW] = handler_lw,
    [U_RV32I_LBU] = handler_lbu,
    [U_RV32I_LHU] = handler_lhu,
    [U_RV32I_SB] = handler_sb,
    [U_RV32I_SH] = handler_sh,
    [U_RV32I_SW] = handler_sw,
    [U_RV32I_ADDI] = handler_addi,
    [U_RV32I_SLTI] = handler_slti,
    [U_RV32I_SLTIU] = handler_sltiu,
    [U_RV32I_XORI] = handler_xori,
    [U_RV32I_ORI] = handler_ori,
    [U_RV32I_ANDI] = handler_andi,
    [U_RV32I_SLLI] = handler_slli,
    [U_RV32I_SRLI] = handler_srli,
    [U_RV32I_SRAI] = handler_srai,
    [U_RV32I_ADD] = handler_add,
    [U_RV32I_SUB] = handler_sub,
    [U_RV32I_SLL] = handler_sll,
    [U_RV32I_SLT] = handler_slt,
    [U_RV32I_SLTU] = handler_sltu,
    [U_RV32I_XOR] = handler_xor,
    [U_RV32I_SRL] = handler_srl,
    [U_RV32I_SRA] = handler_sra,
    [U_RV32I_OR] = handler_or,
    [U_RV32I_AND] = handler_and,
    [U_RV32I_FENCE] = handler_ni,
    [U_RV32I_ECALL] = handler_ecall,
    [U_RV32I_EBREAK] = handler_ni,
    // unprivileged: rv64i
    [U_RV64I_LWU] = handler_lwu,
    [U_RV64I_LD] = handler_ld,
    [U_RV64I_SD] = handler_sd,
    [U_RV64I_SLLI] = handler_slli,
    [U_RV64I_SRLI] = handler_srli,
    [U_RV64I_SRAI] = handler_srai,
    [U_RV64I_ADDIW] = handler_addiw,
    [U_RV64I_SLLIW] = handler_slliw,
    [U_RV64I_SRLIW] = handler_srliw,
    [U_RV64I_SRAIW] = handler_sraiw,
    [U_RV64I_ADDW] = handler_addw,
    [U_RV64I_SUBW] = handler_subw,
    [U_RV64I_SLLW] = handler_sllw,
    [U_RV64I_SRLW] = handler_srlw,
    [U_RV64I_SRAW] = handler_sraw,
    // unprivileged: zifencei
    [U_ZIFENCEI_FENCE_I] = handler_ni,
    // unprivileged: zicsr
    [U_ZICSR_CSRRW] = handler_csrrw,
    [U_ZICSR_CSRRS] = handler_csrrs,
    [U_ZICSR_CSRRC] = handler_csrrc,
    [U_ZICSR_CSRRWI] = handler_csrrwi,
    [U_ZICSR_CSRRSI] = handler_csrrsi,
    [U_ZICSR_CSRRCI] = handler_csrrci,
    // unprivileged: rv32m
    [U_RV32M_MUL] = handler_mul,
    [U_RV32M_MULH] = handler_mulh,
    [U_RV32M_MULHSU] = handler_mulhsu,
    [U_RV32M_MULHU] = handler_mulhu,
    [U_RV32M_DIV] = handler_div,
    [U_RV32M_DIVU] = handler_divu,
    [U_RV32M_REM] = handler_rem,
    [U_RV32M_REMU] = handler_remu,
    // unprivileged: rv64m
    [U_RV64M_MULW] = handler_mulw,
    [U_RV64M_DIVW] = handler_divw,
    [U_RV64M_DIVUW] = handler_divuw,
    [U_RV64M_REMW] = handler_remw,
    [U_RV64M_REMUW] = handler_remuw,
    // unprivileged: rv32a
    [U_RV32A_LR_W] = handler_ni,
    [U_RV32A_SC_W] = handler_ni,
    [U_RV32A_AMOSWAP_W] = handler_ni,
    [U_RV32A_AMOADD_W] = handler_ni,
    [U_RV32A_AMOXOR_W] = handler_ni,
    [U_RV32A_AMOAND_W] = handler_ni,
    [U_RV32A_AMOOR_W] = handler_ni,
    [U_RV32A_AMOMIN_W] = handler_ni,
    [U_RV32A_AMOMAX_W] = handler_ni,
    [U_RV32A_AMOMINU_W] = handler_ni,
    [U_RV32A_AMOMAXU_W] = handler_ni,
    // unprivileged: rv64a
    [U_RV64A_LR_D] = handler_ni,
    [U_RV64A_SC_D] = handler_ni,
    [U_RV64A_AMOSWAP_D] = handler_ni,
    [U_RV64A_AMOADD_D] = handler_ni,
    [U_RV64A_AMOXOR_D] = handler_ni,
    [U_RV64A_AMOAND_D] = handler_ni,
    [U_RV64A_AMOOR_D] = handler_ni,
    [U_RV64A_AMOMIN_D] = handler_ni,
    [U_RV64A_AMOMAX_D] = handler_ni,
    [U_RV64A_AMOMINU_D] = handler_ni,
    [U_RV64A_AMOMAXU_D] = handler_ni,
    // unprivileged: rv32f
    [U_RV32F_FLW] = handler_flw,
    [U_RV32F_FSW] = handler_fsw,
    [U_RV32F_FMADD_S] = handler_fmadd_s,
    [U_RV32F_FMSUB_S] = handler_fmsub_s,
    [U_RV32F_FNMSUB_S] = handler_fnmsub_s,
    [U_RV32F_FNMADD_S] = handler_fnmadd_s,
    [U_RV32F_FADD_S] = handler_fadd_s,
    [U_RV32F_FSUB_S] = handler_fsub_s,
    [U_RV32F_FMUL_S] = handler_fmul_s,
    [U_RV32F_FDIV_S] = handler_fdiv_s,
    [U_RV32F_FSQRT_S] = handler_fsqrt_s,
    [U_RV32F_FSGNJ_S] = handler_fsgnj_s,
    [U_RV32F_FSGNJN_S] = handler_fsgnjn_s,
    [U_RV32F_FSGNJX_S] = handler_fsgnjx_s,
    [U_RV32F_FMIN_S] = handler_fmin_s,
    [U_RV32F_FMAX_S] = handler_fmax_s,
    [U_RV32F_FCVT_W_S] = handler_fcvt_w_s,
    [U_RV32F_FCVT_WU_S] = handler_fcvt_wu_s,
    [U_RV32F_FMV_X_W] = handler_fmv_x_w,
    [U_RV32F_FEQ_S] = handler_feq_s,
    [U_RV32F_FLT_S] = handler_flt_s,
    [U_RV32F_FLE_S] = handler_fle_s,
    [U_RV32F_FCLASS_S] = handler_fclass_s,
    [U_RV32F_FCVT_S_W] = handler_fcvt_s_w,
    [U_RV32F_FCVT_S_WU] = handler_fcvt_s_wu,
    [U_RV32F_FMV_W_X] = handler_fmv_w_x,
    // unprivileged: rv64f
    [U_RV64F_FCVT_L_S] = handler_fcvt_l_s,
    [U_RV64F_FCVT_LU_S] = handler_fcvt_lu_s,
    [U_RV64F_FCVT_S_L] = handler_fcvt_s_l,
    [U_RV64F_FCVT_S_LU] = handler_fcvt_s_lu,
    // unprivileged: rv32d
    [U_RV32D_FLD] = handler_fld,
    [U_RV32D_FSD] = handler_fsd,
    [U_RV32D_FMADD_D] = handler_fmadd_d,
    [U_RV32D_FMSUB_D] = handler_fmsub_d,
    [U_RV32D_FNMSUB_D] = handler_fnmsub_d,
    [U_RV32D_FNMADD_D] = handler_fnmadd_d,
    [U_RV32D_FADD_D] = handler_fadd_d,
    [U_RV32D_FSUB_D] = handler_fsub_d,
    [U_RV32D_FMUL_D] = handler_fmul_d,
    [U_RV32D_FDIV_D] = handler_fdiv_d,
    [U_RV32D_FSQRT_D] = handler_fsqrt_d,
    [U_RV32D_FSGNJ_D] = handler_fsgnj_d,
    [U_RV32D_FSGNJN_D] = handler_fsgnjn_d,
    [U_RV32D_FSGNJX_D] = handler_fsgnjx_d,
    [U_RV32D_FMIN_D] = handler_fmin_d,
    [U_RV32D_FMAX_D] = handler_fmax_d,
    [U_RV32D_FCVT_S_D] = handler_fcvt_s_d,
    [U_RV32D_FCVT_D_S] = handler_fcvt_d_s,
    [U_RV32D_FEQ_D] = handler_feq_d,
    [U_RV32D_FLT_D] = handler_flt_d,
    [U_RV32D_FLE_D] = handler_fle_d,
    [U_RV32D_FCLASS_D] = handler_fclass_d,
    [U_RV32D_FCVT_W_D] = handler_fcvt_w_d,
    [U_RV32D_FCVT_WU_D] = handler_fcvt_wu_d,
    [U_RV32D_FCVT_D_W] = handler_fcvt_d_w,
    [U_RV32D_FCVT_D_WU] = handler_fcvt_d_wu,
    // unprivileged: rv64d
    [U_RV64D_FCVT_L_D] = handler_fcvt_l_d,
    [U_RV64D_FCVT_LU_D] = handler_fcvt_lu_d,
    [U_RV64D_FMV_X_D] = handler_fmv_x_d,
    [U_RV64D_FCVT_D_L] = handler_fcvt_d_l,
    [U_RV64D_FCVT_D_LU] = handler_fcvt_d_lu,
    [U_RV64D_FMV_D_X] = handler_fmv_d_x,
    // privileged: trap-return
    [P_SRET] = handler_ni,
    [P_MRET] = handler_ni,
    // privileged: interrupt-management
    [P_WFI] = handler_ni,
    // privileged: supervisor memory-management
    [P_SFENCE_VMA] = handler_ni,
    [P_SINVAL_VMA] = handler_ni,
    [P_SFENCE_W_INVAL] = handler_ni,
    [P_SFENCE_INVAL_IR] = handler_ni,
};

void exec_block_interp(State* state) {
  static RvInstr instr = {0};
  while (true) {
    u32 instr_raw = *(u32*)TO_HOST(state->pc);
    rv_instr_decode(&instr, instr_raw);
    rv_instr_handler[instr.type](state, &instr);

    state->xregs[X_REG_ZERO] = 0;

    if (instr.cont) break;

    state->pc += instr.rvc ? 2 : 4;
  }
}
