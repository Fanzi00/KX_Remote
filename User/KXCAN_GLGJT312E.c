#include "_Device_config.h"
#if (USER_TYPE == KXCAN_GLGJT312E_L3X)

KX119_Recv_t KX_119_recv_dis = {0};
// 无线数据解析
void WL_Recv(void)
{
	const uint16_t index = gRunInfo.vRcvWL[0] | (gRunInfo.vRcvWL[1] << 8);
	const uint8_t* buf = gRunInfo.vRcvWL;

	if(index <= 3)
	{
		uint16_t* pData = &KX_119_recv_dis.data[index * 3].u16Data;
		pData[0] = buf[2] | (buf[3] << 8);
		pData[1] = buf[4] | (buf[5] << 8);
		pData[2] = buf[6] | (buf[7] << 8);
	}
	else if(index == 4)
	{
		KX_119_recv_dis.powerCheck =
			((uint32_t)buf[2] << 0) | ((uint32_t)buf[3] << 8) | ((uint32_t)buf[4] << 16) | ((uint32_t)buf[5] << 24);
	}
}
// 自检反馈
uint8_t showWarn(uint32_t t)
{
	if(t != 0)
	{
		return 1;
	}
	return 0;
}

// 清理界面
void clear_lcd()
{
	// 清空第一栏，-5是为了不清空电池、信号图标
	for(uint8_t i = 0; i < 320/16 - 5; i++)
	{
		LCD_Disp_Txt(FONT_16, i*16, 5, "  ");
	}

	for(uint8_t y = 0; y < 240/16 - 2; y++)
	{
		for(uint8_t x = 0; x < 320/16; x++)
		{
			LCD_Disp_Txt(FONT_16, x*16, y*16+30, "  ");
		}
	}
}

// 立拱模式界面
void LiGong_mode()
{
	static const uint8_t high = 30;        // 行高
	static const uint8_t margL = 10;       // 左边距
	static const uint8_t margR = 10;       // 右边距
	static const uint8_t Vermid = 320 / 2; // 竖中间线
	static const uint8_t Hormid = 240 / 2; // 横中间线

	LCD_Set_Color(COLOR_WHITE_, -1);
	// 模式显示
	if(_BitV(gDIBitV, 0) && _BitV(gDIBitV, 8) && _BitV(gDIBitV, 9))// 中臂&&!立拱&&!凿岩
		LCD_Disp_Txt(FONT_16, Vermid-16*4/2, 5,"模式未选择");
	else
		LCD_Disp_Txt(FONT_16, Vermid-16*4/2, 5,"立拱模式");
	
	LCD_Disp_Txt(FONT_32, Vermid-208/2, Hormid-32/2,"当前臂架:");// 32*4+16+32*2=208
	if(_BitV(gDIBitV, 2))
		LCD_Disp_Txt(FONT_32, Vermid-208/2+32*4+16, Hormid-32/2,"左臂");
	else if(_BitV(gDIBitV, 3))
		LCD_Disp_Txt(FONT_32, Vermid-208/2+32*4+16, Hormid-32/2,"右臂");
	else if(!_BitV(gDIBitV, 2) && !_BitV(gDIBitV, 3))
		LCD_Disp_Txt(FONT_32, Vermid-208/2+32*4+16, Hormid-32/2,"中臂");
}

