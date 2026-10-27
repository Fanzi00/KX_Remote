#ifndef DIFF_REFRESH_H
#define DIFF_REFRESH_H

// ===================== 差分刷新引擎（屏显节流：只发变化字段，抑制微跳动）=====================
#define DIFF_FLD_MAX      24     // 单页最多缓存的字段数量
#define DIFF_TXT_LEN      16     // 文本字段最大缓存长度（字节）
#define DIFF_FORCE_TIMES  2      // 进页面整页全量次数：低频率下连刷 N 次，补齐"单帧画不全"的内容

// 引擎状态（每个屏一个，由调用方持有）
typedef struct {
    long               v;             // 上次下发的数值
    char               t[DIFF_TXT_LEN]; // 上次下发的文本
    unsigned char      init;          // 0=未初始化（首帧必发）
} DiffSlot;

typedef struct {
    DiffSlot      slot[DIFF_FLD_MAX];
    unsigned char force;              // 整页强制剩余次数：>0 时所有字段强制下发（无视缓存）
} DiffCtx;

// 进页面时调用一次：请求接下来 DIFF_FORCE_TIMES 次整页全量
void Diff_PageEnter(DiffCtx* c);
// 数值字段差分发送：v 与缓存不同(或整页强制/未初始化)才下发；thr>0 时仅当 |Δ|>=thr 才下发（抑制微跳动）
void Diff_Int(DiffCtx* c, unsigned char id, unsigned short x, unsigned short y, long v, unsigned char digits, unsigned char font, long thr);
// 文本字段差分发送：字符串与缓存不同(或整页强制/未初始化)才下发
void Diff_Txt(DiffCtx* c, unsigned char id, unsigned short x, unsigned short y, const char* s, unsigned char font);
// 每帧绘制结束后调用一次：递减整页强制计数
void Diff_EndFrame(DiffCtx* c);

#endif // DIFF_REFRESH_H
