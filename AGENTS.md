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
- 帧类型分配：**Type0 = 屏显A帧(data0~3)、Type5 = 屏显B帧(data4~7)、Type7 = 屏显C帧(data8~11)**；
  Type1/2 给 0x280/0x380 数据，Type3/4 是电磁阀电流帧（特殊命令用），Type6 为"设置AI反馈"（原有功能，保留、按需发送）；正常轮转跳过 3/4/6。
- 轮转序列 `0,1,2,5,7`（`vCh==3||4 → 5`，`vCh==6 → 7`，`>=8 → 0`），100ms 轮询 → **每帧 500ms 刷新一次，且确定性命中**。
- 接收机侧做成接口 `Dev_Disp_Frame(组号, 8字节缓冲)`（User 实现、Src 调用），
  不再用 `lcd_15D_*` 裸全局传递；发射机侧 `WL_Recv(组号, 8字节)` 同为接口，事件驱动调用。

**改动（双端，必须同步烧录）**

接收机 `Source\Project\XR_RCV_119_XR`：

| 文件 | 改动 |
| --- | --- |
| `User/_Device_config.h` | 新增接口声明 `void Dev_Disp_Frame(unsigned char vGrp, unsigned char* pBuf8);` |
| `User/KXCAN_GLZYTC.c`、`User/KXCAN_GLGJT312E_L3X.c` | 删除 `Dev_CAN_RX_DataHandle()`（index 轮播装帧），改为实现 `Dev_Disp_Frame()`；`Dev_main()` 里删掉该调用 |
| `User/KXCAN_GLZYTC.h`、`User/KXCAN_GLGJT312E_L3X.h` | 声明由 `Dev_CAN_RX_DataHandle` 换成 `Dev_Disp_Frame` |
| `Src/App_comm.c` `Uart0_WL_Send()` | case 0/5/7 改为 `Dev_Disp_Frame(0/1/2, &gWLSStr[i]); i += 8;`；`vCh` 轮转改为 `0,1,2,5,7`（跳过 3/4/6，Type6 留给"设置AI反馈"） |

发射机 `FSJ_666`：

| 文件 | 改动 |
| --- | --- |
| `Src/App_usr.h` | 新增接口声明 `void WL_Recv(unsigned char vGrp, const unsigned char* pBuf8);` |
| `Src/App_Comm.c` `Uart0_WL_Rcv()` | `case 0 → WL_Recv(0, vUStr)`；新增 `case 5 → WL_Recv(1, vUStr)`、`case 7 → WL_Recv(2, vUStr)`（原 `case 6` 是"设置AI的反馈"，保留不动） |
| `Src/CanOpen.c` `app10ms()` | 删除 `WL_Recv();` 轮询调用（改为收到帧时事件驱动） |
| `User/Example.c`、`User/KXCAN_GLGJT312E.c`、`User/KXCAN_GLZYTC.c` 及各自 .h | `WL_Recv()` 改为带参版本：按 `vGrp*4+i` 写 4 个 uint16；收齐 3 组置 `gDataFresh=1`、`gLCDFlg=1` 立即刷屏 |

**注意 / 易踩的点**
- **协议变更，新旧固件不能混装**：老发射机收到新帧会把"数据字节0"当成分组号，显示错位；
  老接收机配新发射机则新发射机永远收不到 Type5/7（屏显B/C帧）。两边必须一起编、一起烧。
- 后续扩展帧类型从 **Type 8 顺延**（0/5/7 屏显三帧、1/2 数据、3/4 电磁阀电流、6 设置AI反馈，均已占）。
- 原来 `lcd_15D_0~7` 已被 `Src/App_Can.c` 第 617 行（CAN `0x15d`）整体覆盖的隐患，
  因发送改走接口、不再读 `lcd_15D_*`，已自动消失；`App_Can.c` 那处可留着不动。
- **还想更快**可调 `Src/timer.c` 的 `gWLCnt >= 100` 缩短轮询周期（成倍增加无线负载，需实测丢包）。

## 3b. `_SELF_CHK_EN` 用 `#ifdef` 控制（不要 `#if`）