// 发射机界面显示
void display()
{
	static const uint8_t high = 30;     // 行高
	static const uint8_t margL = 10;    // 左边距
	static const uint8_t margR = 10;    // 右边距
	static const uint8_t margT = 5;		// 上边距
	static const uint8_t Vermid = 320 / 2; // 竖中间线
	static const uint8_t Hormid = 240 / 2; // 横中间线
	static const uint8_t Title = 30; // 第一行的高度
	static const uint8_t LwordLenth = 8 * 3 + 16 * 2; // 左侧文字长度
	static const uint8_t RwordLenth = 8 * 3 + 16 * 2; // 右侧文字长度

	// 模式显示
	uint8_t flag = 0;
	if(_BitV(gDIBitV, 0)) // 左臂
		flag = 0;
	else if(_BitV(gDIBitV, 0)) // 中臂
		flag = 1;
	else // 右臂
		flag = 2;
	if(flag == 0)
	{
		LCD_Set_Color(COLOR_WHITE_, -1);
		LCD_Disp_Txt(FONT_16, Vermid-32/2, margT,"左臂");//16*2=32(文字宽度) 第一行显示
		// 第1列
		LCD_Disp_Txt(FONT_16, margL, Title+high*0,"1#左距:");//16*4+8+8*5+8*3=136,每行设置20
		LCD_Disp_Txt(FONT_16, margL, Title+high*1,"1#右距:");
		LCD_Disp_Txt(FONT_16, margL, Title+high*2,"1#上距:");
		LCD_Disp_Txt(FONT_16, margL, Title+high*3,"1#下距:");

		// 显示数据数字
		LCD_Set_Color(COLOR_YELLW_, -1);
		LCD_Disp_Intx(FONT_16, margL+LwordLenth, Title+high*0, KX_119_recv_dis.data[0].u16Data, 5);
		LCD_Disp_Intx(FONT_16, margL+LwordLenth, Title+high*1, KX_119_recv_dis.data[1].u16Data, 5);
		LCD_Disp_Intx(FONT_16, margL+LwordLenth, Title+high*2, KX_119_recv_dis.data[2].u16Data, 5);
		LCD_Disp_Intx(FONT_16, margL+LwordLenth, Title+high*3, KX_119_recv_dis.data[3].u16Data, 5);
		// LCD_Disp_Intx(FONT_16, margL+16*4+8, Title+high*4, KX_119_recv_dis.u16Data4, 5);
	}
	else if(flag == 1)
	{
		LCD_Set_Color(COLOR_WHITE_, -1);
		LCD_Disp_Txt(FONT_16, Vermid-32/2, margT,"中臂");//16*2=32(文字宽度) 第一行显示
		// 第2列
		LCD_Disp_Txt(FONT_16, margL, Title+high*0,"2#左距:");//16*4+8+8*5+8*3=136,每行设置20
		LCD_Disp_Txt(FONT_16, margL, Title+high*1,"2#右距:");
		LCD_Disp_Txt(FONT_16, margL, Title+high*2,"2#上距:");
		LCD_Disp_Txt(FONT_16, margL, Title+high*3,"2#下距:");

		// 显示数据数字
		LCD_Set_Color(COLOR_YELLW_, -1);
		LCD_Disp_Intx(FONT_16, margL+LwordLenth, Title+high*0, KX_119_recv_dis.data[4].u16Data, 5);
		LCD_Disp_Intx(FONT_16, margL+LwordLenth, Title+high*1, KX_119_recv_dis.data[5].u16Data, 5);
		LCD_Disp_Intx(FONT_16, margL+LwordLenth, Title+high*2, KX_119_recv_dis.data[6].u16Data, 5);
		LCD_Disp_Intx(FONT_16, margL+LwordLenth, Title+high*3, KX_119_recv_dis.data[7].u16Data, 5);
	}
	else if(flag == 2)
	{
		LCD_Set_Color(COLOR_WHITE_, -1);
		LCD_Disp_Txt(FONT_16, Vermid-32/2, margT,"右臂");//16*2=32(文字宽度) 第一行显示
		// 第3列
		LCD_Disp_Txt(FONT_16, margL, Title+high*0,"3#左距:");//16*4+8+8*5+8*3=136,每行设置20
		LCD_Disp_Txt(FONT_16, margL, Title+high*1,"3#右距:");
		LCD_Disp_Txt(FONT_16, margL, Title+high*2,"3#上距:");
		LCD_Disp_Txt(FONT_16, margL, Title+high*3,"3#下距:");

		// 显示数据数字
		LCD_Set_Color(COLOR_YELLW_, -1);
		LCD_Disp_Intx(FONT_16, margL+LwordLenth, Title+high*0, KX_119_recv_dis.data[8].u16Data, 5);
		LCD_Disp_Intx(FONT_16, margL+LwordLenth, Title+high*1, KX_119_recv_dis.data[9].u16Data, 5);
		LCD_Disp_Intx(FONT_16, margL+LwordLenth, Title+high*2, KX_119_recv_dis.data[10].u16Data, 5);
		LCD_Disp_Intx(FONT_16, margL+LwordLenth, Title+high*3, KX_119_recv_dis.data[11].u16Data, 5);
	}
	
	if(showWarn(KX_119_recv_dis.powerCheck))
	{
		LCD_Set_Color(COLOR_RED_, -1);
		LCD_Disp_Txt(FONT_16, margL, Title+high*5,"自检异常");
	}
	else
	{
		LCD_Disp_Txt(FONT_16, margL, Title+high*5,"         ");
	}
	LCD_Set_Color(COLOR_WHITE_, -1); // 字体 设置为白色
	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*0,"mm");
	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*1,"mm");
	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*2,"mm");
	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*3,"mm");
	// LCD_Disp_Txt(FONT_16, margL+16*4+8, Title+high*4,"mm");
