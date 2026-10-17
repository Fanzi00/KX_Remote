# AGENTS.md — FSJ_666 问题解决记录

## 1. 遥控器自检功能需要用宏开关控制（开启/关闭）

**问题**
自检逻辑（`SelfChk_main` / `SelfChk_Proc` / `SelfChk_Str` / `showWarn` / `selfDisplay`）散落在
`Src/main.c`、`Src/App_usr.c` 和三份遥控器 `User/*.c` 中，且 `main.c` 的 10ms 任务里无条件调用
`SelfChk_main()`，无法通过宏整体关闭。

**解决方法（最简方案，只改 3 处）**
不调用 `SelfChk_main()` 即可：`gSelfChkMask` 恒为初值 0 → `Activar()` 的
`if(gSelfChkMask == 0)` 恒成立 → 按键可直接激活；`selfDisplay()` 因 mask==0 只清屏不报警。
因此只在调用处做条件编译，不必给每个自检函数都套 `#if`（套上也只是省 flash，逻辑等价）。

| 文件 | 改动 |
| --- | --- |
| `User/_Device_config.h` | 新增 `#ifndef _SELF_CHK_EN` / `#define _SELF_CHK_EN 1`（放 `USER_TYPE` 前，机型级覆盖可 `#undef` 后重定义） |
| `Src/main.c` | `#if _SELF_CHK_EN` 包裹 10ms 任务里的 `SelfChk_main()` 调用 |
| `Src/App_usr.c` | `gSelfChkOK` 恒取 1（自检关闭即视为通过） |
| `Src/App_usr.h` | 补 `void SelfChk_main(void);` 声明，消除隐式声明告警 |

**注意**
- `User/*.c` 里的自检代码保留不删，关闭时是死代码（仍占 flash），需要省空间时再给
  `SelfChk_Str`/`SelfChk_Proc`/`SelfChk_main`/`showWarn`/`selfDisplay` 套 `#if _SELF_CHK_EN`。
- `KXCAN_GLZYTC.c` 的自检显示写在 `display()` 内部（不是独立 `selfDisplay()`）。

## 2. 按《自学习与死区_移植改动清单.md》升级自学习/状态页/死区

**问题**
原工程只有旧的 `Arm_Study()`（直接写 `gBDParam[0]`、F1 无校验就保存、F4 也触发保存），
且 `KX_Send_joysend()` 用的是固定曲线 `0xd35/0x7dc/0x1fa`，标定和死区 1~9 改了不生效；
主页单 F3 不能进状态页。清单文件在上级目录 `Source\Project\自学习与死区_移植改动清单.md`。

**移植前核对结果（FSJ_666 全部符合清单第 1 节）**
`_V_2=1`、`_ARM_XY=1`、`_ARM_KX=1`、`_ARM_XRZN=0`、`_ARM_ROCK=4`、`_ARM_USE=8`、`_ARM_MAX=10`、
`_XN_LiuL=0`（见 `User\_Device_config.h`）、`_ARM_MIDV/_ARM_MIDVCan=127`、`LCD_FRESH_MS=200`（`App_lcd.h`）、
`vRockV[20][2]`+`vRockDS[10]`（`sizeof(mBDParam)=94`）、`gArmUpTab==gArmDownTab=={10,6,15,7,12,13,0,5}`、
状态页 PIC7 / 学习页 PIC8。

**改动（6 个文件）**

| 文件 | 改动 |
| --- | --- |
| `Src/CanOpen.h` | 新增 `unsigned char KX_JoyValue(unsigned char vCH);` 声明 |
| `Src/CanOpen.c` | 新增 `KX_JoyValue()`（附录 B，接入 `vRockV` 标定 + 死区 `0.06*(1+档位)`）；`KX_Send_joysend()` 替换为 `Y_Send[vCH] = stStudyFlg ? _ARM_MIDVCan : KX_JoyValue(vCH);` |
| `Src/App_usr.c` | `Arm_Study()` 整函数替换为附录 A：`gBDParam[1]` 暂存、F2 中位 / F3 极限 / F1 校验后提交 / F4 丢弃；`stStudyFlg` 用位 0/1/2（1/3/5/7） |
| `Src/App_IO.c` | `Key_ZZT_CtrlStatus` 里 `Arm_Study();` 后加 `if(stStudyFlg || (gp_lcdtask != Disp_ZZT_CtrlStatus)) return;`。**主页入口保持原样**：仍须 F1+F3 组合键进状态页，单 F3 不进入 |
| `Src/App_lcd.c` | `Disp_ZZT_CtrlStatus()` 局部变量之后、`if(gLcdFresh > 2)` 之前插入 `if(stStudyFlg)` 分支：8 轴用 `KX_JoyValue(i)` 显示后 `return`，保留 PIC8 背景 |
| `Src/App_Comm.c` | `YKQ_Data_WL()` 中位帧条件改为 `if(gJiTing \|\| stStudyFlg \|\| (gLaBaFlg==0))` |

