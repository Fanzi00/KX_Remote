#include "DiffRefresh.h"
#include "App_DGUS.h" // LCD_Disp_Intx / LCD_Disp_Txt（差分引擎依赖）
#include <string.h>

// ===================== 差分刷新引擎 =====================
// 调用方各自持有一个 DiffCtx（每个屏一个）；进页面 Diff_PageEnter() 请求整页全量，
// 之后每帧只发变化字段；数值末参 thr>0 时仅 |Δ|>=thr 才发，抑制数据微跳动造成的无用刷新。
/*******************************************************************
 * Diff_PageEnter - 进入新页面时调用一次，请求接下来整页全量刷新
 *
 * 作用：把 ctx->force 置为 DIFF_FORCE_TIMES（默认 2）。
 *       在随后的 DIFF_FORCE_TIMES 帧内，所有字段（无论是否变化）都
 *       会被强制下发，用来补齐"单帧画不全 / 首帧内容不全"的情况；
 *       之后自动回落为仅下发变化字段，实现屏显节流。
 *
 * 参数：
 *   c  [in/out] 差分引擎上下文指针（每个屏/页面一个，由调用方持有）。
 *               必须指向已分配、已初始化的 DiffCtx 变量。
 *******************************************************************/
void Diff_PageEnter(DiffCtx* c) { c->force = DIFF_FORCE_TIMES; }

/*******************************************************************
 * Diff_Int - 数值字段差分发送（带变化检测 + 抖动抑制）
 *
 * 作用：仅当字段需要更新时才经屏串口下发，避免每帧重复刷整屏，
 *       从而省屏串口带宽、减少对无线（433M）的抢占。
 *
 * 参数：
 *   c       [in/out] 差分引擎上下文指针（每个屏一个）。
 *   id      [in]     字段编号（0..DIFF_FLD_MAX-1），同一页面内唯一，
 *                    用于索引缓存槽位；超出范围直接忽略本次调用。
 *   x,y     [in]     该数值在屏上的显示坐标（像素，左上角）。
 *   v       [in]     本次要显示的数值（应已映射好的最终显示值）。
 *   digits  [in]     显示位数（传给 LCD_Disp_Intx，控制补零/对齐）。
 *   font    [in]     字号/字体编号（传给 LCD_Disp_Intx）。
 *   thr     [in]     抖动抑制阈值（可选）：
 *                     thr<=0 : 仅当 v 与上次下发值不同才发；
 *                     thr>0  : 仅当 |v - 上次值| >= thr 才发，
 *                              用于抑制传感器数据微跳动导致的无用刷新。
 *******************************************************************/
void Diff_Int(DiffCtx* c, unsigned char id, unsigned short x, unsigned short y, long v, unsigned char digits, unsigned char font, long thr)
{
	DiffSlot* s;
	long      d;
	if(id >= DIFF_FLD_MAX)
		return;
	s = &c->slot[id];
	d = (v > s->v) ? (v - s->v) : (s->v - v);
	if(c->force || !s->init || (thr > 0 ? (d >= thr) : (v != s->v)))
	{
		LCD_Disp_Intx(font, x, y, v, digits);
		s->v = v;
		s->init = 1;
	}
}

/*******************************************************************
 * Diff_Txt - 文本字段差分发送（带变化检测）
 *
 * 作用：与 Diff_Int 类似，但针对字符串（如状态、单位、报警文本）。
 *       仅当字符串内容与上次下发不同才下发，避免重复刷屏。
 *
 * 参数：
 *   c    [in/out] 差分引擎上下文指针（每个屏一个）。
 *   id   [in]     字段编号（0..DIFF_FLD_MAX-1），同一页面内唯一。
 *   x,y  [in]     文本在屏上的显示坐标（像素，左上角）。
 *   s    [in]     本次要显示的字符串（以 '\0' 结尾）。
 *   font [in]     字号/字体编号（传给 LCD_Disp_Txt）。
 *******************************************************************/
void Diff_Txt(DiffCtx* c, unsigned char id, unsigned short x, unsigned short y, const char* s, unsigned char font)
{
	DiffSlot* sl;
	if(id >= DIFF_FLD_MAX)
		return;
	sl = &c->slot[id];
	if(c->force || !sl->init || (strncmp(sl->t, s, DIFF_TXT_LEN) != 0))
	{
		LCD_Disp_Txt(font, x, y, (unsigned char*)s);
		strncpy(sl->t, s, DIFF_TXT_LEN - 1);
		sl->t[DIFF_TXT_LEN - 1] = 0;
		sl->init = 1;
	}
}

/*******************************************************************
 * Diff_EndFrame - 每帧绘制结束后调用一次，递减整页强制计数
 *
 * 作用：与 Diff_PageEnter 配合。每帧末尾调用，把 ctx->force 减 1
 *       （不为负）。进入页面后的前 DIFF_FORCE_TIMES 帧内 force>0，
 *       字段会被强制全量下发；减到 0 后恢复为仅发变化字段。
 *
 * 参数：
 *   c  [in/out] 差分引擎上下文指针（每个屏一个）。
 *******************************************************************/
void Diff_EndFrame(DiffCtx* c)
{
	if(c->force > 0)
		c->force--;
}
