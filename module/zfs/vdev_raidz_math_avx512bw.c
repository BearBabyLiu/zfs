/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright (C) 2016 Romain Dolbeau. All rights reserved.
 */

#include <sys/isa_defs.h>

#if 0 // defined(__x86_64) && defined(HAVE_AVX512BW)

#include <sys/types.h>
#include <linux/simd_x86.h>

#define	__asm __asm__ __volatile__

#define	_REG_CNT(_0, _1, _2, _3, _4, _5, _6, _7, N, ...) N
#define	REG_CNT(r...) _REG_CNT(r, 8, 7, 6, 5, 4, 3, 2, 1)

#define	VR0_(REG, ...) "zmm"#REG
#define	VR1_(_1, REG, ...) "zmm"#REG
#define	VR2_(_1, _2, REG, ...) "zmm"#REG
#define	VR3_(_1, _2, _3, REG, ...) "zmm"#REG
#define	VR4_(_1, _2, _3, _4, REG, ...) "zmm"#REG
#define	VR5_(_1, _2, _3, _4, _5, REG, ...) "zmm"#REG
#define	VR6_(_1, _2, _3, _4, _5, _6, REG, ...) "zmm"#REG
#define	VR7_(_1, _2, _3, _4, _5, _6, _7, REG, ...) "zmm"#REG

#define	VR0(r...) VR0_(r)
#define	VR1(r...) VR1_(r)
#define	VR2(r...) VR2_(r, 1)
#define	VR3(r...) VR3_(r, 1, 2)
#define	VR4(r...) VR4_(r, 1, 2)
#define	VR5(r...) VR5_(r, 1, 2, 3)
#define	VR6(r...) VR6_(r, 1, 2, 3, 4)
#define	VR7(r...) VR7_(r, 1, 2, 3, 4, 5)

#define	R_01(REG1, REG2, ...) REG1, REG2
#define	_R_23(_0, _1, REG2, REG3, ...) REG2, REG3
#define	R_23(REG...) _R_23(REG, 1, 2, 3)

#define	ASM_BUG()	ASSERT(0)

extern const uint8_t gf_clmul_mod_lt[4*256][16];

#define	ELEM_SIZE 64

typedef struct v {
	uint8_t b[ELEM_SIZE] __attribute__((aligned(ELEM_SIZE)));
} v_t;

#define	PREFETCHNTA(ptr, offset) 					\
{									\
	__asm(								\
	    "prefetchnta " #offset "(%[MEM])\n"				\
	    : : [MEM] "r" (ptr));					\
}

#define	PREFETCH(ptr, offset) 						\
{									\
	__asm(								\
	    "prefetcht0 " #offset "(%[MEM])\n"				\
	    : : [MEM] "r" (ptr));					\
}

#define	XOR_ACC(src, r...)						\
{									\
	switch (REG_CNT(r)) {						\
	case 4:								\
		__asm(							\
		    "vpxorq 0x00(%[SRC]), %%" VR0(r)", %%" VR0(r) "\n"	\
		    "vpxorq 0x40(%[SRC]), %%" VR1(r)", %%" VR1(r) "\n"	\
		    "vpxorq 0x80(%[SRC]), %%" VR2(r)", %%" VR2(r) "\n"	\
		    "vpxorq 0xc0(%[SRC]), %%" VR3(r)", %%" VR3(r) "\n"	\
		    : : [SRC] "r" (src));				\
		break;							\
	case 2:								\
		__asm(							\
		    "vpxorq 0x00(%[SRC]), %%" VR0(r)", %%" VR0(r) "\n"	\
		    "vpxorq 0x40(%[SRC]), %%" VR1(r)", %%" VR1(r) "\n"	\
		    : : [SRC] "r" (src));				\
		break;							\
	default:							\
		ASM_BUG();						\
	}								\
}

#define	XOR(r...)							\
{									\
	switch (REG_CNT(r)) {						\
	case 8:								\
		__asm(							\
		    "vpxorq %" VR0(r) ", %" VR4(r)", %" VR4(r) "\n"	\
		    "vpxorq %" VR1(r) ", %" VR5(r)", %" VR5(r) "\n"	\
		    "vpxorq %" VR2(r) ", %" VR6(r)", %" VR6(r) "\n"	\
		    "vpxorq %" VR3(r) ", %" VR7(r)", %" VR7(r));	\
		break;							\
	case 4:								\
		__asm(							\
		    "vpxorq %" VR0(r) ", %" VR2(r)", %" VR2(r) "\n"	\
		    "vpxorq %" VR1(r) ", %" VR3(r)", %" VR3(r));	\
		break;							\
	default:							\
		ASM_BUG();						\
	}								\
}

