#ifndef TASK_H
#define TASK_H
#include "config.h"
#include "stdio.h"
//#include "data_types.h"

#define MAX_TASK_NUM    4	//			最大可运行任务数
#define SYS_TCK_TIME	10	//			每个tick的时间0.1ms

/*每个任务的定时*/
#define TASK_IO_TM		(60*SYS_TCK_TIME)
#define TASK_AD_TM		(70*SYS_TCK_TIME)
#define TASK_SCR_TM		(80*SYS_TCK_TIME)
#define TASK_CAN_TM	(100* SYS_TCK_TIME)

// Total memory per task is 7 bytes
typedef struct {
	void ( * p_task)(void);  // 任务函数指针
   	unsigned short delay;// 延迟时间
   	unsigned short period;// 调度周期
   	unsigned short ready;// 运行态标志
}  sTask; 

/********************************************************************
分配任务，调度处在运行态的任务
需要在main函数中调用 
********************************************************************/
void dispatch_tasks(void);

/********************************************************************

增加任务，如果超出设定的任务数，返回错误
如果成功，返回当前任务号
********************************************************************/
unsigned char add_task(void(*pFunction)(), const unsigned short , const unsigned short);

/********************************************************************
删除任务，如果欲删除的任务不存在，返回错误
********************************************************************/
unsigned char delete_task(const unsigned char) ;

/********************************************************************
   任务调度
********************************************************************/
void Task_sys_tick(void) ;

#endif
