#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"

/* Linux/Mac 专用头文件 */
#include <editline/readline.h>
#include <editline/history.h>

int main(int argc, char** argv) {

  puts("Lispy Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit\n");

  while (1) {

    /* 输出提示符并获取输入 */
    char* input = readline("lispy> ");

    /* 添加到历史记录 */
    add_history(input);

    /* 回显输入 */
    printf("No you're a %s\n", input);

    /* 释放内存 */
    free(input);

  }

  return 0;
}