#define	ZERO(r...)							\
{									\
	switch (REG_CNT(r)) {						\
	case 4:								\
		__asm(							\
		    "vpxorq %" VR0(r) ", %" VR0(r)", %" VR0(r) "\n"	\
		    "vpxorq %" VR1(r) ", %" VR1(r)", %" VR1(r) "\n"	\
		    "vpxorq %" VR2(r) ", %" VR2(r)", %" VR2(r) "\n"	\
		    "vpxorq %" VR3(r) ", %" VR3(r)", %" VR3(r));	\
		break;							\
	case 2:								\
		__asm(							\
		    "vpxorq %" VR0(r) ", %" VR0(r)", %" VR0(r) "\n"	\
		    "vpxorq %" VR1(r) ", %" VR1(r)", %" VR1(r));	\
		break;							\
	default:							\
		ASM_BUG();						\
	}								\
}

#define	COPY(r...) 							\
{									\
	switch (REG_CNT(r)) {						\
	case 8:								\
		__asm(							\
		    "vmovdqa64 %" VR0(r) ", %" VR4(r) "\n"		\
		    "vmovdqa64 %" VR1(r) ", %" VR5(r) "\n"		\
		    "vmovdqa64 %" VR2(r) ", %" VR6(r) "\n"		\
		    "vmovdqa64 %" VR3(r) ", %" VR7(r));			\
		break;							\
	case 4:								\
		__asm(							\
		    "vmovdqa64 %" VR0(r) ", %" VR2(r) "\n"		\
		    "vmovdqa64 %" VR1(r) ", %" VR3(r));			\
		break;							\
	default:							\
		ASM_BUG();						\
	}								\
}

#define	LOAD(src, r...) 						\
{									\
	switch (REG_CNT(r)) {						\
	case 4:								\
		__asm(							\
		    "vmovdqa64 0x00(%[SRC]), %%" VR0(r) "\n"		\
		    "vmovdqa64 0x40(%[SRC]), %%" VR1(r) "\n"		\
		    "vmovdqa64 0x80(%[SRC]), %%" VR2(r) "\n"		\
		    "vmovdqa64 0xc0(%[SRC]), %%" VR3(r) "\n"		\
		    : : [SRC] "r" (src));				\
		break;							\
	case 2:								\
		__asm(							\
		    "vmovdqa64 0x00(%[SRC]), %%" VR0(r) "\n"		\
		    "vmovdqa64 0x40(%[SRC]), %%" VR1(r) "\n"		\
		    : : [SRC] "r" (src));				\
		break;							\
	default:							\
		ASM_BUG();						\
	}								\
}

#define	STORE(dst, r...)   						\
{									\
	switch (REG_CNT(r)) {						\
	case 4:								\
		__asm(							\
		    "vmovdqa64 %%" VR0(r) ", 0x00(%[DST])\n"		\
		    "vmovdqa64 %%" VR1(r) ", 0x40(%[DST])\n"		\
		    "vmovdqa64 %%" VR2(r) ", 0x80(%[DST])\n"		\
		    "vmovdqa64 %%" VR3(r) ", 0xc0(%[DST])\n"		\
		    : : [DST] "r" (dst));				\
		break;							\
	case 2:								\
		__asm(							\
		    "vmovdqa64 %%" VR0(r) ", 0x00(%[DST])\n"		\
		    "vmovdqa64 %%" VR1(r) ", 0x40(%[DST])\n"		\
		    : : [DST] "r" (dst));				\
		break;							\
	default:							\
		ASM_BUG();						\
	}								\
}

#define	FLUSH()								\
{									\
	__asm("vzeroupper");						\
}

#define	MUL2_SETUP() 							\
{   									\
	__asm("vmovq %0,   %%xmm14" :: "r"(0x1d1d1d1d1d1d1d1d));	\
	__asm("vpbroadcastq %xmm14, %zmm14");				\
	__asm("vmovq %0,   %%xmm13" :: "r"(0x8080808080808080));	\
	__asm("vpbroadcastq %xmm13, %zmm13");				\
	__asm("vmovq %0,   %%xmm12" :: "r"(0xfefefefefefefefe));	\
	__asm("vpbroadcastq %xmm12, %zmm12");				\
	__asm("vpxorq       %zmm15, %zmm15 ,%zmm15");			\
}