`Src/main.c` 里自检调用 `SelfChk_main()` 原用 `#if _SELF_CHK_EN` 判定。空宏（无值）展开后
`#if` 没有表达式会编译报错，所以最初改成 `#define _SELF_CHK_EN 1` + `#if`。
现改为 **`#ifdef _SELF_CHK_EN`**：`User/_Device_config.h` 中各机型分支用 `#define _SELF_CHK_EN`
（定义即开启，无需给值；需要关闭的机型 `#undef` 即可）控制，并删除了原先"必须给值"的默认块。

## 4. `USER_TYPE` 必须与编译的机型 .c 匹配（否则链接报 undefined symbol）

`uvprojx` 的 Device 组只包含 `KXCAN_GLGJT312E.c`、`KXCAN_GLZYTC.c`（不含 `Example.c`）。
各机型 .c 里的 `WL_Recv`/`MainLogic`/`SelfChk_main`/`Activar`/`showWarn`/`clear_lcd`（发射机）
以及 `Dev_Disp_Frame`/`Dev_main`/`error_prote`/`Dev_CAN_TX*`/`Dev_CAN_RX`（接收机）的实现
都被 `#if (USER_TYPE == 机型)` 包裹：`USER_TYPE` 不对该机型时实现整体不进编译 -> 链接 `undefined`。

- **编 Example（示例/通用）机型**：`USER_TYPE` 保持 `Example`，并需把 `User\Example.c`、`User\Example.h` 加入 eide 工程的 User 源文件组（原 uvprojx 的 Device 组未含 `Example.c`，导致实现不进编译、链接 `undefined`）。`Example.c` 内部实现已被 `#if (USER_TYPE == Example)` 包裹，加入后即可提供 `WL_Recv`/`Activar`/`MainLogic`/`SelfChk_main`/`gSwiIconTab1/2` 等全部符号。
- 编其它机型（如 `KXCAN_GLGJT312E_L3X` 拱架台车、`KXCAN_GLZYTC_L2X` 装药台车）：把对应机型 `.c` 加入工程，并让 `USER_TYPE` 与之匹配（或改 `USER_TYPE` 指向该机型）。

## 4b. 设备层（User）公共函数接口统一声明在 `User/_Device_config.h`

为消除调用方（Src 层）缺声明导致的隐式声明/参数不匹配报错，两工程的 `User/_Device_config.h`
各自集中声明了机型 .c 对外暴露的公共函数（不再分散在 `App_usr.h` 和各机型 `.h`）。
`config.h` 已 `#include "_Device_config.h"`，所有 Src/User 文件自动可见。签名须与 .c 实现一致：
- 发射机：`WL_Recv(unsigned char, const unsigned char*)`、`SelfChk_main(void)`、`MainLogic(void)`、`Activar(void)`、`showWarn(uint32_t)`、`clear_lcd(void)`。
- 接收机：`Dev_Disp_Frame(uint8_t, uint8_t*)`、`Dev_main(void)`、`error_prote(void)`、`Dev_CAN_TX_DataHandle(void)`、`Dev_CAN_TX(void)`、`Dev_CAN_RX(void)`。
（`App_usr.h` 与各机型 `.h` 里的同名声明保留，与之一致、无害；若要彻底清理可删除。）

## 5. GLMZ213(锚注) 发射机/接收机按《213遥控器测试结果》映射 gDIBitV 位号

测试文档：`Source\Project\ZZZ_project_data\KX119-GLMZ213-L3X.md`（协议 + 发射机布局 + 测试表）。
核心结论：**接收机 `FSJ_DI` 第 N 位 == 发射机 `gDIBitV` 第 N 位**。FSJ_DI 由无线帧里的 vYKDI1/2 直接拼出（`vYKDI1[0]|vYKDI1[1]<<8|vYKDI2[0]<<16|vYKDI2[1]<<24`），而 vYKDI1/2 正是 gDIBitV 的 4 字节，故发射机 gSwiIconTab 与接收机 0x1A0 映射用同一套位号。

