#pragma once

#include "Point2D.h"
#include <afxwin.h>

// 视图坐标变换 / world-screen transform
class CViewTransform {
private:
    double m_scale;
    double m_offsetX;
    double m_offsetY;
    CRect m_screenRect;

public:
    //构造默认变换参数
    CViewTransform();

    //设置屏幕矩形
    void SetScreenRect(const CRect& rect);
    //获取屏幕矩形
    CRect GetScreenRect() const;

    //执行缩放
    void Zoom(double factor, const CPoint& screenCenter);
    //执行平移
    void Pan(int deltaX, int deltaY);

    //世界坐标转屏幕坐标
    CPoint WorldToScreen(const Point2D& pt) const;
    //屏幕坐标转世界坐标
    Point2D ScreenToWorld(const CPoint& pt) const;
};