#define	_MUL2(r...) 							\
{									\
	switch	(REG_CNT(r)) {						\
	case 2:								\
		__asm(							\
		    "vpandq   %" VR0(r)", %zmm13, %zmm10\n"		\
		    "vpandq   %" VR1(r)", %zmm13, %zmm11\n"		\
		    "vpsrlq   $7, %zmm10, %zmm8\n"			\
		    "vpsrlq   $7, %zmm11, %zmm9\n"			\
		    "vpsllq   $1, %zmm10, %zmm10\n"			\
		    "vpsllq   $1, %zmm11, %zmm11\n"			\
		    "vpsubq   %zmm8, %zmm10, %zmm10\n"			\
		    "vpsubq   %zmm9, %zmm11, %zmm11\n"			\
		    "vpsllq   $1, %" VR0(r)", %" VR0(r) "\n"		\
		    "vpsllq   $1, %" VR1(r)", %" VR1(r) "\n"		\
		    "vpandq   %zmm10, %zmm14, %zmm10\n" 		\
		    "vpandq   %zmm11, %zmm14, %zmm11\n" 		\
		    "vpternlogd $0x6c,%zmm12, %zmm10, %" VR0(r) "\n"	\
		    "vpternlogd $0x6c,%zmm12, %zmm11, %" VR1(r));	\
		break;							\
	default:							\
		ASM_BUG();						\
	}								\
}

#define	MUL2(r...)							\
{									\
	switch (REG_CNT(r)) {						\
	case 4:								\
	    _MUL2(R_01(r));						\
	    _MUL2(R_23(r));						\
	    break;							\
	case 2:								\
	    _MUL2(r);							\
	    break;							\
	default:							\
		ASM_BUG();						\
	}								\
}

#define	MUL4(r...)							\
{									\
	MUL2(r);							\
	MUL2(r);							\
}

#define	_0f		"zmm15"
#define	_as		"zmm14"
#define	_bs		"zmm13"
#define	_ltmod		"zmm12"
#define	_ltmul		"zmm11"
#define	_ta		"zmm10"
#define	_tb		"zmm15"

static const uint8_t __attribute__((aligned(32))) _mul_mask = 0x0F;

#define	_MULx2(c, r...)							\
{									\
	switch (REG_CNT(r)) {						\
	case 2:								\
		__asm(							\
		    "vpbroadcastb (%[mask]), %%" _0f "\n"		\
		    /* upper bits */					\
		    "vbroadcasti32x4 0x00(%[lt]), %%" _ltmod "\n"	\
		    "vbroadcasti32x4 0x10(%[lt]), %%" _ltmul "\n"	\
									\
		    "vpsraw $0x4, %%" VR0(r) ", %%"_as "\n"		\
		    "vpsraw $0x4, %%" VR1(r) ", %%"_bs "\n"		\
		    "vpandq %%" _0f ", %%" VR0(r) ", %%" VR0(r) "\n"	\
		    "vpandq %%" _0f ", %%" VR1(r) ", %%" VR1(r) "\n"	\
		    "vpandq %%" _0f ", %%" _as ", %%" _as "\n"		\
		    "vpandq %%" _0f ", %%" _bs ", %%" _bs "\n"		\
									\
		    "vpshufb %%" _as ", %%" _ltmod ", %%" _ta "\n"	\
		    "vpshufb %%" _bs ", %%" _ltmod ", %%" _tb "\n"	\
		    "vpshufb %%" _as ", %%" _ltmul ", %%" _as "\n"	\
		    "vpshufb %%" _bs ", %%" _ltmul ", %%" _bs "\n"	\
		    /* lower bits */					\
		    "vbroadcasti32x4 0x20(%[lt]), %%" _ltmod "\n"	\
		    "vbroadcasti32x4 0x30(%[lt]), %%" _ltmul "\n"	\
									\
		    "vpxorq %%" _ta ", %%" _as ", %%" _as "\n"		\
		    "vpxorq %%" _tb ", %%" _bs ", %%" _bs "\n"		\
									\
		    "vpshufb %%" VR0(r) ", %%" _ltmod ", %%" _ta "\n"	\
		    "vpshufb %%" VR1(r) ", %%" _ltmod ", %%" _tb "\n"	\
		    "vpshufb %%" VR0(r) ", %%" _ltmul ", %%" VR0(r) "\n"\
		    "vpshufb %%" VR1(r) ", %%" _ltmul ", %%" VR1(r) "\n"\
									\
		    "vpxorq %%" _ta ", %%" VR0(r) ", %%" VR0(r) "\n"	\
		    "vpxorq %%" _as ", %%" VR0(r) ", %%" VR0(r) "\n"	\
		    "vpxorq %%" _tb ", %%" VR1(r) ", %%" VR1(r) "\n"	\
		    "vpxorq %%" _bs ", %%" VR1(r) ", %%" VR1(r) "\n"	\
		    : : [mask] "r" (&_mul_mask),			\
		    [lt] "r" (gf_clmul_mod_lt[4*(c)]));			\
		break;							\
	default:							\
		ASM_BUG();						\
	}								\
}

