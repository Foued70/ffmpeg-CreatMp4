#include <iostream>
#include "inRect.h"
using namespace std; 

// 计算 |p1 p2| X |p1 p|  
float GetCross(Point& p1, Point& p2,Point& p)  
{  
    return (p2.x - p1.x) * (p.y - p1.y) -(p.x - p1.x) * (p2.y - p1.y);  
}  
//判断点是否在5X5 以原点为左下角的正方形内（便于测试）

bool IsPointInMatrix(Point &p1,Point &p2,Point &p3,Point &p4,Point& p)
{  
    return GetCross(p1,p2,p) * GetCross(p3,p4,p) >= 0 && GetCross(p2,p3,p) * GetCross(p4,p1,p) >= 0;
    //return false;  
}  
