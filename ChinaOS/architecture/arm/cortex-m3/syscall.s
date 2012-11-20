;/******************************************************Copyright (c)**************************************************
;**                                              胆怯是阻止一切可能的根本缘由
;**
;**                                             E-Mail: ChinaFengliang@163.com
;**
;**---------File Information-------------------------------------------------------------------------------------------
;** File name:            system.s
;** Last version:         V1.00
;** Descriptions:         系统调用文件.
;** Hardware platform:    Cortex-M3系列
;** SoftWare platform:    ChinaOS
;**
;**--------------------------------------------------------------------------------------------------------------------
;** Created by:           FengLiang
;** Created date:         2010年8月26日  17:12:13
;** Version:              V1.00
;** Descriptions:         The original version
;**
;**--------------------------------------------------------------------------------------------------------------------
;** Modified by:
;** Modified date:
;** Version:
;** Descriptions:
;**
;*********************************************************************************************************************/
    AREA    system_call, CODE, READONLY

;/*********************************************************************************************************************
;                                                   接口申明区
;*********************************************************************************************************************/
;/* 导出接口 --------------------------------------------------------------------------------------------------------*/
    EXPORT      syscall_exception                                           ;/* 系统调用中断服务                     */
    EXPORT      pendsv_exception                                            ;/* pendsv中断服务                       */    

;/* 导入接口 --------------------------------------------------------------------------------------------------------*/
    IMPORT      OS_This;pThreadThis                                                 ;/* 当前运行的线程                       */
    IMPORT      shift_thread                                                ;/* 线程交替                             */
    
;/*********************************************************************************************************************
;                                                   宏定义区
;*********************************************************************************************************************/
;/* 寄存器地址 ------------------------------------------------------------------------------------------------------*/
ICSR            EQU                 0xE000ED04                              ;/* 中断控制及状态寄存器ICSR             */

