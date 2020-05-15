/* 2019 04 10 */
/* By hxdyxd */

#ifndef _USER_MAIN_H_
#define _USER_MAIN_H_

#define SYNC_TIMEOUT   (100)



#define LCD_PAGE_OUTPUT_INFO    (0)
#define LCD_PAGE_SET            (1)
#define LCD_PAGE_PKT            (2)
#define LCD_PAGE_VER            (3)
#define LCD_PAGE_NUM            (4)



#define LCD_PAGE_SET_DEFAULT   (0)
#define LCD_PAGE_SET_VOLTAGE   (1)
#define LCD_PAGE_SET_CURRENT   (2)
#define LCD_PAGE_SET_NUM       (3)




void user_system_setup(void);
void user_setup(void);
void user_loop(void);


#endif
/*****************************END OF FILE***************************/
