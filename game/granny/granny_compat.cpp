/*  granny_compat.cpp - portable reimplementation of the RAD Granny 1.x ("1.2b", 2000) functions used by
    Jimmy Neutron vs. Jimmy Negatron.  Reference: granny.dll's machine code; see docs/granny-replacement.md.

    Arithmetic: granny.dll runs the x87 in 53-bit mode (its CRT sets control word 0x027f), so its
    expressions behave like C double arithmetic, rounded to float wherever the code stores to a float.
    Every routine below follows the order of operations of the corresponding granny.dll function (its
    address is noted) with double temporaries and explicit float stores, which makes the results
    bit-identical (tools/granny/verify.sh).  This file must be compiled without FMA contraction
    (-ffp-contract=off) and with IEEE double (SSE2 / ARM64; not x87).                                  */
#include "granny_compat.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>

#if defined(__clang__)
#pragma STDC FP_CONTRACT OFF
#endif

typedef uint32_t u32;
typedef uint16_t u16;
typedef double D;

namespace {

/* --------------------------------------------------------------------------------------- math --- */

struct Local            /* granny's decomposed transform (16 floats, bone + 0xac) */
{
	float pos[3];
	float q[4];         /* x y z w */
	float ss[9];        /* scale/shear, row-major */
};

struct World            /* granny's composed transform (16 floats, bone + 0xec) */
{
	float q[4];
	float t[3];
	float M[9];         /* = R(q) * scale/shear, row-major, column vectors */
};

static void local_identity(Local &l)
{
	memset(&l, 0, sizeof l);
	l.q[3] = 1.0f;
	l.ss[0] = l.ss[4] = l.ss[8] = 1.0f;
}

static inline float F(D v) { return (float)v; }

/* the normalisation used after sampling, in 0x100220b0 and at the end of 0x10021910:
   one Newton step towards unit length, sum taken as ((x*x + y*y) + z*z) + w*w */
static void qnorm_a(float q[4])
{
	D x = q[0], y = q[1], z = q[2], w = q[3];
	D s = (3.0 - (((x * x + y * y) + z * z) + w * w)) * 0.5;
	q[0] = F(s * x); q[1] = F(s * y); q[2] = F(s * z); q[3] = F(s * w);
}

/* rotation matrix of a quaternion (0x100185f0; same code inline in 0x100220b0 / 0x10021b90) */
static void rotmat(const float q[4], float m[9])
{
	D x = q[0], y = q[1], z = q[2], w = q[3];
	D xx = x * x, yy = y * y, zz = z * z, xy = y * x, zw = w * z;
	float XZ = F(z * x), YZ = F(z * y), XW = F(w * x), YW = F(w * y);
	m[0] = F(1.0 - (zz + yy) * 2.0);
	m[1] = F((xy - zw) * 2.0);
	m[2] = F(((D)YW + (D)XZ) * 2.0);
	m[3] = F((zw + xy) * 2.0);
	m[4] = F(1.0 - (zz + xx) * 2.0);
	m[5] = F(((D)YZ - (D)XW) * 2.0);
	m[6] = F(((D)XZ - (D)YW) * 2.0);
	m[7] = F(((D)XW + (D)YZ) * 2.0);
	m[8] = F(1.0 - (yy + xx) * 2.0);
}

/* local -> composed form (0x100220b0): q normalised, M = R(q) * ss, t = pos */
static void local_to_world(const Local &l, World &w)
{
	float q[4] = { l.q[0], l.q[1], l.q[2], l.q[3] }, R[9];
	qnorm_a(q);
	memcpy(w.q, q, sizeof q);
	{
		/* 0x100220b0 computes the rotation with the same sequence as rotmat but takes x*z as z*x
		   etc.; products commute, so rotmat is exact here */
		D x = q[0], y = q[1], z = q[2], ww = q[3];
		D xx = x * x, yy = y * y, zz = z * z, xy = y * x, zw = ww * z;
		float XZ = F(x * z), YZ = F(y * z), XW = F(x * ww), YW = F(y * ww);
		R[0] = F(1.0 - (zz + yy) * 2.0);
		R[1] = F((xy - zw) * 2.0);
		R[2] = F(((D)YW + (D)XZ) * 2.0);
		R[3] = F((zw + xy) * 2.0);
		R[4] = F(1.0 - (zz + xx) * 2.0);
		R[5] = F(((D)YZ - (D)XW) * 2.0);
		R[6] = F(((D)XZ - (D)YW) * 2.0);
		R[7] = F(((D)XW + (D)YZ) * 2.0);
		R[8] = F(1.0 - (yy + xx) * 2.0);
	}
	for (int r = 0; r < 3; r++)
		for (int c = 0; c < 3; c++)
			w.M[r * 3 + c] = F(((D)l.ss[c] * R[r * 3 + 0] + (D)l.ss[3 + c] * R[r * 3 + 1]) + (D)l.ss[6 + c] * R[r * 3 + 2]);
	w.t[0] = l.pos[0]; w.t[1] = l.pos[1]; w.t[2] = l.pos[2];
}

/* child := parent o child in composed form (0x100217b0) */
static void world_compose(World &c, const World &p)
{
	D cx = c.q[0], cy = c.q[1], cz = c.q[2], cw = c.q[3];
	D px = p.q[0], py = p.q[1], pz = p.q[2], pw = p.q[3];
	c.q[0] = F(((cz * py - cy * pz) + cw * px) + cx * pw);
	c.q[1] = F(((cx * pz - cz * px) + cw * py) + cy * pw);
	c.q[2] = F(((cy * px - cx * py) + cw * pz) + cz * pw);
	c.q[3] = F(cw * pw - ((cz * pz + cy * py) + cx * px));
	D tx = c.t[0], ty = c.t[1], tz = c.t[2];
	float t[3];
	for (int r = 0; r < 3; r++)
		t[r] = F((tz * p.M[r * 3 + 2] + ty * p.M[r * 3 + 1]) + tx * p.M[r * 3 + 0]);
	for (int r = 0; r < 3; r++) c.t[r] = F((D)p.t[r] + (D)t[r]);
	float m[9];
	const float *C = c.M;
	for (int r = 0; r < 3; r++)
	{
		const float *P = p.M + r * 3;
		m[r * 3 + 0] = F(((D)P[1] * C[3] + (D)C[6] * P[2]) + (D)P[0] * C[0]);
		m[r * 3 + 1] = F(((D)C[7] * P[2] + (D)P[1] * C[4]) + (D)P[0] * C[1]);
		m[r * 3 + 2] = F(((D)P[1] * C[5] + (D)C[2] * P[0]) + (D)C[8] * P[2]);
	}
	memcpy(c.M, m, sizeof m);
}

/* out = A * B (0x10021e50) */
static void mat_mul_e50(const float A[9], const float B[9], float out[9])
{
	float o[9];
	for (int r = 0; r < 3; r++)
	{
		const float *a = A + r * 3;
		o[r * 3 + 0] = F(((D)a[0] * B[0] + (D)B[3] * a[1]) + (D)B[6] * a[2]);
		o[r * 3 + 1] = F(((D)a[0] * B[1] + (D)a[2] * B[7]) + (D)a[1] * B[4]);
		o[r * 3 + 2] = F(((D)B[8] * a[2] + (D)B[5] * a[1]) + (D)B[2] * a[0]);
	}
	memcpy(out, o, sizeof o);
}

/* out = A^T * B (0x10021ec0) */
static void mat_tmul_ec0(const float A[9], const float B[9], float out[9])
{
	float o[9];
	for (int r = 0; r < 3; r++)
	{
		D a0 = A[r], a1 = A[3 + r], a2 = A[6 + r];
		o[r * 3 + 0] = F((a0 * B[0] + (D)B[3] * a1) + (D)B[6] * a2);
		o[r * 3 + 1] = F((a0 * B[1] + a2 * B[7]) + a1 * B[4]);
		o[r * 3 + 2] = F((a2 * B[8] + (D)B[5] * a1) + (D)B[2] * a0);
	}
	memcpy(out, o, sizeof o);
}

/* v = M v (0x10021f30) */
static void mat_vec_f30(const float M[9], float v[3])
{
	D x = v[0], y = v[1], z = v[2];
	for (int r = 0; r < 3; r++) v[r] = F((z * M[r * 3 + 2] + y * M[r * 3 + 1]) + x * M[r * 3 + 0]);
}

/* A := A o B in decomposed form, B acting as the parent (0x10021b90; used with the per-bone offset of
   a control, with a sequence's placement for the root, and to flatten a bone chain) */
static void local_compose(Local &A, const Local &B)
{
	D ax = A.q[0], ay = A.q[1], az = A.q[2], aw = A.q[3];
	D bx = B.q[0], by = B.q[1], bz = B.q[2], bw = B.q[3];
	float x = F(((az * by - ay * bz) + ax * bw) + aw * bx);
	float y = F(((ax * bz - az * bx) + ay * bw) + aw * by);
	float z = F(((ay * bx - ax * by) + az * bw) + aw * bz);
	D w = aw * bw - ((ax * bx + az * bz) + ay * by);
	D s = (3.0 - (((w * w + (D)z * z) + (D)y * y) + (D)x * x)) * 0.5;
	A.q[0] = F((D)x * s); A.q[1] = F((D)y * s); A.q[2] = F((D)z * s); A.q[3] = F(w * s);
	float T1[9], T2[9], X[9];
	rotmat(B.q, T1);
	rotmat(A.q, T2);
	const float *S = A.ss;
	for (int r = 0; r < 3; r++)
	{
		const float *t = T2 + r * 3;
		X[r * 3 + 0] = F(((D)t[1] * S[3] + (D)S[6] * t[2]) + (D)t[0] * S[0]);
		X[r * 3 + 1] = F(((D)t[1] * S[4] + (D)t[2] * S[7]) + (D)t[0] * S[1]);
		X[r * 3 + 2] = F(((D)t[1] * S[5] + (D)S[2] * t[0]) + (D)S[8] * t[2]);
	}
	mat_mul_e50(B.ss, X, A.ss);
	mat_tmul_ec0(T2, A.ss, A.ss);
	mat_vec_f30(B.ss, A.pos);
	mat_vec_f30(T1, A.pos);
	for (int i = 0; i < 3; i++) A.pos[i] = F((D)B.pos[i] + (D)A.pos[i]);
}

/* A := blend(A, B, w): lerp position and scale/shear, slerp rotation (0x10021910) */
static void blend(Local &A, float w, const Local &B)
{
	float omw = F(1.0 - (D)w);
	for (int i = 0; i < 3; i++) A.pos[i] = F((D)F((D)A.pos[i] * omw) + (D)F((D)B.pos[i] * w));
	float ca = omw, cb = w, sign = 1.0f;
	float d = F(((((D)A.q[0] * B.q[0] + (D)A.q[1] * B.q[1]) + (D)A.q[2] * B.q[2]) + (D)A.q[3] * B.q[3]));
	if (d < 0.0f) { sign = -1.0f; d = -d; }
	if (1.0 - (D)d > (D)9.99999974737875e-05f)
	{
		D th = acos((D)d), inv = 1.0 / sin(th);
		ca = F(sin(th * ca) * inv * sign);
		cb = F(sin(th * cb) * inv);
	}
	else
		ca = F((D)sign * ca);
	float q[4];
	for (int i = 0; i < 4; i++) q[i] = F((D)F((D)B.q[i] * cb) + (D)F((D)A.q[i] * ca));
	for (int i = 0; i < 9; i++) A.ss[i] = F((D)w * B.ss[i] + (D)omw * A.ss[i]);
	memcpy(A.q, q, sizeof q);
	qnorm_a(A.q);
}

/* ------------------------------------------------------------------ root motion (0x10026c60) --- */

/* out = A * B, sums in index order (0x100183e0) */
static void m3_mul(const float A[9], const float B[9], float out[9])
{
	float o[9];
	for (int r = 0; r < 3; r++)
		for (int c = 0; c < 3; c++)
			o[r * 3 + c] = F(((D)A[r * 3] * B[c] + (D)A[r * 3 + 1] * B[3 + c]) + (D)A[r * 3 + 2] * B[6 + c]);
	memcpy(out, o, sizeof o);
}

static void m3_transpose(const float m[9], float out[9])     /* 0x10018260 */
{
	float o[9];
	for (int r = 0; r < 3; r++) for (int c = 0; c < 3; c++) o[c * 3 + r] = m[r * 3 + c];
	memcpy(out, o, sizeof o);
}

/* out = M v, sums in index order (0x10018470) */
static void m3_vec(const float M[9], const float v[3], float out[3])
{
	float o[3];
	for (int r = 0; r < 3; r++) o[r] = F(((D)v[0] * M[r * 3] + (D)v[1] * M[r * 3 + 1]) + (D)v[2] * M[r * 3 + 2]);
	memcpy(out, o, sizeof o);
}

/* 3x3 inverse by cofactors (0x100182b0); a singular matrix is returned unchanged */
static void m3_inverse(const float m[9], float out[9])
{
	D c0 = (D)m[4] * m[8] - (D)m[7] * m[5];
	float c1 = F((D)m[3] * m[8] - (D)m[6] * m[5]);
	float c2 = F((D)m[7] * m[3] - (D)m[6] * m[4]);
	D det = (c0 * m[0] - (D)c1 * m[1]) + (D)c2 * m[2];
	if (det == 0.0) { memcpy(out, m, 36); return; }
	D id = 1.0 / det;
	float r[9];
	r[0] = F(c0 * id);
	r[3] = F(-((D)c1 * id));
	r[6] = F((D)c2 * id);
	r[1] = F(-(((D)m[1] * m[8] - (D)m[7] * m[2]) * id));
	r[4] = F(((D)m[0] * m[8] - (D)m[6] * m[2]) * id);
	r[7] = F(-(((D)m[7] * m[0] - (D)m[6] * m[1]) * id));
	r[2] = F(((D)m[1] * m[5] - (D)m[2] * m[4]) * id);
	r[5] = F(-(((D)m[0] * m[5] - (D)m[3] * m[2]) * id));
	r[8] = F(((D)m[4] * m[0] - (D)m[3] * m[1]) * id);
	memcpy(out, r, sizeof r);
}

/* inverse of a decomposed transform as 0x10026c60 computes it inline:
   q' = conj q, ss' = R(q')^T ss^-1 R(q'), pos' = R(q') ss' (-pos) */
static void root_inverse(const Local &a, Local &o)
{
	float cq[4] = { -a.q[0], -a.q[1], -a.q[2], a.q[3] };
	float Rc[9], Rct[9], inv[9], M[9], neg[3], v[3];
	rotmat(cq, Rc);
	m3_inverse(a.ss, inv);
	m3_transpose(Rc, Rct);
	m3_mul(Rct, inv, M);
	m3_mul(M, Rc, M);
	for (int i = 0; i < 3; i++) neg[i] = -a.pos[i];
	m3_vec(M, neg, v);
	m3_vec(Rc, v, o.pos);
	memcpy(o.q, cq, sizeof cq);
	memcpy(o.ss, M, sizeof M);
}

/* out = X o Y (Y applied first) in decomposed form.  round_ww selects 0x100280c0, which rounds
   wX*wY to float before subtracting the dot product; the inline copy in 0x10026c60 (quaternion
   product 0x100186d0) does not */
static void root_compose(const Local &X, const Local &Y, Local &out, bool round_ww)
{
	const float *vx = X.q, *vy = Y.q;
	float wx = X.q[3], wy = Y.q[3];
	float p1[3], p2[3], c[3], q[4];
	for (int i = 0; i < 3; i++) { p1[i] = F((D)wy * vx[i]); p2[i] = F((D)wx * vy[i]); }
	c[0] = F((D)vy[2] * vx[1] - (D)vx[2] * vy[1]);
	c[1] = F((D)vx[2] * vy[0] - (D)vy[2] * vx[0]);
	c[2] = F((D)vy[1] * vx[0] - (D)vy[0] * vx[1]);
	for (int i = 0; i < 3; i++) q[i] = F((D)F((D)c[i] + p2[i]) + p1[i]);
	D dot = ((D)vx[0] * vy[0] + (D)vx[1] * vy[1]) + (D)vx[2] * vy[2];
	q[3] = round_ww ? F((D)F((D)wy * wx) - dot) : F((D)wy * wx - dot);
	float RX[9], RY[9], RYt[9], S[9], v[3], pos[3];
	rotmat(X.q, RX);
	rotmat(Y.q, RY);
	m3_transpose(RY, RYt);
	m3_mul(RYt, X.ss, S);
	m3_mul(S, RY, S);
	m3_mul(S, Y.ss, S);
	m3_vec(X.ss, Y.pos, v);
	m3_vec(RX, v, v);
	for (int i = 0; i < 3; i++) pos[i] = F((D)v[i] + X.pos[i]);
	memcpy(out.pos, pos, sizeof pos);
	memcpy(out.q, q, sizeof q);
	memcpy(out.ss, S, sizeof S);
}

/* the per-track offset of a control (0x10031080 called from 0x1002ac50): B o A^-1 where granny looks
   both A and B up by the track's name in the same table, the sequence's model skeleton - so it is the
   bone's rest transform composed with its own inverse: an identity up to rounding noise (positions
   ~1e-6, scale/shear ~1e-7) that granny composes onto every sample of every track 1.. */
static void track_offset(const Local &b, const Local &a, Local &out)
{
	Local inv;
	root_inverse(a, inv);
	root_compose(b, inv, out, false);
}

/* inverse of a 3x3 by cofactors (as 0x10024853 / 0x1002a600) - only used for attaching by a child bone
   other than 0, which the game never does */
static void mat_inverse(const float m[9], float o[9])
{
	D det = ((D)m[4] * m[8] - (D)m[7] * m[5]) * m[0] - ((D)m[3] * m[8] - (D)m[6] * m[5]) * m[1]
	      + ((D)m[7] * m[3] - (D)m[6] * m[4]) * m[2];
	if (det == 0.0) { memcpy(o, m, 36); return; }
	D id = 1.0 / det;
	float r[9];
	r[0] = F(((D)m[8] * m[4] - (D)m[5] * m[7]) * id);
	r[3] = F(-(((D)m[8] * m[3] - (D)m[6] * m[5]) * id));
	r[6] = F(((D)m[7] * m[3] - (D)m[6] * m[4]) * id);
	r[1] = F(-(((D)m[8] * m[1] - (D)m[7] * m[2]) * id));
	r[4] = F(((D)m[8] * m[0] - (D)m[2] * m[6]) * id);
	r[7] = F(-(((D)m[7] * m[0] - (D)m[1] * m[6]) * id));
	r[2] = F(((D)m[5] * m[1] - (D)m[2] * m[4]) * id);
	r[5] = F(-(((D)m[5] * m[0] - (D)m[2] * m[3]) * id));
	r[8] = F(((D)m[0] * m[4] - (D)m[1] * m[3]) * id);
	memcpy(o, r, sizeof r);
}

/* inverse of a decomposed transform (approximately what 0x10021f70 + 0x1001c140 compute) */
static void local_inverse(const Local &a, Local &o)
{
	World w;
	local_to_world(a, w);
	float Mi[9];
	mat_inverse(w.M, Mi);
	o.q[0] = -w.q[0]; o.q[1] = -w.q[1]; o.q[2] = -w.q[2]; o.q[3] = w.q[3];
	float R[9], Rt[9];
	rotmat(o.q, R);
	for (int r = 0; r < 3; r++) for (int c = 0; c < 3; c++) Rt[r * 3 + c] = R[c * 3 + r];
	/* ss = R(o.q)^T * M^-1 */
	for (int r = 0; r < 3; r++)
		for (int c = 0; c < 3; c++)
			o.ss[r * 3 + c] = F((D)Rt[r * 3] * Mi[c] + (D)Rt[r * 3 + 1] * Mi[3 + c] + (D)Rt[r * 3 + 2] * Mi[6 + c]);
	for (int r = 0; r < 3; r++)
		o.pos[r] = F(-((D)Mi[r * 3] * w.t[0] + (D)Mi[r * 3 + 1] * w.t[1] + (D)Mi[r * 3 + 2] * w.t[2]));
}

/* ----------------------------------------------------------------------------------- b-splines --- */

/* 0x1001fa50 / 0x1001fd10 / 0x10020020: evaluate around knot k (pointer to knot i) and control point c */
static void bspline_eval(int degree, float t, const float *k, const float *c, int dim, float *out)
{
	switch (degree)
	{
	case 0:
		for (int i = 0; i < dim; i++) out[i] = c[i];
		return;
	case 1:
	{
		D f = ((D)t - k[-1]) / ((D)k[0] - k[-1]), g = 1.0 - f;
		for (int i = 0; i < dim; i++) out[i] = F(g * c[i - dim] + f * c[i]);
		return;
	}
	case 2:
	{
		float d1 = F((D)t - k[-1]);
		float f3 = F((D)d1 / ((D)k[0] - k[-1]));
		D f2 = ((D)t - k[-2]) / ((D)k[0] - k[-2]);
		float s2 = F(((D)f3 + f2) - f2 * f3);
		D q = (D)d1 / ((D)k[1] - k[-1]) * f3;
		D c1 = (D)s2 - q, c2 = 1.0 - (D)s2;
		for (int i = 0; i < dim; i++)
			out[i] = F((c2 * c[i - 2 * dim] + c1 * c[i - dim]) + q * c[i]);
		return;
	}
	default:
	{
		/* cubic: not used by the game's files; same formula as granny's case 3 */
		D t0 = t, k0 = k[0], km1 = k[-1], km2 = k[-2], km3 = k[-3];
		float a = F(t0 - km1), b = F(t0 - km2);
		float f9 = F(a / (k0 - km1)), f8 = F(b / (k0 - km2)), f4 = F(a / ((D)k[1] - km1));
		float f6 = F((t0 - km3) / (k0 - km3)), f1 = F(b / ((D)k[1] - km2)), f5 = F(a / ((D)k[2] - km1));
		D p3 = (1.0 - f8) * (1.0 - f9), p2 = (D)f4 * f9, c7 = (1.0 - f6) * p3;
		D p4 = (1.0 - f4) * f9 + (1.0 - f9) * f8;
		D c3 = p3 * f6 + (1.0 - f1) * p4, c1 = p4 * f1 + (1.0 - f5) * p2, c0 = p2 * f5;
		for (int i = 0; i < dim; i++)
			out[i] = F(((c0 * c[i] + c1 * c[i - dim]) + c3 * c[i - 2 * dim]) + c7 * c[i - 3 * dim]);
		return;
	}
	}
}

struct Curve
{
	int degree;
	int knots;                    /* knot array used */
	std::vector<float> cps;       /* padded: 3 copies of the first, n points, 2 copies of the last */
};

struct Track
{
	std::string name;
	int kind;                     /* 1 = b-spline, 0 = raw keys */
	std::vector<float> knots[3];  /* padded: k0-3, k0-2, k0-1, n knots, last, last */
	int nknots[3];
	Curve curve[3];               /* position (3), orientation (4), scale/shear (9) */
	std::vector<float> keys;      /* raw: n x { time, pos[3], quat[4], ss[9] } */
};

static const int curve_dim[3] = { 3, 4, 9 };

/* bspline_motion::sample (0x1001f8c0) / raw_motion::sample (0x1001f160) */
static bool sample_track(const Track &tr, D t, Local &out)
{
	if (tr.kind == 0)
	{
		size_t n = tr.keys.size() / 17, i = 0;
		if (!n) return false;
		while (i < n && !((D)tr.keys[i * 17] >= t)) i++;
		if (i == n) i--;
		const float *k = &tr.keys[i * 17];
		memcpy(out.pos, k + 1, 12); memcpy(out.q, k + 4, 16); memcpy(out.ss, k + 8, 36);
		return true;
	}
	bool any = false;
	float *dst[3] = { out.pos, out.q, out.ss };
	for (int c = 0; c < 3; c++)
	{
		const Curve &cv = tr.curve[c];
		int n = tr.nknots[cv.knots];
		if (!n) continue;
		const std::vector<float> &K = tr.knots[cv.knots];
		int count = n + 5, i = 3, lim = count - 3;
		if (lim > 3)
			while (i < lim && (D)K[i] < t) i++;
		bspline_eval(cv.degree, F(t), &K[i], &cv.cps[i * curve_dim[c]], curve_dim[c], dst[c]);
		if (c == 1) qnorm_a(out.q);
		any = true;
	}
	return any;
}

/* --------------------------------------------------------------------------------- data files --- */

struct Binding { u32 bone; float bt[3]; float B[9]; };

struct MeshState { u32 texture; u32 src_blend, dst_blend; std::vector<u16> tris; };

struct Mesh
{
	std::vector<Binding> bindings;
	u32 nrender;
	std::vector<float> pos, nrm, uv, col;
	u32 nsource;
	std::vector<u32> weights;     /* per source vertex: n, n x { binding, f32 bits } */
	std::vector<u32> wofs;        /* offset of each source vertex in weights */
	std::vector<u32> copies;      /* per source vertex: n, n x relative render index */
	std::vector<u32> normal_vertex;
	std::vector<MeshState> states;
	bool rigid() const { return nsource == 0 && bindings.size() < 2; }
};

struct TextureData { u32 w, h, format, bpp, pitch; std::vector<unsigned char> pixels; };

struct BoneDef
{
	std::string name;
	int parent;
	Local rest;
	Local offset;       /* rest o rest^-1 as granny computes it: see track_offset */
};

struct GrcFile
{
	std::string path;
	int refs;
	bool has_model, has_anim;
	std::vector<BoneDef> bones;
	std::vector<TextureData> textures;
	std::vector<Mesh> meshes;
	D duration;
	std::vector<Track> tracks;
};

struct Reader
{
	const unsigned char *p, *e;
	bool ok;
	u32 u() { if (e - p < 4) { ok = false; return 0; } u32 v; memcpy(&v, p, 4); p += 4; return v; }
	float f() { u32 v = u(); float r; memcpy(&r, &v, 4); return r; }
	D d() { if (e - p < 8) { ok = false; return 0; } D v; memcpy(&v, p, 8); p += 8; return v; }
	void bytes(void *dst, size_t n) { if ((size_t)(e - p) < n) { ok = false; return; } memcpy(dst, p, n); p += n; }
	void floats(std::vector<float> &v, size_t n) { v.resize(n); if (n) bytes(&v[0], n * 4); }
	void words(std::vector<u32> &v, size_t n) { v.resize(n); if (n) bytes(&v[0], n * 4); }
	std::string str() { u32 n = u(); if ((u32)(e - p) < n) { ok = false; return std::string(); } std::string s((const char *)p, n); p += n; return s; }
};

static std::map<std::string, GrcFile *> g_files;
static std::string g_datadir;

static std::string grc_path(const char *file)
{
	std::string s(file);
	for (size_t i = 0; i < s.size(); i++) if (s[i] == '\\') s[i] = '/';
	size_t slash = s.rfind('/');
	std::string dir = slash == std::string::npos ? std::string() : s.substr(0, slash + 1);
	std::string base = slash == std::string::npos ? s : s.substr(slash + 1);
	size_t dot = base.rfind('.');
	if (dot != std::string::npos) base = base.substr(0, dot);
	for (size_t i = 0; i < base.size(); i++) if (base[i] >= 'A' && base[i] <= 'Z') base[i] = (char)(base[i] - 'A' + 'a');
	if (!g_datadir.empty()) return g_datadir + "/" + base + ".grc";
	return dir + base + ".grc";
}

static void pad_track(Track &t)
{
	for (int a = 0; a < 3; a++)
	{
		std::vector<float> &k = t.knots[a];
		int n = t.nknots[a];
		if (!n) continue;
		std::vector<float> p(n + 5);
		for (int i = 0; i < n; i++) p[i + 3] = k[i];
		D v = (D)p[3] - 1.0; p[2] = F(v); v -= 1.0; p[1] = F(v); v -= 1.0; p[0] = F(v);
		p[n + 3] = p[n + 2]; p[n + 4] = p[n + 2];
		k.swap(p);
	}
	for (int c = 0; c < 3; c++)
	{
		Curve &cv = t.curve[c];
		int n = t.nknots[cv.knots], dim = curve_dim[c];
		if (!n) continue;
		std::vector<float> p((n + 5) * dim);
		for (int i = 0; i < n * dim; i++) p[3 * dim + i] = cv.cps[i];
		for (int j = 0; j < dim; j++)
		{
			p[j] = p[dim + j] = p[2 * dim + j] = p[3 * dim + j];
			p[(n + 3) * dim + j] = p[(n + 4) * dim + j] = p[(n + 2) * dim + j];
		}
		cv.cps.swap(p);
	}
}

static GrcFile *load_grc(const char *file, granny_result *err)
{
	std::string path = grc_path(file);
	std::map<std::string, GrcFile *>::iterator it = g_files.find(path);
	if (it != g_files.end()) { it->second->refs++; return it->second; }
	FILE *fp = fopen(path.c_str(), "rb");
	if (!fp) { *err = GrannyUnableToReadFromFile; return 0; }
	std::vector<unsigned char> buf;
	{
		unsigned char tmp[65536];
		size_t n;
		while ((n = fread(tmp, 1, sizeof tmp, fp)) > 0) buf.insert(buf.end(), tmp, tmp + n);
		fclose(fp);
	}
	Reader r = { buf.empty() ? 0 : &buf[0], buf.empty() ? 0 : &buf[0] + buf.size(), true };
	if (buf.size() < 12 || memcmp(&buf[0], "GRC1", 4)) { *err = GrannyUnableToParseFile; return 0; }
	r.p += 4;
	GrcFile *g = new GrcFile;
	g->path = path; g->refs = 1;
	u32 version = r.u(), flags = r.u();
	if (version != 1) { delete g; *err = GrannyUnableToParseFile; return 0; }
	g->has_model = (flags & 1) != 0; g->has_anim = (flags & 2) != 0;
	g->duration = 0;
	if (g->has_model)
	{
		u32 nb = r.u();
		g->bones.resize(nb);
		for (u32 i = 0; i < nb && r.ok; i++)
		{
			BoneDef &b = g->bones[i];
			b.name = r.str(); b.parent = (int)r.u();
			r.bytes(b.rest.pos, 12); r.bytes(b.rest.q, 16); r.bytes(b.rest.ss, 36);
			track_offset(b.rest, b.rest, b.offset);
		}
		u32 nt = r.u();
		g->textures.resize(nt);
		for (u32 i = 0; i < nt && r.ok; i++)
		{
			TextureData &t = g->textures[i];
			t.w = r.u(); t.h = r.u(); t.format = r.u(); t.bpp = r.u(); t.pitch = r.u();
			u32 size = r.u();
			t.pixels.resize(size);
			if (size) r.bytes(&t.pixels[0], size);
		}
		u32 nm = r.u();
		g->meshes.resize(nm);
		for (u32 i = 0; i < nm && r.ok; i++)
		{
			Mesh &m = g->meshes[i];
			u32 nbd = r.u();
			m.bindings.resize(nbd);
			for (u32 k = 0; k < nbd && r.ok; k++) { m.bindings[k].bone = r.u(); r.bytes(m.bindings[k].bt, 12); r.bytes(m.bindings[k].B, 36); }
			m.nrender = r.u();
			r.floats(m.pos, m.nrender * 3); r.floats(m.nrm, m.nrender * 3);
			r.floats(m.uv, m.nrender * 3); r.floats(m.col, m.nrender * 3);
			m.nsource = r.u();
			r.words(m.weights, r.u());
			r.words(m.copies, r.u());
			r.words(m.normal_vertex, r.u());
			m.wofs.resize(m.nsource + 1);
			{
				u32 p = 0;
				for (u32 v = 0; v < m.nsource; v++) { m.wofs[v] = p; p += 1 + 2 * (p < m.weights.size() ? m.weights[p] : 0); }
				m.wofs[m.nsource] = p;
				if (p > m.weights.size()) r.ok = false;
			}
			u32 ns = r.u();
			m.states.resize(ns);
			for (u32 s = 0; s < ns && r.ok; s++)
			{
				MeshState &st = m.states[s];
				st.texture = r.u(); st.src_blend = r.u(); st.dst_blend = r.u();
				u32 nt3 = r.u();
				st.tris.resize(nt3 * 3);
				if (nt3) r.bytes(&st.tris[0], nt3 * 6);
				if ((nt3 * 6) & 2) r.p += 2;
			}
		}
	}
	if (g->has_anim)
	{
		g->duration = r.d();
		u32 nt = r.u();
		g->tracks.resize(nt);
		for (u32 i = 0; i < nt && r.ok; i++)
		{
			Track &t = g->tracks[i];
			t.name = r.str();
			t.kind = (int)r.u();
			if (t.kind == 0) { u32 nk = r.u(); r.floats(t.keys, nk * 17); continue; }
			for (int a = 0; a < 3; a++) { t.nknots[a] = (int)r.u(); r.floats(t.knots[a], t.nknots[a]); }
			for (int c = 0; c < 3; c++)
			{
				t.curve[c].degree = (int)r.u(); t.curve[c].knots = (int)r.u();
				if (t.curve[c].knots < 0 || t.curve[c].knots > 2) { r.ok = false; break; }
				r.floats(t.curve[c].cps, t.nknots[t.curve[c].knots] * curve_dim[c]);
			}
			pad_track(t);
		}
	}
	if (!r.ok) { delete g; *err = GrannyUnableToParseFile; return 0; }
	g_files[path] = g;
	return g;
}

static void release_grc(GrcFile *g)
{
	if (g && --g->refs == 0) { g_files.erase(g->path); delete g; }
}

/* --------------------------------------------------------------------------------- the runtime --- */

static const u32 VERSION_HANDLE = 0x82ca5e2a;

struct Model;
struct Sequence;
struct Animation;

struct TextureInst { Model *model; u32 index; granny_cookie cookie; bool delivered; };

/* models and animations are reference counted like in the DLL: closing one that a sequence (model)
   or a control (animation) still uses only invalidates its handle */
struct Model
{
	u32 id;
	GrcFile *file;
	std::vector<TextureInst *> textures;
	u32 refs;           /* open sequences */
	bool closed;
};

struct Animation { u32 id; GrcFile *file; u32 refs; bool closed; };

struct Control
{
	u32 id;
	Sequence *seq;
	Animation *anim;
	std::vector<int> track_bone;         /* sequence bone driven by each track (track 0: the root), -1: none */
	D start, speed, loops, blendin, blendout, frozen_local, duration;
	float w_hold, w_before, w_after, priority;
	bool active, released;
};

struct Sequence
{
	u32 id;
	Model *model;
	std::vector<Local> local;
	std::vector<World> world;
	Local placement;
	Sequence *parent;
	u32 parent_bone;
	bool enabled;
	u32 frame;
};

struct Lock
{
	Sequence *seq;
	u32 present;
	u32 next;                                   /* index of the next state */
	std::vector<std::pair<u32, u32> > states;   /* (mesh, state in mesh) */
	bool state_locked;
	std::vector<std::vector<float> > pos, nrm;  /* deformed, per mesh */
	std::vector<bool> done;
};

static bool g_open;
static u32 g_next_id;
static D g_clock;
static u32 g_frame;
static granny_result g_last_error;
static std::vector<Model *> g_models;
static std::vector<Animation *> g_anims;
static std::vector<Sequence *> g_seqs;
static std::vector<Control *> g_controls;       /* the clock's list: creation order */
static std::vector<TextureInst *> g_new_textures;
static Lock *g_lock;
static const float g_identity4[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

static granny_result ret(granny_result r) { g_last_error = r; return r; }

template <class T> static T *find_id(std::vector<T *> &v, u32 id)
{
	for (size_t i = 0; i < v.size(); i++) if (v[i]->id == id) return v[i];
	return 0;
}

static Model *find_model(u32 h0, u32 h1)
{
	Model *m = g_open && h0 == VERSION_HANDLE ? find_id(g_models, h1) : 0;
	return m && !m->closed ? m : 0;
}
static Animation *find_anim(u32 h0, u32 h1)
{
	Animation *a = g_open && h0 == VERSION_HANDLE ? find_id(g_anims, h1) : 0;
	return a && !a->closed ? a : 0;
}
static Sequence *find_seq(u32 h0, u32 h1) { return g_open && h0 == VERSION_HANDLE ? find_id(g_seqs, h1) : 0; }
static Control *find_ctrl(u32 c0, u32 c1, u32 c2)
{
	if (!g_open || c0 != VERSION_HANDLE) return 0;
	Control *c = find_id(g_controls, c2);
	return c && !c->released && c->seq->id == c1 ? c : 0;
}

static const GrcFile *seq_file(const Sequence *s) { return s->model->file; }

/* total length (0x10022600), end time (0x10022620) */
static D ctrl_total(const Control *c) { return c->duration / c->speed * c->loops; }
static D ctrl_end(const Control *c) { return ctrl_total(c) + c->start; }
static bool ctrl_alive(const Control *c, D t) { return c->active && ctrl_end(c) > t; }

/* weight at global time t (0x100229f0) */
static float ctrl_weight(const Control *c, D t)
{
	D total = ctrl_total(c), out_start = total - c->blendout, in = c->blendin;
	D rel = t - c->start;
	if (rel < 0.0) return c->w_before;
	if (rel < in) return F((rel - 0.0) / (in - 0.0) * ((D)c->w_hold - (D)c->w_before) + (D)c->w_before);
	if (rel < out_start) return c->w_hold;
	if (rel < total) return F((rel - out_start) / (total - out_start) * ((D)c->w_after - (D)c->w_hold) + (D)c->w_hold);
	return c->w_after;
}

/* root motion between two local times (0x10026c60): the root track's change from t0 to t1,
   delta = S(t0)^-1 o S(t1); when t1 runs past the end of the animation the part past the end
   wraps around: delta = (S(0)^-1 o S(t1 - duration)) o S(t0)^-1 o S(duration) */
static bool root_delta(const Control *c, D t0, D t1, Local &out)
{
	const Track &tr = c->anim->file->tracks[0];
	local_identity(out);
	D dur = c->duration, t2 = t1;
	if (t1 > dur)
	{
		D t3 = t1 - dur;
		t2 = dur;
		Local C, Dl;
		local_identity(C);
		local_identity(Dl);
		if (sample_track(tr, 0.0, C) && sample_track(tr, t3, Dl))
		{
			Local Ci;
			root_inverse(C, Ci);
			root_compose(out, Ci, out, false);
			root_compose(out, Dl, out, false);
		}
	}
	Local A, B;
	local_identity(A);
	local_identity(B);
	if (sample_track(tr, t0, A) && sample_track(tr, t2, B))
	{
		Local Ai;
		root_inverse(A, Ai);
		root_compose(out, Ai, out, true);
		root_compose(out, B, out, true);
	}
	return true;
}

/* rst_affector::affect (0x1002aae0) with 0x10026b50: tracks 1.. drive the bones of the same name, in
   track order (a later track for the same bone wins); track 0 moves the root by the root motion
   between the previous and the current local time (granny's default "accumulate" mode, sequence
   flag +0x6d = 1, which the game never changes) */
static void ctrl_affect(Control *c, D local_time, D old_local, D new_local, float w)
{
	Sequence *s = c->seq;
	if (!s->enabled) return;
	const std::vector<Track> &tracks = c->anim->file->tracks;
	for (size_t i = 1; i < tracks.size(); i++)
	{
		int b = c->track_bone[i];
		if (b < 0) continue;
		Local X;
		local_identity(X);
		if (!sample_track(tracks[i], local_time, X)) continue;
		local_compose(X, seq_file(s)->bones[b].offset);
		if (w == 1.0f) s->local[b] = X;
		else blend(s->local[b], w, X);
	}
	if (!tracks.empty() && !s->local.empty())
	{
		Local X;
		if (root_delta(c, old_local, new_local, X))
		{
			local_compose(X, s->local[0]);
			blend(s->local[0], w, X);
		}
	}
}

/* animation_affector::update (0x10022770) */
static void ctrl_update(Control *c, D old_t, D new_t)
{
	if (!c->active) return;
	D dur = c->duration;
	D local = (old_t - c->start) * c->speed;                 /* 0x10022910 */
	if (!(c->loops == 1.0) && local / dur < c->loops)       /* 0x10022930 */
		local = fmod(local, dur);
	/* root motion interval (0x10022980): from the wrapped old local time, unclamped and not frozen */
	D old_local = local, new_local = (new_t - old_t) * c->speed + local;
	if (local > dur) local = dur;
	if (c->speed > 0.0001) c->frozen_local = local;
	else local = c->frozen_local;
	ctrl_affect(c, local, old_local, new_local, ctrl_weight(c, new_t));
}

static void bone_world(Sequence *s, size_t i)
{
	local_to_world(s->local[i], s->world[i]);
	if (i == 0)
	{
		if (s->parent) world_compose(s->world[0], s->parent->world[s->parent_bone]);
	}
	else
		world_compose(s->world[i], s->world[seq_file(s)->bones[i].parent]);
}

/* 0x1001c5d0 */
static void seq_update(Sequence *s)
{
	if (!s->enabled || s->frame == g_frame) return;
	s->frame = g_frame;
	if (s->parent) seq_update(s->parent);
	for (size_t i = 0; i < s->local.size(); i++) bone_world(s, i);
}

static void seq_compute_all(Sequence *s)
{
	for (size_t i = 0; i < s->local.size(); i++) bone_world(s, i);
}

/* decomposed transform of bone i relative to the top of its chain (0x1002a5a0) */
static void flatten(Sequence *s, u32 i, Local &out)
{
	out = s->local[i];
	for (;;)
	{
		Sequence *ps = s;
		int pi;
		if (i == 0) { if (!s->parent) break; ps = s->parent; pi = (int)s->parent_bone; }
		else pi = seq_file(s)->bones[i].parent;
		local_compose(out, ps->local[pi]);
		s = ps; i = (u32)pi;
	}
}

static void seq_detach(Sequence *s)
{
	if (!s->parent) return;
	Local l;
	flatten(s, 0, l);
	s->local[0] = l;
	s->placement = l;
	s->parent = 0;
}

static void free_model(Model *m)
{
	for (size_t k = 0; k < m->textures.size(); k++)
	{
		for (size_t i = 0; i < g_new_textures.size(); i++)
			if (g_new_textures[i] == m->textures[k]) { g_new_textures.erase(g_new_textures.begin() + (long)i); break; }
		delete m->textures[k];
	}
	for (size_t i = 0; i < g_models.size(); i++) if (g_models[i] == m) { g_models.erase(g_models.begin() + (long)i); break; }
	release_grc(m->file);
	delete m;
}

static void free_anim(Animation *a)
{
	for (size_t i = 0; i < g_anims.size(); i++) if (g_anims[i] == a) { g_anims.erase(g_anims.begin() + (long)i); break; }
	release_grc(a->file);
	delete a;
}

static void destroy_control(Control *c)
{
	for (size_t i = 0; i < g_controls.size(); i++)
		if (g_controls[i] == c) { g_controls.erase(g_controls.begin() + (long)i); break; }
	Animation *a = c->anim;
	delete c;
	if (--a->refs == 0 && a->closed) free_anim(a);
}

/* ---------------------------------------------------------------------------------- skinning --- */

static void skin_mesh(const Sequence *s, const Mesh &m, bool want_pos, bool want_nrm,
                      std::vector<float> &out_pos, std::vector<float> &out_nrm)
{
	size_t nb = m.bindings.size();
	std::vector<float> L(nb * 9), T(nb * 3);
	for (size_t b = 0; b < nb; b++)               /* composite per binding (0x100298e0) */
	{
		const World &w = s->world[m.bindings[b].bone];
		const float *Bm = m.bindings[b].B, *bt = m.bindings[b].bt;
		float *l = &L[b * 9];
		for (int r = 0; r < 3; r++)
		{
			const float *Mr = w.M + r * 3;
			l[r * 3 + 0] = F(((D)Mr[1] * Bm[3] + (D)Bm[6] * Mr[2]) + (D)Mr[0] * Bm[0]);
			l[r * 3 + 1] = F(((D)Bm[7] * Mr[2] + (D)Mr[0] * Bm[1]) + (D)Mr[1] * Bm[4]);
			l[r * 3 + 2] = F(((D)Mr[1] * Bm[5] + (D)Bm[8] * Mr[2]) + (D)Mr[0] * Bm[2]);
		}
		for (int r = 0; r < 3; r++)
			T[b * 3 + r] = F(((D)bt[2] * l[r * 3 + 2] + (D)bt[1] * l[r * 3 + 1]) + (D)bt[0] * l[r * 3 + 0]);
		for (int r = 0; r < 3; r++) T[b * 3 + r] = F((D)w.t[r] + (D)T[b * 3 + r]);
	}
	size_t n = m.nrender;
	if (want_pos)
	{
		out_pos.assign(n * 3, 0.0f);
		if (m.rigid())
		{
			if (nb)                                  /* 0x100292e0 */
				for (size_t v = 0; v < n; v++)
				{
					D x = m.pos[v * 3], y = m.pos[v * 3 + 1], z = m.pos[v * 3 + 2];
					for (int r = 0; r < 3; r++)
						out_pos[v * 3 + r] = F(((x * L[r * 3] + y * L[r * 3 + 1]) + z * L[r * 3 + 2]) + (D)T[r]);
				}
		}
		else
		{
			for (u32 v = 0; v < m.nsource; v++)     /* 0x100291d0 */
			{
				D x = m.pos[v * 3], y = m.pos[v * 3 + 1], z = m.pos[v * 3 + 2], acc[3] = { 0.0, 0.0, 0.0 };
				const u32 *wl = &m.weights[m.wofs[v]];
				for (u32 k = 0; k < wl[0]; k++)
				{
					u32 b = wl[1 + 2 * k];
					float wt;
					memcpy(&wt, &wl[2 + 2 * k], 4);
					if (b >= nb) continue;
					const float *l = &L[b * 9];
					for (int r = 0; r < 3; r++)
						acc[r] = acc[r] + ((((x * l[r * 3] + y * l[r * 3 + 1]) + z * l[r * 3 + 2]) + (D)T[b * 3 + r]) * (D)wt);
				}
				for (int r = 0; r < 3; r++) out_pos[v * 3 + r] = F(acc[r]);
			}
			u32 p = 0;                               /* 0x10029830 */
			for (u32 v = 0; v < m.nsource && p < m.copies.size(); v++)
			{
				u32 cnt = m.copies[p++];
				for (u32 k = 0; k < cnt && p < m.copies.size(); k++)
				{
					u32 dst = v + m.copies[p++];
					if (dst < n) memcpy(&out_pos[dst * 3], &out_pos[v * 3], 12);
				}
			}
		}
	}
	if (want_nrm)
	{
		out_nrm.assign(n * 3, 0.0f);
		if (m.rigid())
		{
			if (nb)                                  /* 0x10029360 */
				for (size_t v = 0; v < n; v++)
				{
					D x = m.nrm[v * 3], y = m.nrm[v * 3 + 1], z = m.nrm[v * 3 + 2];
					for (int r = 0; r < 3; r++) out_nrm[v * 3 + r] = F((x * L[r * 3] + y * L[r * 3 + 1]) + z * L[r * 3 + 2]);
				}
		}
		else
		{
			for (size_t v = 0; v < m.normal_vertex.size() && v < n; v++)   /* 0x10029710 */
			{
				u32 sv = m.normal_vertex[v];
				D x = m.nrm[v * 3], y = m.nrm[v * 3 + 1], z = m.nrm[v * 3 + 2], acc[3] = { 0.0, 0.0, 0.0 };
				if (sv < m.nsource)
				{
					const u32 *wl = &m.weights[m.wofs[sv]];
					for (u32 k = 0; k < wl[0]; k++)
					{
						u32 b = wl[1 + 2 * k];
						float wt;
						memcpy(&wt, &wl[2 + 2 * k], 4);
						if (b >= nb) continue;
						const float *l = &L[b * 9];
						for (int r = 0; r < 3; r++)
							acc[r] = acc[r] + (((x * l[r * 3] + y * l[r * 3 + 1]) + z * l[r * 3 + 2]) * (D)wt);
					}
				}
				for (int r = 0; r < 3; r++) out_nrm[v * 3 + r] = F(acc[r]);
			}
		}
		for (size_t v = 0; v < n; v++)               /* 0x10029880 */
		{
			D x = out_nrm[v * 3], y = out_nrm[v * 3 + 1], z = out_nrm[v * 3 + 2];
			D inv = 1.0 / sqrt((y * y + x * x) + z * z);
			out_nrm[v * 3] = F(inv * x); out_nrm[v * 3 + 1] = F(inv * y); out_nrm[v * 3 + 2] = F(inv * z);
		}
	}
}

static void set_stream(granny_stream &s, u32 present, u32 type, u32 count, const void *data)
{
	s.present = present; s.type = type; s.dim = 3; s.count = count; s.unknown = 0; s.data = present ? data : 0;
}

} /* namespace */

/* ----------------------------------------------------------------------------------- the API --- */

extern "C" {

void GrannyCompatSetDataDirectory(const char *dir) { g_datadir = dir ? dir : ""; }

granny_result GrannyGetLastError(void) { return g_last_error; }

granny_result GrannyOpenVersion(const char *version, const char *platform, const char *date,
                                const char *copyright, granny_uint32 *out_version)
{
	(void)platform; (void)date; (void)copyright;
	if (!out_version) return ret(GrannyInvalidPointer);
	if (!version || strcmp(version, "1.2b")) return ret(GrannyMismatchedVersions);
	if (g_open) GrannyClose(VERSION_HANDLE);
	g_open = true;
	g_next_id = VERSION_HANDLE + 1;
	g_clock = 0.0;
	g_frame = 1;
	*out_version = VERSION_HANDLE;
	return ret(GrannyOK);
}

granny_result GrannyAllowTextureFormat(granny_uint32 version, granny_uint32 format)
{
	if (!g_open || version != VERSION_HANDLE) return ret(GrannyInvalidHandle);
	return ret(format == 4 ? GrannyOK : GrannyOperationNotSupported);
}

granny_result GrannyOpenModel(granny_uint32 version, const char *file, granny_uint32 out_model[2])
{
	if (!g_open || version != VERSION_HANDLE) return ret(GrannyInvalidHandle);
	if (!out_model) return ret(GrannyInvalidPointer);
	out_model[0] = version;     /* the DLL fills in the first word even when the open fails */
	if (!file) return ret(GrannyInvalidString);
	granny_result err = GrannyOK;
	GrcFile *g = load_grc(file, &err);
	if (!g) return ret(err);
	if (!g->has_model) { release_grc(g); return ret(GrannyUnableToFindData); }
	Model *m = new Model;
	m->id = g_next_id++;
	m->file = g;
	m->refs = 0;
	m->closed = false;
	for (u32 i = 0; i < g->textures.size(); i++)
	{
		TextureInst *t = new TextureInst;
		t->model = m; t->index = i; t->cookie = 0; t->delivered = false;
		m->textures.push_back(t);
		g_new_textures.push_back(t);
	}
	g_models.push_back(m);
	out_model[0] = VERSION_HANDLE; out_model[1] = m->id;
	return ret(GrannyOK);
}

granny_result GrannyOpenAnimation(granny_uint32 version, const char *file, granny_uint32 out_animation[2])
{
	if (!g_open || version != VERSION_HANDLE) return ret(GrannyInvalidHandle);
	if (!out_animation) return ret(GrannyInvalidPointer);
	out_animation[0] = version;
	if (!file) return ret(GrannyInvalidString);
	granny_result err = GrannyOK;
	GrcFile *g = load_grc(file, &err);
	if (!g) return ret(err);
	if (!g->has_anim) { release_grc(g); return ret(GrannyUnableToFindData); }
	Animation *a = new Animation;
	a->id = g_next_id++;
	a->file = g;
	a->refs = 0;
	a->closed = false;
	g_anims.push_back(a);
	out_animation[0] = VERSION_HANDLE; out_animation[1] = a->id;
	return ret(GrannyOK);
}

granny_result GrannyOpenSequence(granny_uint32 m0, granny_uint32 m1, granny_uint32 out_sequence[2])
{
	if (!g_open || m0 != VERSION_HANDLE) return ret(GrannyInvalidHandle);
	if (!out_sequence) return ret(GrannyInvalidPointer);
	out_sequence[0] = m0;
	Model *m = find_model(m0, m1);
	if (!m) return ret(GrannyInvalidHandle);
	Sequence *s = new Sequence;
	s->id = g_next_id++;
	s->model = m;
	m->refs++;
	size_t nb = m->file->bones.size();
	s->local.resize(nb);
	s->world.resize(nb);
	for (size_t i = 0; i < nb; i++) s->local[i] = m->file->bones[i].rest;
	local_identity(s->placement);
	s->parent = 0; s->parent_bone = 0; s->enabled = true; s->frame = 0;
	seq_compute_all(s);
	g_seqs.push_back(s);
	out_sequence[0] = VERSION_HANDLE; out_sequence[1] = s->id;
	return ret(GrannyOK);
}

granny_result GrannyAddControlledAnimation(granny_uint32 s0, granny_uint32 s1, granny_uint32 a0, granny_uint32 a1,
                                           granny_uint32 out_control[3])
{
	/* checking order of 0x10006530 / 0x10011220 */
	if (!g_open || s0 != VERSION_HANDLE) return ret(GrannyInvalidHandle);
	if (a0 != s0) return ret(GrannyMismatchedHandles);
	if (!out_control) return ret(GrannyInvalidPointer);
	out_control[0] = s0;
	Sequence *s = find_seq(s0, s1);
	Animation *a = find_anim(a0, a1);
	if (!s || !a) return ret(GrannyInvalidHandle);
	Control *c = new Control;
	c->id = g_next_id++;
	c->seq = s; c->anim = a;
	a->refs++;
	const GrcFile *mf = seq_file(s);
	const std::vector<Track> &tracks = a->file->tracks;
	c->track_bone.assign(tracks.size(), -1);
	for (size_t k = 1; k < tracks.size(); k++)
		for (size_t i = 0; i < mf->bones.size(); i++)
			if (tracks[k].name == mf->bones[i].name) { c->track_bone[k] = (int)i; break; }
	if (!tracks.empty()) c->track_bone[0] = 0;
	c->start = g_clock; c->speed = 1.0; c->loops = 1.0; c->blendin = 0.0; c->blendout = 0.0;
	c->frozen_local = 0.0; c->duration = a->file->duration;
	c->w_hold = 1.0f; c->w_before = 0.0f; c->w_after = 0.0f; c->priority = 0.0f;
	c->active = true; c->released = false;
	g_controls.push_back(c);
	out_control[0] = VERSION_HANDLE; out_control[1] = s->id; out_control[2] = c->id;
	return ret(GrannyOK);
}

granny_result GrannySetAnimationParameter(granny_uint32 c0, granny_uint32 c1, granny_uint32 c2,
                                          granny_uint32 param, double v)
{
	Control *c = find_ctrl(c0, c1, c2);
	if (!c) return ret(GrannyInvalidHandle);
	switch (param)
	{
	case 0: c->start = v; break;
	case 1: c->start = (v - ctrl_end(c)) + c->start; break;
	case 2:                                                  /* 0x10011e2e: keep the current phase */
	{
		D old_speed = c->speed, old_start = c->start, old_total = ctrl_total(c), now = g_clock;
		c->speed = v;
		bool rescale = old_total > 1e-05 && old_speed > 1e-05;
		if (v > 1e-05)
		{
			D total = ctrl_total(c);
			if (rescale) c->start = (now - old_start) / old_total * (old_total - total) + old_start;
			else c->start = c->start - ((c->frozen_local / c->speed + c->start) - now);
		}
		break;
	}
	case 3: c->loops = v; break;
	case 4: c->w_hold = (float)v; break;
	case 5: c->w_before = (float)v; break;
	case 6: c->blendin = v; break;
	case 7: c->w_after = (float)v; break;
	case 8: c->blendout = v; break;
	default: return ret(GrannyInvalidIndex);
	}
	return ret(GrannyOK);
}

granny_result GrannyGetAnimationParameter(granny_uint32 c0, granny_uint32 c1, granny_uint32 c2,
                                          granny_uint32 param, double *v)
{
	Control *c = find_ctrl(c0, c1, c2);
	if (!c) return ret(GrannyInvalidHandle);
	if (!v) return ret(GrannyInvalidPointer);
	switch (param)
	{
	case 0: *v = c->start; break;
	case 1: *v = ctrl_end(c); break;
	case 2: *v = c->speed; break;
	case 3: *v = c->loops; break;
	case 4: *v = c->w_hold; break;
	case 5: *v = c->w_before; break;
	case 6: *v = c->blendin; break;
	case 7: *v = c->w_after; break;
	case 8: *v = c->blendout; break;
	default: return ret(GrannyInvalidIndex);
	}
	return ret(GrannyOK);
}

granny_result GrannyHasControlExpired(granny_uint32 c0, granny_uint32 c1, granny_uint32 c2, granny_uint32 *expired)
{
	Control *c = find_ctrl(c0, c1, c2);
	if (!c) return ret(GrannyInvalidHandle);
	if (!expired) return ret(GrannyInvalidPointer);
	*expired = g_clock > ctrl_end(c) ? 1 : 0;
	return ret(GrannyOK);
}

granny_result GrannyRemoveControlledAnimation(granny_uint32 c0, granny_uint32 c1, granny_uint32 c2)
{
	Control *c = find_ctrl(c0, c1, c2);
	if (!c) return ret(GrannyInvalidHandle);
	destroy_control(c);
	return ret(GrannyOK);
}

granny_result GrannyReleaseControlledAnimation(granny_uint32 c0, granny_uint32 c1, granny_uint32 c2)
{
	Control *c = find_ctrl(c0, c1, c2);
	if (!c) return ret(GrannyInvalidHandle);
	c->released = true;          /* plays on, deleted once it is no longer alive */
	return ret(GrannyOK);
}

granny_result GrannyAdvanceTime(granny_uint32 version, double seconds)
{
	if (!g_open || version != VERSION_HANDLE) return ret(GrannyInvalidHandle);
	D old_t = g_clock;
	g_clock = old_t + (D)(float)seconds;                     /* the core takes a float */
	std::vector<Control *> list(g_controls);                 /* 0x1001b830 */
	for (size_t i = 0; i < list.size(); i++)
	{
		Control *c = list[i];
		if (ctrl_alive(c, old_t)) ctrl_update(c, old_t, g_clock);
		if (!ctrl_alive(c, g_clock) && c->released) destroy_control(c);
	}
	g_frame++;                                               /* 0x1001b3c0 */
	for (size_t i = 0; i < g_seqs.size(); i++) seq_update(g_seqs[i]);
	return ret(GrannyOK);
}

granny_result GrannyGetCurrentTime(granny_uint32 version, double *seconds)
{
	if (!g_open || version != VERSION_HANDLE) return ret(GrannyInvalidHandle);
	if (!seconds) return ret(GrannyInvalidPointer);
	*seconds = g_clock;
	return ret(GrannyOK);
}

granny_result GrannyLockSequenceForRendering(granny_uint32 s0, granny_uint32 s1, granny_uint32 present,
                                             granny_uint32 out_lock[2])
{
	Sequence *s = find_seq(s0, s1);
	if (!s) return ret(GrannyInvalidHandle);
	if (present & ~0x3fu) return ret(GrannyInvalidFlags);
	if (!out_lock) return ret(GrannyInvalidPointer);
	if (g_lock) return ret(GrannyInvalidCall);
	Lock *l = new Lock;
	l->seq = s; l->present = present; l->next = 0; l->state_locked = false;
	const GrcFile *f = seq_file(s);
	for (u32 m = 0; m < f->meshes.size(); m++)
		for (u32 k = 0; k < f->meshes[m].states.size(); k++) l->states.push_back(std::make_pair(m, k));
	l->pos.resize(f->meshes.size()); l->nrm.resize(f->meshes.size()); l->done.assign(f->meshes.size(), false);
	g_lock = l;
	out_lock[0] = VERSION_HANDLE; out_lock[1] = s->id;
	return ret(GrannyOK);
}

/* a lock handle is (version, sequence id); there is at most one lock at a time (0x10012a70) */
static granny_result check_lock(u32 l0, u32 l1)
{
	if (!find_seq(l0, l1)) return GrannyInvalidHandle;
	if (!g_lock) return GrannyInvalidCall;
	if (g_lock->seq->id != l1) return GrannyMismatchedHandles;
	return GrannyOK;
}

granny_result GrannyGetRenderingStatesLeft(granny_uint32 l0, granny_uint32 l1, granny_uint32 *count)
{
	if (!g_open || l0 != VERSION_HANDLE) return ret(GrannyInvalidHandle);
	if (!count) return ret(GrannyInvalidPointer);
	granny_result e = check_lock(l0, l1);
	if (e) return ret(e);
	*count = (u32)g_lock->states.size() - g_lock->next;
	return ret(GrannyOK);
}

granny_result GrannyLockNextRenderingState(granny_uint32 l0, granny_uint32 l1, granny_rendering_state *st)
{
	if (!g_open || l0 != VERSION_HANDLE) return ret(GrannyInvalidHandle);
	if (!st) return ret(GrannyInvalidPointer);
	granny_result e = check_lock(l0, l1);
	if (e) return ret(e);
	Lock *l = g_lock;
	if (l->state_locked || l->next >= l->states.size()) return ret(GrannyInvalidCall);
	u32 mi = l->states[l->next].first, si = l->states[l->next].second;
	l->next++;
	l->state_locked = true;
	const GrcFile *f = seq_file(l->seq);
	const Mesh &m = f->meshes[mi];
	const MeshState &ms = m.states[si];
	bool want_pos = (l->present & GrannyStreamPositions) != 0, want_nrm = (l->present & GrannyStreamNormals) != 0;
	if (!l->done[mi])
	{
		skin_mesh(l->seq, m, want_pos, want_nrm, l->pos[mi], l->nrm[mi]);
		l->done[mi] = true;
	}
	memset(st, 0, sizeof *st);
	st->lock[0] = l0; st->lock[1] = l1;
	st->sequence[0] = VERSION_HANDLE; st->sequence[1] = l->seq->id;
	st->present = l->present;
	{
		const TextureInst *t = ms.texture < l->seq->model->textures.size() ? l->seq->model->textures[ms.texture] : 0;
		st->texture_pending = t && t->delivered ? 0 : 1;
		st->texture_cookie = t && t->delivered ? t->cookie : 0;
	}
	st->has_transform = 1;
	st->transform = g_identity4;
	st->deformed = 1;
	st->mesh_cookie = 0;
	set_stream(st->positions, want_pos ? 1u : 0u, 6, m.nrender, l->pos[mi].empty() ? 0 : &l->pos[mi][0]);
	set_stream(st->color0, 1, 6, m.nrender, m.col.empty() ? 0 : &m.col[0]);
	set_stream(st->color1, (l->present & GrannyStreamColor1) ? 1u : 0u, 6, m.nrender, m.col.empty() ? 0 : &m.col[0]);
	set_stream(st->texcoords, (l->present & GrannyStreamTexCoords) ? 1u : 0u, 6, m.nrender, m.uv.empty() ? 0 : &m.uv[0]);
	set_stream(st->normals, want_nrm ? 1u : 0u, 6, m.nrender, l->nrm[mi].empty() ? 0 : &l->nrm[mi][0]);
	set_stream(st->triangles, (l->present & GrannyStreamTriangles) ? 1u : 0u, 3, (u32)(ms.tris.size() / 3),
	           ms.tris.empty() ? 0 : &ms.tris[0]);
	st->src_blend = ms.src_blend; st->dst_blend = ms.dst_blend;
	st->internal = l;
	return ret(GrannyOK);
}

granny_result GrannyUnlockRenderingState(granny_rendering_state *st)
{
	if (!st) return ret(GrannyInvalidPointer);
	if (!g_lock || st->internal != g_lock || !g_lock->state_locked) return ret(GrannyInvalidCall);
	g_lock->state_locked = false;
	st->internal = 0;
	return ret(GrannyOK);
}

granny_result GrannyUnlockRendering(granny_uint32 l0, granny_uint32 l1)
{
	if (!g_open || l0 != VERSION_HANDLE) return ret(GrannyInvalidHandle);
	granny_result e = check_lock(l0, l1);           /* 0x10012950 */
	if (e) return ret(e);
	if (g_lock->state_locked) return ret(GrannyInvalidCall);
	delete g_lock;
	g_lock = 0;
	return ret(GrannyOK);
}

granny_result GrannyGetNewTexturesLeft(granny_uint32 version, granny_uint32 *count)
{
	if (!g_open || version != VERSION_HANDLE) return ret(GrannyInvalidHandle);
	if (!count) return ret(GrannyInvalidPointer);
	*count = (u32)g_new_textures.size();
	return ret(GrannyOK);
}

granny_result GrannyLockNextNewTexture(granny_uint32 version, granny_new_texture *t)
{
	if (!g_open || version != VERSION_HANDLE) return ret(GrannyInvalidHandle);
	if (!t) return ret(GrannyInvalidPointer);
	if (g_new_textures.empty()) return ret(GrannyInvalidCall);
	TextureInst *ti = g_new_textures.front();
	g_new_textures.erase(g_new_textures.begin());
	const TextureData &d = ti->model->file->textures[ti->index];
	memset(t, 0, sizeof *t);
	t->version = VERSION_HANDLE;
	t->format = d.format; t->width = d.w; t->height = d.h; t->bytes_per_pixel = d.bpp; t->pitch = d.pitch;
	t->size = (u32)d.pixels.size();
	t->pixels = d.pixels.empty() ? 0 : &d.pixels[0];
	t->internal = ti;
	return ret(GrannyOK);
}

granny_result GrannyUnlockNewTexture(granny_new_texture *t, granny_cookie cookie)
{
	if (!t || !t->internal) return ret(GrannyInvalidPointer);
	TextureInst *ti = (TextureInst *)t->internal;
	bool live = false;
	for (size_t i = 0; i < g_models.size() && !live; i++)
		for (size_t k = 0; k < g_models[i]->textures.size(); k++) if (g_models[i]->textures[k] == ti) live = true;
	if (!live) return ret(GrannyInvalidPointer);
	ti->cookie = cookie;
	ti->delivered = true;
	t->internal = 0;
	return ret(GrannyOK);
}

granny_result GrannyGetBoneCount(granny_uint32 s0, granny_uint32 s1, granny_uint32 *count)
{
	Sequence *s = find_seq(s0, s1);
	if (!s) return ret(GrannyInvalidHandle);
	if (!count) return ret(GrannyInvalidPointer);
	*count = (u32)s->local.size();
	return ret(GrannyOK);
}

granny_result GrannyGetBoneIndexFromName(granny_uint32 s0, granny_uint32 s1, const char *name, granny_uint32 *index)
{
	Sequence *s = find_seq(s0, s1);
	if (!s) return ret(GrannyInvalidHandle);
	if (!index) return ret(GrannyInvalidPointer);
	*index = 0;
	if (!name) return ret(GrannyInvalidString);
	const GrcFile *f = seq_file(s);
	for (u32 i = 0; i < f->bones.size(); i++)
		if (f->bones[i].name == name) { *index = i; return ret(GrannyOK); }
	return ret(GrannyInvalidString);
}

granny_result GrannyGetBoneState(granny_uint32 s0, granny_uint32 s1, granny_uint32 bone, granny_uint32 present,
                                 granny_transform_state *st)
{
	Sequence *s = find_seq(s0, s1);
	if (!s) return ret(GrannyInvalidHandle);
	if (present & ~0x1ffu) return ret(GrannyInvalidFlags);
	if (bone >= s->local.size()) return ret(GrannyInvalidIndex);
	if (!st) return ret(GrannyInvalidPointer);
	memset(st, 0, sizeof *st);
	st->sequence[0] = s0; st->sequence[1] = s1; st->bone = bone; st->present = present;
	st->parent = bone ? (u32)seq_file(s)->bones[bone].parent : 0;
	/* the DLL (0x100166c0) reports the local transform and writes its 3x3 matrices transposed
	   (column-major); combined = rotation * scale/shear (0x100183e0) */
	const Local &l = s->local[bone];
	float R[9], C[9];
	rotmat(l.q, R);
	for (int r = 0; r < 3; r++)
		for (int c = 0; c < 3; c++)
			C[r * 3 + c] = F(((D)R[r * 3] * l.ss[c] + (D)R[r * 3 + 1] * l.ss[3 + c]) + (D)R[r * 3 + 2] * l.ss[6 + c]);
	memcpy(st->position, l.pos, 12);
	memcpy(st->quaternion, l.q, 16);
	for (int r = 0; r < 3; r++)
		for (int c = 0; c < 3; c++)
		{
			st->rotation[c * 3 + r] = R[r * 3 + c];
			st->scale_shear[c * 3 + r] = l.ss[r * 3 + c];
			st->combined[c * 3 + r] = C[r * 3 + c];
		}
	return ret(GrannyOK);
}

granny_result GrannyAttachSequence(granny_uint32 s0, granny_uint32 s1, granny_uint32 bone,
                                   granny_uint32 p0, granny_uint32 p1, granny_uint32 parent_bone,
                                   granny_uint32 dof0, granny_uint32 dof1)
{
	/* checking order of 0x10009fb0 / 0x10010bd0: an attached child is detached before the bone
	   indices and the attachment itself are checked, so a failed re-attach leaves it detached.
	   The degrees of freedom are only range-checked: the game always passes 0x1ff, 0x1ff */
	if (!g_open || s0 != VERSION_HANDLE) return ret(GrannyInvalidHandle);
	if (p0 != s0) return ret(GrannyMismatchedHandles);
	if ((dof0 | dof1) & ~0x1ffu) return ret(GrannyInvalidFlags);
	Sequence *s = find_seq(s0, s1);
	Sequence *p = find_seq(p0, p1);
	if (!s || !p) return ret(GrannyInvalidHandle);
	if (s->parent) seq_detach(s);
	if (bone >= s->local.size() || parent_bone >= p->local.size()) return ret(GrannyInvalidIndex);
	for (Sequence *q = p; q; q = q->parent) if (q == s) return ret(GrannyOperationFailure);
	/* 0x1001c140: the root becomes the inverse of the attach bone's transform within the sequence */
	Local inv;
	if (bone == 0)
	{
		local_identity(inv);
		inv.pos[0] = inv.pos[1] = inv.pos[2] = -0.0f;
		inv.q[0] = inv.q[1] = inv.q[2] = -0.0f;
	}
	else
	{
		local_identity(s->local[0]);
		Local l;
		flatten(s, bone, l);
		local_inverse(l, inv);
	}
	s->local[0] = inv;
	s->placement = inv;
	s->parent = p;
	s->parent_bone = parent_bone;
	return ret(GrannyOK);
}

granny_result GrannyDetachSequence(granny_uint32 s0, granny_uint32 s1)
{
	Sequence *s = find_seq(s0, s1);
	if (!s) return ret(GrannyInvalidHandle);
	if (!s->parent) return ret(GrannyInvalidCall);
	seq_detach(s);
	return ret(GrannyOK);
}

granny_result GrannyCloseSequence(granny_uint32 s0, granny_uint32 s1)
{
	Sequence *s = find_seq(s0, s1);
	if (!s) return ret(GrannyInvalidHandle);
	if (g_lock && g_lock->seq == s) return ret(GrannyInvalidCall);
	for (size_t i = 0; i < g_seqs.size(); i++) if (g_seqs[i]->parent == s) seq_detach(g_seqs[i]);
	for (size_t i = g_controls.size(); i-- > 0;) if (g_controls[i]->seq == s) destroy_control(g_controls[i]);
	for (size_t i = 0; i < g_seqs.size(); i++) if (g_seqs[i] == s) { g_seqs.erase(g_seqs.begin() + (long)i); break; }
	Model *m = s->model;
	delete s;
	if (--m->refs == 0 && m->closed) free_model(m);
	return ret(GrannyOK);
}

granny_result GrannyCloseAnimation(granny_uint32 a0, granny_uint32 a1)
{
	Animation *a = find_anim(a0, a1);
	if (!a) return ret(GrannyInvalidHandle);
	a->closed = true;                   /* live controls keep playing it */
	if (a->refs == 0) free_anim(a);
	return ret(GrannyOK);
}

granny_result GrannyCloseModel(granny_uint32 m0, granny_uint32 m1)
{
	Model *m = find_model(m0, m1);
	if (!m) return ret(GrannyInvalidHandle);
	m->closed = true;                   /* open sequences (and their textures) stay usable */
	if (m->refs == 0) free_model(m);
	return ret(GrannyOK);
}

granny_result GrannyClose(granny_uint32 version)
{
	if (!g_open || version != VERSION_HANDLE) return ret(GrannyInvalidHandle);
	delete g_lock; g_lock = 0;
	while (!g_controls.empty()) destroy_control(g_controls.back());
	while (!g_seqs.empty()) { delete g_seqs.back(); g_seqs.pop_back(); }
	while (!g_anims.empty()) { release_grc(g_anims.back()->file); delete g_anims.back(); g_anims.pop_back(); }
	while (!g_models.empty())
	{
		Model *m = g_models.back();
		for (size_t k = 0; k < m->textures.size(); k++) delete m->textures[k];
		release_grc(m->file); delete m; g_models.pop_back();
	}
	g_new_textures.clear();
	g_open = false;
	return ret(GrannyOK);
}

} /* extern "C" */
