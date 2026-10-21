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

// ADC 线性映射：adc ∈ [adc_min, adc_max] -> out ∈ [out_min, out_max]
// 支持反向映射(out_min > out_max)；超量程自动钳位；除零保护；int64 防乘积溢出。
// 参数顺序与 Analog_Remap 保持一致：out_min 在 out_max 之前。
uint16_t adc_map_int(uint32_t adc, uint16_t adc_min, uint16_t adc_max,
                     uint16_t out_min, uint16_t out_max)
{
    int32_t span_adc, span_out, adc_s, out;

    if (adc_max == adc_min) return out_min;          /* 除零保护 */

    span_adc = (int32_t)adc_max - (int32_t)adc_min;
    span_out = (int32_t)out_max - (int32_t)out_min;

    /* 输入钳位到量程内（同时兼容 adc_min > adc_max 的反向量程）*/
    if (adc < adc_min) adc = adc_min;
    if (adc > adc_max) adc = adc_max;
    adc_s = (int32_t)adc - (int32_t)adc_min;

    /* 先乘(int64 防溢出)再四舍五入后除，结果落在 [out_min, out_max] 附近 */
    out = out_min + (int32_t)(((int64_t)adc_s * span_out + (span_adc >> 1)) / span_adc);

    /* 钳位到 uint16 合法范围 */
    if (out < 0)      out = 0;
    if (out > 65535)  out = 65535;

    return (uint16_t)out;
}

// DI 数字量映射：in > 阈值(500) 时置位 v1/v2 的第 num 位，否则清位
// 保护：num 须 < 32（移位越界为 UB，直接返回）；v1/v2 可为 NULL（忽略该目标）
void di_map_int(uint32_t in, unsigned long* v1, unsigned long* v2, uint8_t num)
{
    unsigned long bit;

    if (num >= 32U) return;                 /* 移位越界保护 */
    bit = 1UL << num;

    if (in > 500)
    {
        if (v1) *v1 |= bit;
        if (v2) *v2 |= bit;
    }
    else
    {
        if (v1) *v1 &= ~bit;
        if (v2) *v2 &= ~bit;
    }
}

__weak void Straight_handle(void)
{}

__weak void ADC_Correction(void)
{}

__weak void DI_Correction(void)
{}