### gDIBitV 实测位号（= FSJ_DI 位号）
- 水泵=0(关=1/开=0)  搅拌=1(关=1/开=0)  注浆正/反=2/3(均0=中位)  水阀开/关=4/5(均0=中位)
- 换钎=15(开=1)  空压机=28/29(开=1, 两bit联动)  电机=16  钻臂=17  锚杆=18  吊篮=19
- F5~F8=20~23(仅F8打铃)  启动=24  手柄2/3顶钮=30
- 左手柄使能 测试在 0x188.1(手柄数据, 非FSJ_DI)；右手柄/中手柄使能 在 bit30

### 发射机 FSJ_666/User/KX119_GLMZ213_L3X.c
- `gSwiIconTab1/2`：shift 改为实测位号（水泵0/水阀4/换钎15/注浆2/搅拌1/空压机29）。3档开关 gears=3，两bit=shift 与 shift+1。
- 自检：新增 `SelfChk_Str`/`SelfChk_Proc`（错误码 10~20 开关/模式、30~34 自复位按钮、50~55 手柄），`SelfChk_main` 改为 0延时→1自检循环→2运行 状态机，判据为信号图标(未激活gLaBaFlg==0 或 无线gWLSingV<=1 为红)。
- `SelfChk_Proc` 数据用 `gDIBitV0 ^ 0x8000`（安全帧会清零 gDIBitV，不能直接用）。手柄中位判据 `VKC_Send[2..7] != 0x7F`。
- `Activar` 的 F1~F8 位映射(16电机/17钻臂/18锚杆/19吊篮/20~23 F5~F8) 已与测试一致，无需改。

### 接收机 XR_RCV_119_XR/User/KX119_GLMZ213_L3X.c
- `Dev_CAN_TX_DataHandle` 把 FSJ_DI 位 → 0x1A0 开/关位：水泵(0)、水阀(4/5)、注浆(2/3)、空压机(28|29)、换钎(15)、搅拌(1)、电机(16)、钻臂(17)、锚杆(18)、吊篮(19)、F8打铃(23)、启动(24)、手柄使能(bit30)。
- 推进调压=B2 = `gRunInfo.vYKDI3[0]`（发射机 ADCRst[9] 缩放 0~127，无线帧 byte15）。
- 0x2A0/0x3A0 仍由 `gRunInfo.vArm[0..5]` 映射（摇杆）。
- 位号已从旧猜测(换钎=7、空压机=6 等)改成测试结果；`KX119_GLMZ213_L3X.h` 注释已同步。
- 未覆盖项：Watchdog(发射机周期翻转位)、Tilt-Switch、左手柄使能(在手柄数据不在FSJ_DI) — 暂按约定/置0，待面板接线确认。

## 6. GLMZ_L2X3J(湿喷机) 按《KX119-GLMZ-L2X3J.md 测试》修正位号映射

测试文档：`Source\Project\ZZZ_project_data\KX119-GLMZ-L2X3J.md`（协议 + 测试，测试部分原先缺失、现已补齐）。
核心结论同 §5：接收机 FSJ_DI 第 N 位 == 发射机 gDIBitV 第 N 位；位号规则 = 测试观测帧 0x18A 的 byte*8 + bit。

### 错误修正（原代码按协议猜位号、测试补齐前写的，多处错误）
- 发射机 `gSwiIconTab1/2` 原先 shift 全为协议猜测值（空压机=1、换钎=2、注浆=5、水阀=3、搅拌=7），全部错；应按测试改为：
  水泵=0(关=1/开=0) 空压机=25(关=1/开=0) 换钎=15(开=1/关=0) 注浆=8/9(关/开,三档) 水阀=2/3(开/关,三档自复位) 搅拌=1(开=1/关=0, 模拟量ADCRst[16]>500, 经DI_Correction映射)。
- 发射机 `Activar` 原先把 F5~F8 映射到 bit20~23，但本机型只有 F1~F4 侧边按钮（bit16~19）；bit20~23 实为电机/转臂/锚杆/吊篮模式开关。已改为 F1~F4=bit16~19，去掉 F5~F8。
- 接收机 `Dev_CAN_TX_DataHandle` 原先按协议猜位号（水泵=0但极性反、空压机=1、换钎=2、水阀=3/4、左手柄使能=8、右手柄使能=9、F4=12、电机=13、转臂=14、锚杆=15、启动=24、吊篮=25），全部错；已按测试重写极性+位号。`KX119_GLMZ_L2X3J.h` 位域注释已同步。