;/*********************************************************************************************************************
;** Function name:           static_service_entry
;** Descriptions:            系统调用服务入口
;** Input parameters:        所有参数放在栈顶位置
;** Output parameters:       
;** Returned value:          
;**--------------------------------------------------------------------------------------------------------------------
;** Created by:              Fengliang
;** Created Date:            2010-8-26  17:22:54
;** Test recorde:
;**--------------------------------------------------------------------------------------------------------------------
;** Modified by:
;** Modified date:
;** Test recorde:
;*********************************************************************************************************************/
syscall_exception
        ;/* 获取栈顶地址至R0 */
        TST     LR,     #4
        ITE     EQ
        MRSEQ   R3,     MSP
        MRSNE   R3,     PSP

		;/* 此时R3保存栈顶指针 */

		;/* 由于硬件自动将R0~R3压入栈,所以参数应该从栈里取出 */
		LDR     R0, 	[R3]												;/* 读取地址R0变量的值   				*/		
		LDR     R1,     [R3, #4]											;/* 读取地址R1变量的值   				*/


        CMP.W   R0,     #8                                                  ;/* 限定最大服务数目: [0,8]             */
        BXHI    LR
        TBH.W   [PC,    R0, LSL #1]

static_service_table                                                        ;/*------------- 服务列表 --------------*/
        DCI     ((static_thread_switch - static_service_table)/2)           ;/* 0号服务: 线程切换                   */
        DCI     ((static_activate_system - static_service_table)/2)         ;/* 1号服务: 激活操作系统               */
		DCI     ((static_system_restart - static_service_table)/2)          ;/* 2号服务: 系统重启                   */
        DCI     ((static_atom_operate_lock - static_service_table)/2)       ;/* 3号服务: 原子操作开始               */
        DCI     ((static_atom_operate_unlock - static_service_table)/2)     ;/* 4号服务: 原子操作结束               */
        
;/*********************************************************************************************************************
;** Function name:           void static_thread_switch(void)
;** Descriptions:            线程切换(函数主动要求线程切换入口)
;** Input parameters:        None
;** Output parameters:       None
;** Returned value:          None
;**--------------------------------------------------------------------------------------------------------------------
;** Created by:              Fengliang
;** Created Date:            2010-9-2  9:19:42
;** Test recorde:
;**--------------------------------------------------------------------------------------------------------------------
;** Modified by:
;** Modified date:
;** Test recorde:
;*********************************************************************************************************************/
static_thread_switch
        ;/* 悬起pendsv中断,以进入pendsv切换线程情景 */
        LDR     r0, =ICSR
        LDR     r1, =0x10000000                                             ;/* PENDSVSET位                         */
        STR     r1, [r0]                                                    ;/* 置位ICSR寄存器的PENDSVSET位         */
        BX      lr                                                          ;/* 返回                                */
;/*********************************************************************************************************************
;** Function name:           static_thread_switch
;** Descriptions:            线程切换(函数主动要求线程切换入口)
;** Input parameters:        None
;** Output parameters:       None
;** Returned value:          None
;**--------------------------------------------------------------------------------------------------------------------
;** Created by:              Fengliang
;** Created Date:            2010-9-2  9:19:42
;** Test recorde:
;**--------------------------------------------------------------------------------------------------------------------
;** Modified by:
;** Modified date:
;** Test recorde:
;*********************************************************************************************************************/
pendsv_exception
        ;/* 保存this线程的运行环境 */
        MRS     r1, psp                                                     ;/* get from thread stack pointer        */
        STMFD   r1!, {r4 - r11}                                             ;/* push r4 - r11 register               */
        MOV		r0, r1														;/* 修正: SP               				 */
        
        B       update_thread

;/*********************************************************************************************************************
;** Function name:          void static_activate_system(void)
;** Descriptions:           激活操作系统
;** Input parameters:       
;** Output parameters:      
;** Returned value:         R0 : 即将登场线程的栈顶指针
;**--------------------------------------------------------------------------------------------------------------------
;** Created by:
;** Created Date:            2010-9-2  14:19:45
;** Test recorde:
;**--------------------------------------------------------------------------------------------------------------------
;** Modified by:
;** Modified date:
;** Test recorde:
;*********************************************************************************************************************/
static_activate_system
		;/* 
		; * 栈寄存器: MSP线程栈
		; * 权限模式: 特权权限
		; */
		MOV		R0, #0x0
		MSR		CONTROL,	R0 

        LDR     R0, =OS_This;pThreadThis
        LDR     R0, [R0]
        LDR     R0, [R0]
				
update_thread      
        BL      shift_thread
;/*********************************************************************************************************************
;** Function name:           void regist_recover(int R0)
;** Descriptions:            将新线程环境从堆栈中恢复到的现场
;** Input parameters:        R0 : 即将登场线程的栈顶指针
;** Output parameters:
;** Returned value:          
;**--------------------------------------------------------------------------------------------------------------------
;** Created by:
;** Created Date:            
;** Test recorde:
;**--------------------------------------------------------------------------------------------------------------------
;** Modified by:
;** Modified date:
;** Test recorde:
;*********************************************************************************************************************/
regist_recover
		;/* 异常返回线程模式,并使用线程堆栈PSP寄存器 */
		MOV		LR,		#0xFFFFFFFD
        ;LDR     r0, [r0]                                                   ;/* load thread stack pointer            */
        LDMFD   r0!, {r4 - r11}                                             ;/* pop r4 - r11 register                */
        MSR     psp, r0                                                     ;/* 更新线程堆栈                         */
		BX		LR
    
;/*********************************************************************************************************************
;** Function name:           int static_atom_operate_lock(void)
;** Descriptions:            原子操作开始
;** Input parameters:        
;** Output parameters:       
;** Returned value:          中断状态控制字
;**--------------------------------------------------------------------------------------------------------------------
;** Created by:              Fengliang
;** Created Date:            2010-8-26  17:32:9
;** Test recorde:
;**--------------------------------------------------------------------------------------------------------------------
;** Modified by:
;** Modified date:
;** Test recorde:
;*********************************************************************************************************************/
static_atom_operate_lock        PROC
		EXPORT		static_atom_operate_lock
        MRS     r0,  	PRIMASK                                             ;/* 拷贝中断控制状态                     */
        CPSID   I															;/* PRIMASK=1; 关中断					 */
		BX      LR
        ENDP
    
;/*********************************************************************************************************************
;** Function name:           void static_atom_operate_unlock(int R0)
;** Descriptions:            原子操作结束
;** Input parameters:        R0: 中断状态控制字
;** Output parameters:       
;** Returned value:          
;**--------------------------------------------------------------------------------------------------------------------
;** Created by:              Fengliang
;** Created Date:            2010-8-26  17:33:13
;** Test recorde:
;**--------------------------------------------------------------------------------------------------------------------
;** Modified by:
;** Modified date:
;** Test recorde:
;*********************************************************************************************************************/
static_atom_operate_unlock      PROC
		EXPORT	static_atom_operate_unlock
        MSR     PRIMASK,  R0                                                ;/* 恢复中断控制状态                     */             
        BX      LR
        ENDP

;/*********************************************************************************************************************
;** Function name:           static_system_restart
;** Descriptions:            系统重启
;** Input parameters:        
;** Output parameters:       
;** Returned value:          
;**--------------------------------------------------------------------------------------------------------------------
;** Created by:              Fengliang
;** Created Date:            2010-8-26  17:33:13
;** Test recorde:
;**--------------------------------------------------------------------------------------------------------------------
;** Modified by:
;** Modified date:
;** Test recorde:
;*********************************************************************************************************************/
static_system_restart			PROC

		ENDP
		ALIGN

        END                                                                 ;/* 代码末端                             */
;/*********************************************************************************************************************
;                                                    END OF FILE
;*********************************************************************************************************************/