**注意 / 易踩的点**
- 数据链路：`ADCRst → app10ms → KX_Send_joysend → Y_Send → KX_Send_joylogic → VKC_Send → YKQ_Data_WL`
  （`Can_Send_Prog` 也用 `Y_Send`）。**只改 `Arm_Conv/gADArmSend` 没用**，那条不是 KX 无线手柄输出。
- `Arm_Study` 用 `gDIBitV0` 的 bit16~19 取原始按键电平做长按计时；`gKeyValue` 是边沿锁存值，不能用来计时。
- `Sys_Write_BD()` 内部会 `memcpy(gBDParam[1] ← gBDParam[0])`，但发生在 `Arm_Study` 已把 `[1]→[0]` 之后，不冲突；
  `Sys_Read_BD()`、`Key_Cmd_Sel()` 也会写 `[1]`，但都在非学习页执行。
- 新 `Arm_Study` 去掉了旧的 `else if(gSaveFlg == 2) gSaveFlg = 0;`。主循环 `gSaveFlg==1 → 2 → Sys_Write_BD()`，
  保存后再置 1 仍可触发下一次保存，功能不受影响。
- 源码编码 GBK/CRLF：改动后用字节检查确认无 UTF-8 混入，并统一了行尾为 CRLF。

## 3. 回传显示数据（臂距）刷新慢：按 HNGL_KXCAN_GLG1Z50_L2X3J 的做法改为"多帧全量 + 双端接口"

**参考工程**：`Source\HNGL_KXCAN_GLG1Z50_L2X3J`（其 `XR_RCV_119-X\AGENTS.md` 记录了同一问题的排查）。
该工程的做法是**取消帧内 index 轮播、8 字节装全量、超过 8 字节就用不同帧 Type 分帧**，
本次按同一思路改，并把接收机的发送封装成接口。

**问题**
屏显的 12 个回传数据（臂距）更新很慢，看起来几乎是卡住的。

**原因分析**（链路：`发射机 100ms 发一帧查询 → 接收机收到才回一帧 → 发射机解析 → 200ms 定时刷屏`）
1. **主因：帧内分组号与发送节拍失配。** 旧协议一帧 8 字节 = `2 字节分组号 + 6 字节(3个uint16)`，
   接收机 `Dev_CAN_RX_DataHandle()` 的分组号 `index` 由 10ms 节拍 `Dev_main()` 自增（0~4），
   而显示帧只在 `vCh==0` 时发出（约 300ms 一次）。300ms/10ms = 30 ≡ 0 (mod 5)，
   每次都采样到同一组，其余组只能靠抖动才慢慢轮到 → 表现为卡住/极慢。
2. `vCh` 按 0/1/2 等间隔轮转，显示帧只占 1/3；`index=4` 还是全 0 空帧，又浪费 1/5。
3. 发射机 `WL_Recv()` 在 `app10ms()` 里每 10ms 轮询解析同一帧，且收到数据后还要等 200ms 定时才刷屏。

**方案：帧内不再放分组号，改由帧类型定位**
- 12 个 uint16 = 24 字节 → 分 3 帧，每帧 8 字节 = 4 个 uint16（小端）。
- 帧类型分配：**Type0 = 屏显A帧(data0~3)、Type5 = 屏显B帧(data4~7)、Type6 = 屏显C帧(data8~11)**；
  Type1/2 保留给 0x280/0x380 数据，Type3/4 是电磁阀电流帧（特殊命令用），正常轮转跳过。
