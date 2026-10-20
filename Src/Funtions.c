#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* ================================================================
 * 分段线性重映射（uint16_t 全参数版 + 模拟量输入为指针）
 *
 *  *in      模拟量输入指针（只读，NULL 时按 in_min 处理，避免死机）
 *  in_min   模拟量最小值      in_mid 模拟量中间值    in_max 模拟量最大值
 *  in_dead  中间值死区半宽（死区 = [in_mid-in_dead, in_mid+in_dead]）
 *  out_min  映射最小值        out_mid 映射中间值     out_max 映射最大值
 *  返回     映射结果（uint16_t）
 *
 *  映射关系：
 *    in ∈ [in_min, in_mid-in_dead)  ->  out ∈ [out_min, out_mid]
 *    in ∈ [in_mid-in_dead, in_mid+in_dead] -> out = out_mid   (死区)
 *    in ∈ (in_mid+in_dead, in_max]  ->  out ∈ [out_mid, out_max]
 *    超出量程 -> 钳位到 out_min / out_max
 * ================================================================ */

/* 线性插值 + 四舍五入，支持 out_min > out_max 的反向映射 */
static int32_t interp_u16(uint32_t num, uint32_t den, uint16_t a, uint16_t b)
{
    uint32_t abs_d, prod, mag;

    if (den == 0U) return (int32_t)b;            /* 除零保护：直接给终点 */
    if (num >= den) return (int32_t)b;           /* 跑完该段：给终点     */

    abs_d = (b >= a) ? (uint32_t)(b - a) : (uint32_t)(a - b);
    prod  = num * abs_d;                         /* ≤ 65535*65535，uint32 装得下 */
    mag   = (prod + den / 2U) / den;             /* 四舍五入              */

    return (b >= a) ? (int32_t)a + (int32_t)mag
                    : (int32_t)a - (int32_t)mag;
}

uint16_t Analog_Remap(const uint32_t *in,
                      uint16_t in_min, uint16_t in_mid, uint16_t in_max,
                      uint16_t in_dead,
                      uint16_t out_min, uint16_t out_mid, uint16_t out_max)
{
    uint32_t x, lo, hi, lo32, hi32;
    int32_t  v;

    /* ---- 1. 取输入：指针为空时按最小值处理 ---- */
    x = (in != NULL) ? (uint32_t)*in : (uint32_t)in_min;

    /* ---- 2. 参数规整 ---- */
    if (in_min > in_max) { uint16_t s = in_min; in_min = in_max; in_max = s; }
    if (in_mid < in_min) in_mid = in_min;
    if (in_mid > in_max) in_mid = in_max;

    /* ---- 3. 死区边界（用 uint32 算，防止 uint16 下溢/上溢）---- */
    lo32 = ((uint32_t)in_mid > in_dead) ? ((uint32_t)in_mid - in_dead) : 0U;
    hi32 = (uint32_t)in_mid + in_dead;
    if (lo32 < in_min) lo32 = in_min;                 /* 下边界不越量程 */
    if (hi32 > in_max) hi32 = in_max;                 /* 上边界不越量程 */
    lo = lo32; hi = hi32;

    /* ---- 4. 超量程钳位 ---- */
    if (x <= in_min) return out_min;
    if (x >= in_max) return out_max;

    /* ---- 5. 死区：直接给中间值 ---- */
    if (x >= lo && x <= hi) return out_mid;

    /* ---- 6. 分段插值 ---- */
    if (x < lo)  v = interp_u16(x - in_min, lo - in_min,          out_min, out_mid);
    else         v = interp_u16(x - hi,     in_max - hi,          out_mid, out_max);

    /* ---- 7. 结果钳位到 uint16 合法范围 ---- */
    if (v < 0)      return 0U;
    if (v > 65535)  return 65535U;
    return (uint16_t)v;
}
