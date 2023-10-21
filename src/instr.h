#ifndef RVEMU_INSTR_H_
#define RVEMU_INSTR_H_

typedef enum {
  // unprivileged: rv32i
  U_RV32I_LUI,
  U_RV32I_AUIPC,
  U_RV32I_JAL,
  U_RV32I_JALR,
  U_RV32I_BEQ,
  U_RV32I_BNE,
  U_RV32I_BLT,
  U_RV32I_BGE,
  U_RV32I_BLTU,
  U_RV32I_BGEU,
  U_RV32I_LB,
  U_RV32I_LH,
  U_RV32I_LW,
  U_RV32I_LBU,
  U_RV32I_LHU,
  U_RV32I_SB,
  U_RV32I_SH,
  U_RV32I_SW,
  U_RV32I_ADDI,
  U_RV32I_SLTI,
  U_RV32I_SLTIU,
  U_RV32I_XORI,
  U_RV32I_ORI,
  U_RV32I_ANDI,
  U_RV32I_SLLI,
  U_RV32I_SRLI,
  U_RV32I_SRAI,
  U_RV32I_ADD,
  U_RV32I_SUB,
  U_RV32I_SLL,
  U_RV32I_SLT,
  U_RV32I_SLTU,
  U_RV32I_XOR,
  U_RV32I_SRL,
  U_RV32I_SRA,
  U_RV32I_OR,
  U_RV32I_AND,
  U_RV32I_FENCE,
  U_RV32I_ECALL,
  U_RV32I_EBREAK,
  // unprivileged: rv64i
  U_RV64I_LWU,
  U_RV64I_LD,
  U_RV64I_SD,
  U_RV64I_SLLI,
  U_RV64I_SRLI,
  U_RV64I_SRAI,
  U_RV64I_ADDIW,
  U_RV64I_SLLIW,
  U_RV64I_SRLIW,
  U_RV64I_SRAIW,
  U_RV64I_ADDW,
  U_RV64I_SUBW,
  U_RV64I_SLLW,
  U_RV64I_SRLW,
  U_RV64I_SRAW,
  // unprivileged: zifencei
  U_ZIFENCEI_FENCE_I,
  // unprivileged: zicsr
  U_ZICSR_CSRRW,
  U_ZICSR_CSRRS,
  U_ZICSR_CSRRC,
  U_ZICSR_CSRRWI,
  U_ZICSR_CSRRSI,
  U_ZICSR_CSRRCI,
  // unprivileged: rv32m
  U_RV32M_MUL,
  U_RV32M_MULH,
  U_RV32M_MULHSU,
  U_RV32M_MULHU,
  U_RV32M_DIV,
  U_RV32M_DIVU,
  U_RV32M_REM,
  U_RV32M_REMU,
  // unprivileged: rv64m
  U_RV64M_MULW,
  U_RV64M_DIVW,
  U_RV64M_DIVUW,
  U_RV64M_REMW,
  U_RV64M_REMUW,
  // unprivileged: rv32a
  U_RV32A_LR_W,
  U_RV32A_SC_W,
  U_RV32A_AMOSWAP_W,
  U_RV32A_AMOADD_W,
  U_RV32A_AMOXOR_W,
  U_RV32A_AMOAND_W,
  U_RV32A_AMOOR_W,
  U_RV32A_AMOMIN_W,
  U_RV32A_AMOMAX_W,
  U_RV32A_AMOMINU_W,
  U_RV32A_AMOMAXU_W,
  // unprivileged: rv64a
  U_RV64A_LR_D,
  U_RV64A_SC_D,
  U_RV64A_AMOSWAP_D,
  U_RV64A_AMOADD_D,
  U_RV64A_AMOXOR_D,
  U_RV64A_AMOAND_D,
  U_RV64A_AMOOR_D,
  U_RV64A_AMOMIN_D,
  U_RV64A_AMOMAX_D,
  U_RV64A_AMOMINU_D,
  U_RV64A_AMOMAXU_D,
  // unprivileged: rv32f
  U_RV32F_FLW,
  U_RV32F_FSW,
  U_RV32F_FMADD_S,
  U_RV32F_FMSUB_S,
  U_RV32F_FNMSUB_S,
  U_RV32F_FNMADD_S,
  U_RV32F_FADD_S,
  U_RV32F_FSUB_S,
  U_RV32F_FMUL_S,
  U_RV32F_FDIV_S,
  U_RV32F_FSQRT_S,
  U_RV32F_FSGNJ_S,
  U_RV32F_FSGNJN_S,
  U_RV32F_FSGNJX_S,
  U_RV32F_FMIN_S,
  U_RV32F_FMAX_S,
  U_RV32F_FCVT_W_S,
  U_RV32F_FCVT_WU_S,
  U_RV32F_FMV_X_W,
  U_RV32F_FEQ_S,
  U_RV32F_FLT_S,
  U_RV32F_FLE_S,
  U_RV32F_FCLASS_S,
  U_RV32F_FCVT_S_W,
  U_RV32F_FCVT_S_WU,
  U_RV32F_FMV_W_X,
  // unprivileged: rv64f
  U_RV64F_FCVT_L_S,
  U_RV64F_FCVT_LU_S,
  U_RV64F_FCVT_S_L,
  U_RV64F_FCVT_S_LU,
  // unprivileged: rv32d
  U_RV32D_FLD,
  U_RV32D_FSD,
  U_RV32D_FMADD_D,
  U_RV32D_FMSUB_D,
  U_RV32D_FNMSUB_D,
  U_RV32D_FNMADD_D,
  U_RV32D_FADD_D,
  U_RV32D_FSUB_D,
  U_RV32D_FMUL_D,
  U_RV32D_FDIV_D,
  U_RV32D_FSQRT_D,
  U_RV32D_FSGNJ_D,
  U_RV32D_FSGNJN_D,
  U_RV32D_FSGNJX_D,
  U_RV32D_FMIN_D,
  U_RV32D_FMAX_D,
  U_RV32D_FCVT_S_D,
  U_RV32D_FCVT_D_S,
  U_RV32D_FEQ_D,
  U_RV32D_FLT_D,
  U_RV32D_FLE_D,
  U_RV32D_FCLASS_D,
  U_RV32D_FCVT_W_D,
  U_RV32D_FCVT_WU_D,
  U_RV32D_FCVT_D_W,
  U_RV32D_FCVT_D_WU,
  // unprivileged: rv64d
  U_RV64D_FCVT_L_D,
  U_RV64D_FCVT_LU_D,
  U_RV64D_FMV_X_D,
  U_RV64D_FCVT_D_L,
  U_RV64D_FCVT_D_LU,
  U_RV64D_FMV_D_X,
  // privileged: trap-return
  P_SRET,
  P_MRET,
  // privileged: interrupt-management
  P_WFI,
  // privileged: supervisor memory-management
  P_SFENCE_VMA,
  P_SINVAL_VMA,
  P_SFENCE_W_INVAL,
  P_SFENCE_INVAL_IR,
  // total numbers
  RV_INSTR_NUM,
} RvInstrType;

#endif  // RVEMU_INSTR_H_
