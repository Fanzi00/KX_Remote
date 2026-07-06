#include "Task.h"

//全局任务列表 
static sTask g_tasks[MAX_TASK_NUM];

/********************************************************************
分配任务，调度处在运行态的任务
需要在main函数中调用 
********************************************************************/
void dispatch_tasks(void) 
{
	unsigned char Index;

   	for (Index = 0; Index < MAX_TASK_NUM; Index++) 
   	{
		if ( g_tasks[Index].ready ) 
		{
		 	(*g_tasks[Index].p_task)();  //4 							//运行该任务
		 	g_tasks[Index].ready -= 1;   //4 							//减少ready标志
			if (g_tasks[Index].period == 0)
		    	delete_task(Index);
		}
   	}
}

/********************************************************************
增加任务，如果超出设定的任务数，返回错误
如果成功，返回当前任务号
********************************************************************/
unsigned char add_task(void( * p_task)(), const unsigned short delay, const unsigned short period)    
{
   	unsigned char Index = 0;
   
   	while ((g_tasks[Index].p_task) && (Index < MAX_TASK_NUM)) {    	// First find a gap in the array (if there is one)
		Index++;
      	} 
   
   	if (Index == MAX_TASK_NUM) {			   						//Have we reached the end of the list?   
	      return MAX_TASK_NUM;  
      }
         	
	g_tasks[Index].p_task  = p_task;								//If we're here, there is a space in the task array
	     
	g_tasks[Index].delay  = delay;
	g_tasks[Index].period = period;

	g_tasks[Index].ready  = 0;

	return Index; 													//return position of task (to allow later deletion)
}

/********************************************************************
删除任务，如果欲删除的任务不存在，返回FALSE
********************************************************************/
unsigned char delete_task(const unsigned char index) 
{
   	unsigned char rc;

   	if ( 0 == g_tasks[index].p_task ) {							//check if the task function is null
	//	rc = FALSE;
		rc =0;
    }
	else {
		rc = 1;
//		rc = TRUE;
   	}      
   
	g_tasks[index].p_task   = NULL;									//Clear all task info
	g_tasks[index].delay   = 0;
	g_tasks[index].period  = 0;
	g_tasks[index].ready  = 0;

   	return rc;       												// return status
}

/********************************************************************
  smt_sys_tick() 任务调度，被定时器中断调用
********************************************************************/
void Task_sys_tick(void) 
{
   	uint8 index ;

   	// 计算单位是ticks，而不是毫秒
	//任务调度
	for (index = 0; index < MAX_TASK_NUM; index++) {
		if (g_tasks[index].p_task) { 								//非空任务 
			if (g_tasks[index].delay == 0) {							//该运行了
				g_tasks[index].ready += 1;  						//增加运行标志			
				if (g_tasks[index].period) {
					g_tasks[index].delay = g_tasks[index].period; 	 //运行周期重新装入
				}
            }
		if(g_tasks[index].delay != 0)
			g_tasks[index].delay -= 1; 								//所有任务等待值减一
		}         
    }
} 