### gDIBitV 实测位号（= FSJ_DI 位号，来自测试）
- 水泵=0(关=1/开=0) 空压机=25(关=1/开=0) 水阀开/关=2/3(三档自复位,均0=中位) 换钎=15(开=1/关=0)
- 注浆关/开=8/9(三档自锁,均0=中位) 搅拌=1(开=1/关=0, ADCRst[16]>500 经DI_Correction映射)
- F1~F4=16~19(侧边自复位) 电机启停=20 转臂模式=21 锚杆模式=22 吊篮模式=23
- 右手柄使能=28(顶钮 0x18A.3.4, ADCRst[1]>500 通用映射) 左手柄使能=6(顶钮 0x188.1=VKC_Send[1]==0, 经DI_Correction映射)
- 启动/打铃 = F4(=19，协议B3.b0与B1.b4同为打铃)；急停走 vYKJiTing(负逻辑)，非 FSJ_DI 位

### 协议差异注意
- 本机型协议 0x1A0 的 B1.b2/b3（注浆位）与 B2（推进调压）均为"保留"，即 注浆/搅拌 在协议中无对应帧位 -> 接收机不向车转发（仅遥控端屏显）。这与213不同（213把注浆/搅拌送了车）。

### 模拟量/手柄顶钮开关的 gDIBitV 映射（发射机必须定义 DI_Correction，App_IO.c 在算 gDIBitV 前调用）
- 数字开关：DI 扫描 `gDIBitV0 |=(1<<i)`，位号 = 测试 0x18A 帧的 byte*8+bit（水泵/空压机/水阀/换钎/注浆/F1~F4/电机/转臂/锚杆/吊篮）。
- 右手柄使能：`ADCRst[1]>500 -> bit28`（App_IO.c 通用映射，非本文件）。
- 搅拌：`ADCRst[16]>500 -> bit1`（0x18D.4&5=ADCRst[16]），本文件 `DI_Correction` 映射，开=1/关=0。
- 左手柄使能：`VKC_Send[1]==0 -> bit6`（0x188.1=Y_Send[1]），本文件 `DI_Correction` 映射。
- 注意：`DI_Correction()` 在 `_Device_config.h` 无条件声明、被公共 `App_IO.c` 调用，故每个机型都必须提供定义，否则链接报 undefined。

### 手柄/直线摇杆 通道（来自测试"手柄通道测试"，用于自检回中 与 收发轴映射）
- 左手柄：上下=VKC_Send[2](0x188.2)  左右=VKC_Send[3](0x188.3)
- 右手柄：上下=VKC_Send[6](0x188.6)  左右=VKC_Send[7](0x188.7)
- 直线摇杆1：模拟量 ADCRst[3](0x189.6&7, 上0xE35/中0x7EE/下0x1A0)；由 `ADC_Correction` 线性映射到 0~255 -> `gLinR1Send` -> 无线帧 ad1 槽(索引8) -> 接收机 vArm[7] -> 0x3A0 左直线
- 直线摇杆2：VKC_Send[5](0x188.5, 摇杆轴, 0~255) -> vArm[4] -> 0x3A0 中直线（唯一走 VKC_Send 的直线摇杆）
- 直线摇杆3：模拟量 ADCRst[9](0x18B.4&5, 上0xE36/中0x7F1/下0x1AB)；由 `ADC_Correction` 线性映射到 0~126 -> `gLinR3Send` -> 无线帧 ad2 槽(索引9) -> 接收机 vArm[8] -> 0x3A0 右直线
- **为何直线摇杆1/3 走 ad 槽而非 VKC_Send**：避免 `KX_Send_joylogic` 的 (4,5) 轴对主轴优先级逻辑（两轴同对时从轴被清零）。将 1/3 放 ad1/ad2 槽（接收机 vArm[7]/[8]），只留 直线摇杆2 在 VKC_Send[5]，其轴对伙伴 VKC_Send[4] 空闲=中位，无冲突。
- 接收机侧：无线帧首字节=gWLDType(=VKC_Send[0])，故 vArm[k]=VKC_Send[k+1]（仅 vk[0..7] 经 SCALE_255_TO_AMAX 缩放；ad 槽 vArm[7] 缩放、vArm[8] 非缩放，故直线摇杆3 直接发 0~126 与协议 0x3A0 量程一致）。
- 接收机 0x2A0：左手柄 Y/X = vArm[1]/[2]，右手柄 Y/X = vArm[5]/[6]。0x3A0：左直线=vArm[7](直线摇杆1)、中直线=vArm[4](直线摇杆2)、右直线=vArm[8](直线摇杆3)。
- 自检回中：左手柄=VKC_Send[2,3]、右手柄=VKC_Send[6,7]、直线摇杆2=VKC_Send[5] 判 !=0x7F；直线摇杆1 判 ADCRst[3] 离中位0x7EE 超 0x100；直线摇杆3 判 ADCRst[9] 离中位0x7F1 超 0x100。