#define	MUL(c, r...)							\
{									\
	switch (REG_CNT(r)) {						\
	case 4:								\
		_MULx2(c, R_01(r));					\
		_MULx2(c, R_23(r));					\
		break;							\
	case 2:								\
		_MULx2(c, R_01(r));					\
		break;							\
	default:							\
		ASM_BUG();						\
	}								\
}

#define	raidz_math_begin()	kfpu_begin()
#define	raidz_math_end()						\
{									\
	FLUSH();							\
	kfpu_end();							\
}

#define	ZERO_STRIDE		4
#define	ZERO_DEFINE()		{}
#define	ZERO_D			0, 1, 2, 3

#define	COPY_STRIDE		4
#define	COPY_DEFINE()		{}
#define	COPY_D			0, 1, 2, 3

#define	ADD_STRIDE		4
#define	ADD_DEFINE()		{}
#define	ADD_D 			0, 1, 2, 3

#define	MUL_STRIDE		4
#define	MUL_DEFINE() 		{}
#define	MUL_D			0, 1, 2, 3

#define	GEN_P_DEFINE()		{}
#define	GEN_P_STRIDE		4
#define	GEN_P_P			0, 1, 2, 3

#define	GEN_PQ_DEFINE() 	{}
#define	GEN_PQ_STRIDE		4
#define	GEN_PQ_D		0, 1, 2, 3
#define	GEN_PQ_P		4, 5, 6, 7
#define	GEN_PQ_Q		20, 21, 22, 23

#define	GEN_PQR_DEFINE() 	{}
#define	GEN_PQR_STRIDE		2
#define	GEN_PQR_D		0, 1
#define	GEN_PQR_P		2, 3
#define	GEN_PQR_Q		4, 5
#define	GEN_PQR_R		6, 7

#define	REC_P_DEFINE() 		{}
#define	REC_P_STRIDE		4
#define	REC_P_X			0, 1, 2, 3

#define	REC_Q_DEFINE() 		{}
#define	REC_Q_STRIDE		4
#define	REC_Q_X			0, 1, 2, 3

#define	REC_R_DEFINE() 		{}
#define	REC_R_STRIDE		4
#define	REC_R_X			0, 1, 2, 3

#define	REC_PQ_DEFINE() 	{}
#define	REC_PQ_STRIDE		4
#define	REC_PQ_X		0, 1, 2, 3
#define	REC_PQ_Y		4, 5, 6, 7
#define	REC_PQ_D		20, 21, 22, 23

#define	REC_PR_DEFINE() 	{}
#define	REC_PR_STRIDE		4
#define	REC_PR_X		0, 1, 2, 3
#define	REC_PR_Y		4, 5, 6, 7
#define	REC_PR_D		20, 21, 22, 23

#define	REC_QR_DEFINE() 	{}
#define	REC_QR_STRIDE		4
#define	REC_QR_X		0, 1, 2, 3
#define	REC_QR_Y		4, 5, 6, 7
#define	REC_QR_D		20, 21, 22, 23

#define	REC_PQR_DEFINE() 	{}
#define	REC_PQR_STRIDE		2
#define	REC_PQR_X		0, 1
#define	REC_PQR_Y		2, 3
#define	REC_PQR_Z		4, 5
#define	REC_PQR_D		6, 7
#define	REC_PQR_XS		6, 7
#define	REC_PQR_YS		8, 9


#include <sys/vdev_raidz_impl.h>
#include "vdev_raidz_math_impl.h"

DEFINE_GEN_METHODS(avx512bw);
DEFINE_REC_METHODS(avx512bw);

static boolean_t
raidz_will_avx512bw_work(void)
{
	return (zfs_avx_available() &&
		zfs_avx512f_available() &&
		zfs_avx512bw_available());
}

const raidz_impl_ops_t vdev_raidz_avx512bw_impl = {
	.init = NULL,
	.fini = NULL,
	.gen = RAIDZ_GEN_METHODS(avx512bw),
	.rec = RAIDZ_REC_METHODS(avx512bw),
	.is_supported = &raidz_will_avx512bw_work,
	.name = "avx512bw"
};

#endif /* defined(__x86_64) && defined(HAVE_AVX512BW) */
