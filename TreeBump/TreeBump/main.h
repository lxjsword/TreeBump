#ifndef MAIN_H
#define MAIN_H

// 初始化
void Init(void);

// 绘制
void Display(void);

// 窗口大小改变
void Reshape(int w, int h);

// 键盘响应
void Keyboard(unsigned char key, int x, int y);

int main(int argc, char** argv);

#endif	//MAIN_H