- 轮转序列 `0,1,2,5,6`（`vCh==3||4 → 5`，`>=7 → 0`），100ms 轮询 → **每帧 500ms 刷新一次，且确定性命中**。
- 接收机侧做成接口 `Dev_Disp_Frame(组号, 8字节缓冲)`（User 实现、Src 调用），
  不再用 `lcd_15D_*` 裸全局传递；发射机侧 `WL_Recv(组号, 8字节)` 同为接口，事件驱动调用。

**改动（双端，必须同步烧录）**

接收机 `Source\Project\XR_RCV_119_XR`：

| 文件 | 改动 |
| --- | --- |
| `User/_Device_config.h` | 新增接口声明 `void Dev_Disp_Frame(unsigned char vGrp, unsigned char* pBuf8);` |
| `User/KXCAN_GLZYTC.c`、`User/KXCAN_GLGJT312E_L3X.c` | 删除 `Dev_CAN_RX_DataHandle()`（index 轮播装帧），改为实现 `Dev_Disp_Frame()`；`Dev_main()` 里删掉该调用 |
| `User/KXCAN_GLZYTC.h`、`User/KXCAN_GLGJT312E_L3X.h` | 声明由 `Dev_CAN_RX_DataHandle` 换成 `Dev_Disp_Frame` |
| `Src/App_comm.c` `Uart0_WL_Send()` | case 0/5/6 改为 `Dev_Disp_Frame(0/1/2, &gWLSStr[i]); i += 8;`；`vCh` 轮转改为 `0,1,2,5,6`（跳过 3/4） |

发射机 `FSJ_666`：

| 文件 | 改动 |
| --- | --- |
| `Src/App_usr.h` | 新增接口声明 `void WL_Recv(unsigned char vGrp, const unsigned char* pBuf8);` |
| `Src/App_Comm.c` `Uart0_WL_Rcv()` | `case 0 → WL_Recv(0, vUStr)`；新增 `case 5 → WL_Recv(1, vUStr)`、`case 6 → WL_Recv(2, vUStr)` |
| `Src/CanOpen.c` `app10ms()` | 删除 `WL_Recv();` 轮询调用（改为收到帧时事件驱动） |
| `User/Example.c`、`User/KXCAN_GLGJT312E.c`、`User/KXCAN_GLZYTC.c` 及各自 .h | `WL_Recv()` 改为带参版本：按 `vGrp*4+i` 写 4 个 uint16；收齐 3 组置 `gDataFresh=1`、`gLCDFlg=1` 立即刷屏 |

**注意 / 易踩的点**
- **协议变更，新旧固件不能混装**：老发射机收到新帧会把"数据字节0"当成分组号，显示错位；
  老接收机配新发射机则新发射机永远收不到 Type5/6。两边必须一起编、一起烧。
- 后续扩展帧类型从 **Type 7 顺延**（0/5/6 已占，3/4 为电磁阀电流保留）。
- 原来 `lcd_15D_0~7` 已被 `Src/App_Can.c` 第 617 行（CAN `0x15d`）整体覆盖的隐患，
  因发送改走接口、不再读 `lcd_15D_*`，已自动消失；`App_Can.c` 那处可留着不动。
- **还想更快**可调 `Src/timer.c` 的 `gWLCnt >= 100` 缩短轮询周期（成倍增加无线负载，需实测丢包）。

## 3b. `User/_Device_config.h` 的 `_SELF_CHK_EN` 被写成空宏

`_SELF_CHK_EN` 曾被写成 `#define _SELF_CHK_EN`（无值），`Src/main.c` 里是 `#if _SELF_CHK_EN`，
空宏展开后 `#if` 没有表达式，编译报错。已全部改为 `#define _SELF_CHK_EN 1`，
并在注释里注明"必须给值"。

## 4.（待确认）`User/Example.c` 未加入 Keil 工程

`HDH_YKQ_SPJ.uvprojx` 的 Device 组只包含 `KXCAN_GLGJT312E.c`、`KXCAN_GLZYTC.c`，
但 `User/_Device_config.h` 里 `USER_TYPE` 目前定义为 `Example`，而 `Example.c` 不在工程中。
该组合下 `MainLogic`/`WL_Recv`/`Activar`/`gSwiIconTab1` 等符号无定义，链接会失败。
如需使用 `Example`，需把 `User\Example.c` 加入 uvprojx 的 Device 组。