### 待确认项（已尽量用最合理推导，实机前可复核）
- 0x2A0/0x3A0 的 上下/左右 方向与车上实测一致后再确认是否取反（已留 TODO）
- 急停：协议有 B3.b5 急停(负逻辑)，但测试未测；现沿用 vYKJiTing 无线急停标志，未在 FSJ_DI 置位
- 强制激活：本机型 `Activar` 目前**未实现**（仅有 F1~F4 正常激活，gSelfChkMask==0 门控）。213 的强制激活用 F5~F8 长按6s绕过自检，但 L2X3J 无 F5~F8（bit20~23 是模式开关），需等实机确认哪颗空闲侧边按钮(或组合)改派为强制激活按钮后再补 `forceDly` 逻辑（参考 §2.4）
- 自检中"手柄使能"项：原 34/35(左手柄使能/右手柄使能) 与 50~53 手柄回中重复，已删除；若需在上电自检时检查手柄使能顶钮是否被按住，需另起错误码（区别于手柄回中）
- L2X3J 的 `_SELF_CHK_EN` 在 `User/_Device_config.h` 当前被注释（自检功能关闭）；要启用上电自检，需去掉该行注释（当前 SelfChk 代码已就绪，但 dormant）

## 7. RX drop must re-activate + force-activate for all TX (2026-10-20)
- Bug: after RX drops and reconnects, TX auto-connects because gLaBaFlg was not cleared.
- Fix: Src/main.c gT0SFlg block: whenever RX transitions offline->online (gWLSingV>0 while stWLHad was 0; covers both reconnect-after-drop AND RX powered-on later after TX already activated) clear gLaBaFlg=0; TX will NOT auto-connect, user must press activate key or force-activate. Online-only activation (RX already up when key pressed) is not disturbed.
- Force-activate: added to all User/*.c Activar(): F5~F8 long-press 6s (forceDly counts to 6000, DI_Scan 1ms tick) sets gLaBaFlg=1 bypassing self-check. L2X3J has no F5~F8, uses F1~F4.
- 关闭自动重连，掉线后必须手动激活（2026-10-24 修订，`Src/main.c` gT0S_Prog 分支）：`gWLSingV==0`（本秒未收到任何有效接收机回帧）时累计 `stWLLostDly`，**连续丢满 `_WL_LOST_DLY`(=2s) 才清 `gLaBaFlg = 0`**（在 `stWLLostDly >= _WL_LOST_DLY` 分支内显式清除；`wl_reset()` 本身只复位无线模块 IO、不清激活态）。即：瞬时(<2s)丢帧不清激活态、可自动重连；持续≥2s 掉线才清激活态，恢复后须手动重新激活（按 F3~F8 或强制激活 6s）。撤销 534cdca「对齐标准机、掉线后保持激活态自动重连」的做法。

## 8. 主页面(Disp_mainSPJ)显示优化：静态文字进页面只画一次，数值实时刷新（2026-10-24）

**问题**
213 发射机 `User/KX119_GLMZ213_L3X.c` 的 `display()` 每 200ms 屏刷都把全部标签文字+单位+数值全量重绘（约 30 条文字 + 12 个数值）。
主循环被重显示拖住，与单帧 UART 接收（uart.c 仅 64 字节缓冲 + 单标志 `gUart0RcvFlg`，`Uart0_WL_Rcv` 一次只解一帧并 `UART0Count=0`）叠加，
偶发丢失无线回帧 → `gWLSingV` 在 5~9 波动 → 屏幕信号格 2?3 跳。标准机 `BRC_T_ST` 显示轻，故稳。

**解决方法（拆静态/动态绘制）**
- `display()` 保留全量绘制；新增 `display_static()`（只画标签+单位）、`display_dynamic()`（只刷动态数值：模式值/各路 Intx/水流量开关）。
- `MainLogic()`（每帧）改调 `display_dynamic()` + `selfDisplay()`。
- 新增 `MainLogic_Enter()`（进页面首帧 `gLcdFresh==1` 时由 `Disp_mainSPJ` 调用）：调一次 `display()` 全量绘制（静态文字只此一次）。
- `User/_Device_config.h` 增加 `void MainLogic_Enter(void);` 声明。
- `Src/App_lcd.c` 的 `Disp_mainSPJ`：`if((gLcdFresh==1)||(gDataFresh==1))` 块内捕获首帧 `vEnter`，首帧 `MainLogic_Enter()`、其余 `MainLogic()`。
- 注意：`Disp_GJ`（臂架界面）与 `Disp_mainSPJ` 进页面块结构完全克隆，但 `Disp_GJ` 不调 `MainLogic`（用 `remote_18c`），故仅改 `Disp_mainSPJ` 体内（用其独有 `LCD_Set_Color(COLOR_BLUE_,-1)` 锚定），不要把 `MainLogic_Enter()` 加进克隆的进页面块（会误画到臂架页）。

**效果 / 注意**
- 静态文字仅进页面绘制一次，数值每帧实时更新；屏刷负载显著下降，预期缓解信号格 2?3 跳动。
- 数值 `LCD_Disp_Intx(...,5)` 依赖底层按固定宽度清格；若某数值位数减少出现残影，需在 `display_dynamic` 中先清数值格再画。
- 验证：刷机后对比 213 与标准机 `gWLSingV` 实时值，应更接近标准机（稳定 3 格）。
- 修正（2026-10-24）：单次 `display()` 在 `LCD_Disp_PIC(31)` 大图光栅之后调用会被 LCD 模块丢弃一部分（实测只画到“孔数:”之后就断了），所以进页面**连续 3 帧**调用 `display_static()`（由 `Disp_mainSPJ` 内 `static stEnterCnt` 控制：进页面置 3，每帧 >0 则画静态并自减），3 帧后再只 `display_dynamic()`。静态文字只在进页面画一次（3 帧内画全），数值每帧实时刷新。

- GLMZ213_L3X already had force-activate; GLZYTC/GLGJT312E/Example/QLG0312A/L2X3J added now.

## 8. 自学习通道选择接口 `_ARM_STUDY_MASK`（2026-10-21, 所有遥控器通用）
**需求**：F2 采中位原要求 8 路万向轴全部在 100~4090；没接手柄的通道读到 ≈0/≈4095 会触发"手柄采样异常"并在采集前 `return`，卡住整个自学习。需让每个遥控器可配置哪些通道参与自学习。模拟量今后都接 VKC_Send 的 8 个通道。

**实现（按位掩码，配置集中在 `_Device_config.h` 各设备块）**
- `Src/App_usr.c` `Arm_Study()`：F2 校验、F2 采集、F3 初始化、F3 跟踪、F1 校验 5 处万向轴循环均在循环体首加 `if(!((_ARM_STUDY_MASK >> i) & 1)) continue;`。掩码外的轴：不校验、不采集(保留进入学习时从 [0] memcpy 来的默认/旧值)、F1 不校验；F1 提交 `memcpy` 原样带回。
- `User/_Device_config.h`：每个 `#if (USER_TYPE == …)` 设备块内 `#include "X.h"` 之后加 `#define _ARM_STUDY_MASK <值>`；文件末尾加 `#ifndef _ARM_STUDY_MASK / #define 0xFF / #endif` 兜底。bit0~bit7 = 万向轴 i = `VKC_Send[i]`（经 `KX_Send_joysend→KX_Send_joylogic` 下标不变，joylogic 只把每对里动得少的轴置中位、不重排下标），1=学/0=跳。
- 各机默认：`KX119_GLMZ_L2X3J` = `((1<<2)|(1<<3)|(1<<6)|(1<<7))`（0xCC，实测 2 万向=主/副操作杆轴 2/3/6/7）；其余 Example/GLGJT312E/GLZYTC/QLG0312A/GLMZ213 = 0xFF（全学）。

**为什么不放设备头 / App_usr.h**：`config.h` 会在 `_Device_config.h` 包含设备头之前就拉入 `App_usr.h`，若在 `App_usr.h` 或设备头里定义掩码，会被 armcc 判 47-D“incompatible redefinition”。集中在 `_Device_config.h` 设备块（单一定义）+ 末尾 `#ifndef` 兜底最稳。

**注意 / 副作用**
- 发射通路(KX_Send_joysend/KX_Send_joylogic)仍按全部 _ARM_USE=8 轴运行，`_ARM_STUDY_MASK` 只影响自学习校验/采集范围，不改变发射轴数；未接轴的发射值由各自默认/旧标定决定。
- bit i ? `VKC_Send[i]` ? 万向轴 i ? `ADCRst[gArmUpTab[i]]`（gArmUpTab 随机型分支不同：KX 分支 ={10,6,15,7,12,13,0,5}）。

## 9. 213遥控器连接后经常掉线：信号丢失立即断线，改为连续丢失2秒才判定离线（2026-10-24）
**现象**：客户反馈 GLMZ213（213遥控器）连接之后经常掉线，掉线后要重按激活键才能恢复控制。
**原因**：`Src/main.c` 的 1 秒任务（`gT0SFlg`，`Src/timer.c` 中 `gT0SCnt>=1000` 置位）里，只要过去 1 秒内一帧有效数据都没收到（`gWLRcvOk==0`；`Uart0_WL_Rcv()` 中校验错的帧提前 return、不计入），就**立即** `wl_reset()` 重置无线模块并 `stWLHad=0` 判定接收机离线；等信号恢复时因 `stWLHad==0` 又清 `gLaBaFlg`，用户必须重按激活键（见 §7）。瞬时丢帧、信号闪烁、或接收机 CAN 忙时连续漏回帧（接收机 `main.c` 仅 `gCanOvDly==0` 才回发无线帧，CAN 优先），都会凑出"整秒无数据"→ 误掉线。
**修改（只改发射机 `FSJ_666/Src/main.c`，接收机未动）**
- 新增 `#define _WL_LOST_DLY 2`（连续丢失秒数门限，即断线延时）与 `static unsigned char stWLLostDly`（信号丢失计时，秒）。
- `gT0SFlg` 块：`gWLSingV==0` 时 `stWLLostDly++`（封顶 `_WL_LOST_DLY`），仅当 `stWLLostDly>=_WL_LOST_DLY`（连续 2 秒无信号）才 `wl_reset()` + `stWLHad=0`；收到数据时 `stWLLostDly=0`。掉线延时期间 `stWLHad` 保持 1，不清 `gLaBaFlg`，信号恢复无需重新激活。
**效果**：<2 秒的信号中断不再断线、不重置无线模块、不需重激活；连续 2 秒无信号才按原逻辑判定离线（离线期间仍每秒 `wl_reset()` 促重连）。
**注意 / 后续**
- `gWLSingV`（信号格显、timer.c 红灯）仍为瞬时值、未加延时；要调断线延时只改 `_WL_LOST_DLY`。
- 接收机侧 `gCommOvDly>500`（500ms 收不到有效帧就 `wl_reset()` 重置无线模块）本次未改；若实机仍偶发掉线，可评估将该门限也放宽到 2s，避免两侧互相重置导致级联中断。