#if 0
	// 第二列
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136, Title+high*0,"润滑气压:");
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136, Title+high*1,"推进压力:");
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136, Title+high*2,"润滑压力:");
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136, Title+high*3,"注浆压力:");
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136, Title+high*4,"空压机:");
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136, Title+high*5,"水泵:");

	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136+16*4+8*6, Title+high*0,"bar");
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136+16*4+8*6, Title+high*1,"bar");
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136+16*4+8*6, Title+high*2,"bar");
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136+16*4+8*6, Title+high*3,"bar");
	// 
	if(_BitV(gDIBitV, 17)) LCD_Disp_Txt(FONT_16, margL+16*2+8, 30+high*4, "开启"); else LCD_Disp_Txt(FONT_16, margL+16*2+8, 30+high*4, "关闭"); 
	if(_BitV(gDIBitV, 18)) LCD_Disp_Txt(FONT_16, margL+16*2+8, 30+high*5, "开启"); else LCD_Disp_Txt(FONT_16, margL+16*2+8, 30+high*5, "关闭"); 
	// 第二列
	LCD_Disp_Intx(FONT_16, (Vermid*2-margR-136)+16*4+8, 30+high*0, KX_119_recv_dis.Data4, 5);
	LCD_Disp_Intx(FONT_16, (Vermid*2-margR-136)+16*4+8, 30+high*1, KX_119_recv_dis.Press_Push, 5);
	LCD_Disp_Intx(FONT_16, (Vermid*2-margR-136)+16*4+8, 30+high*2, KX_119_recv_dis.Press_Lube, 5);
	LCD_Disp_Intx(FONT_16, (Vermid*2-margR-136)+16*4+8, 30+high*3, KX_119_recv_dis.Press_Grout, 5);
	if(_BitV(gDIBitV, 21)) LCD_Disp_Txt(FONT_16, Vermid*2-margR-136+16*3+8, 30+high*4, "开启"); else LCD_Disp_Txt(FONT_16, Vermid*2-margR-136+16*3+8, 30+high*4, "关闭"); 
	if(_BitV(gDIBitV, 22)) LCD_Disp_Txt(FONT_16, Vermid*2-margR-136+16*2+8, 30+high*5, "开启"); else LCD_Disp_Txt(FONT_16, Vermid*2-margR-136+16*2+8, 30+high*5, "关闭"); 
#endif
}

// 主要逻辑
void MainLogic()
{
	display();
}

#endif // USER_TYPE == USER_HNGL_GLT312E
