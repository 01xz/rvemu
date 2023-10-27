#ifndef RVEMU_INSTR_H_
#define RVEMU_INSTR_H_

// clang-format off
#define RV32I_INSTRS(_)                                                   \
  _(LUI)                                                                  \
  _(AUIPC)                                                                \
  _(JAL)                                                                  \
  _(JALR)                                                                 \
  _(BEQ) _(BNE) _(BLT) _(BGE) _(BLTU) _(BGEU)                             \
  _(LB) _(LH) _(LW) _(LBU) _(LHU)                                         \
  _(SB) _(SH) _(SW)                                                       \
  _(ADDI) _(SLTI) _(SLTIU) _(XORI) _(ORI) _(ANDI) _(SLLI) _(SRLI) _(SRAI) \
  _(ADD) _(SUB) _(SLL) _(SLT) _(SLTU) _(XOR) _(SRL) _(SRA) _(OR) _(AND)   \
  _(FENCE)                                                                \
  _(ECALL) _(EBREAK)
// clang-format on

#define RV32I_NAMES(s) U_RV32I_##s,

// clang-format off
#define RV64I_INSTRS(_)               \
  _(LWU) _(LD)                        \
  _(SD)                               \
  _(SLLI) _(SRLI) _(SRAI)             \
  _(ADDIW) _(SLLIW) _(SRLIW) _(SRAIW) \
  _(ADDW) _(SUBW) _(SLLW) _(SRLW) _(SRAW)
// clang-format on

#define RV64I_NAMES(s) U_RV64I_##s,

// clang-format off
#define ZIFENCEI_INSTRS(_) \
  _(FENCE_I)
// clang-format on

#define ZIFENCEI_NAMES(s) U_ZIFENCEI_##s,

// clang-format off
#define ZICSR_INSTRS(_) \
  _(CSRRW) _(CSRRS) _(CSRRC) _(CSRRWI) _(CSRRSI) _(CSRRCI)
// clang-format on

#define ZICSR_NAMES(s) U_ZICSR_##s,

// clang-format off
#define RV32M_INSTRS(_) \
  _(MUL) _(MULH) _(MULHSU) _(MULHU) _(DIV) _(DIVU) _(REM) _(REMU)
// clang-format on

#define RV32M_NAMES(s) U_RV32M_##s,

// clang-format off
#define RV64M_INSTRS(_) \
  _(MULW) _(DIVW) _(DIVUW) _(REMW) _(REMUW)
// clang-format on

#define RV64M_NAMES(s) U_RV64M_##s,

// clang-format off
#define RV32A_INSTRS(_)        \
  _(LR)                        \
  _(SC)                        \
  _(AMOSWAP)                   \
  _(AMOADD)                    \
  _(AMOXOR) _(AMOAND) _(AMOOR) \
  _(AMOMIN) _(AMOMAX) _(AMOMINU) _(AMOMAXU)
// clang-format on

#define RV32A_NAMES(s) U_RV32A_##s##_W,

#define RV64A_INSTRS RV32A_INSTRS

#define RV64A_NAMES(s) U_RV64A_##s##_D,

// clang-format off
#define RV32F_INSTRS(_)                         \
  _(FLW)                                        \
  _(FSW)                                        \
  _(FMADD_S) _(FMSUB_S) _(FNMSUB_S) _(FNMADD_S) \
  _(FADD_S) _(FSUB_S) _(FMUL_S) _(FDIV_S)       \
  _(FSQRT_S)                                    \
  _(FSGNJ_S) _(FSGNJN_S) _(FSGNJX_S)            \
  _(FMIN_S) _(FMAX_S)                           \
  _(FCVT_W_S) _(FCVT_WU_S)                      \
  _(FMV_X_W)                                    \
  _(FEQ_S) _(FLT_S) _(FLE_S)                    \
  _(FCLASS_S)                                   \
  _(FCVT_S_W) _(FCVT_S_WU)                      \
  _(FMV_W_X)
// clang-format on

#define RV32F_NAMES(s) U_RV32F_##s,

// clang-format off
#define RV64F_INSTRS(_) \
  _(FCVT_L_S) _(FCVT_LU_S) _(FCVT_S_L) _(FCVT_S_LU)
// clang-format on

#define RV64F_NAMES(s) U_RV64F_##s,

// clang-format off
#define RV32D_INSTRS(_)                         \
  _(FLD)                                        \
  _(FSD)                                        \
  _(FMADD_D) _(FMSUB_D) _(FNMSUB_D) _(FNMADD_D) \
  _(FADD_D) _(FSUB_D) _(FMUL_D) _(FDIV_D)       \
  _(FSQRT_D)                                    \
  _(FSGNJ_D) _(FSGNJN_D) _(FSGNJX_D)            \
  _(FMIN_D) _(FMAX_D)                           \
  _(FCVT_S_D) _(FCVT_D_S)                       \
  _(FEQ_D) _(FLT_D) _(FLE_D)                    \
  _(FCLASS_D)                                   \
  _(FCVT_W_D) _(FCVT_WU_D) _(FCVT_D_W) _(FCVT_D_WU)
// clang-format on

#define RV32D_NAMES(s) U_RV32D_##s,

// clang-format off
#define RV64D_INSTRS(_) \
  _(FCVT_L_D) _(FCVT_LU_D) _(FMV_X_D) _(FCVT_D_L) _(FCVT_D_LU) _(FMV_D_X)
// clang-format on

#define RV64D_NAMES(s) U_RV64D_##s,

// clang-format off
#define PRIVILEGED_INSTRS(_) \
  _(SRET) _(MRET)            \
  _(WFI)                     \
  _(SFENCE_VMA) _(SINVAL_VMA) _(SFENCE_W_INVAL) _(SFENCE_INVAL_IR)
// clang-format on

#define PRIVILEGED_NAMES(s) P_##s,

#define EXTENSION(s) s##_INSTRS(s##_NAMES)

#define RVEMU_ISA(...)        \
  typedef enum {              \
    __VA_ARGS__ RV_INSTR_NUM, \
  } RvInstrType;

// clang-format off
RVEMU_ISA(              \
  EXTENSION(RV32I)      \
  EXTENSION(RV64I)      \
  EXTENSION(ZIFENCEI)   \
  EXTENSION(ZICSR)      \
  EXTENSION(RV32M)      \
  EXTENSION(RV64M)      \
  EXTENSION(RV32A)      \
  EXTENSION(RV64A)      \
  EXTENSION(RV32F)      \
  EXTENSION(RV64F)      \
  EXTENSION(RV32D)      \
  EXTENSION(RV64D)      \
  EXTENSION(PRIVILEGED) \
);
// clang-format on

#endif  // RVEMU_INSTR_H_
