#include "pch.h"
#include "CViewTransform.h"

namespace {
const double kDefaultScale = 1.0;
const double kMinScale = 1e-9;
}

//构造视图变换对象并初始化默认参数
CViewTransform::CViewTransform() : m_scale(kDefaultScale), m_offsetX(0.0), m_offsetY(0.0) {}

//设置绘图区屏幕矩形
void CViewTransform::SetScreenRect(const CRect& rect) { m_screenRect = rect; }

//获取绘图区屏幕矩形
CRect CViewTransform::GetScreenRect() const { return m_screenRect; }
// return: CRect:当前绘图区屏幕矩形;

//围绕指定屏幕点执行缩放
void CViewTransform::Zoom(double factor, const CPoint& screenCenter) {
    if (factor <= 0.0) return;

    Point2D worldCenter = ScreenToWorld(screenCenter);
    m_scale *= factor;
    if (m_scale < kMinScale) m_scale = kMinScale;
    Point2D newWorldCenter = ScreenToWorld(screenCenter);
    m_offsetX += (newWorldCenter.x - worldCenter.x);
    m_offsetY += (newWorldCenter.y - worldCenter.y);
}

//按屏幕像素增量平移视图
void CViewTransform::Pan(int deltaX, int deltaY) {
    m_offsetX += deltaX / m_scale;
    m_offsetY -= deltaY / m_scale;
}

//world->screen
CPoint CViewTransform::WorldToScreen(const Point2D& pt) const {
    int sx = static_cast<int>((pt.x + m_offsetX) * m_scale);
    int sy = m_screenRect.Height() - static_cast<int>((pt.y + m_offsetY) * m_scale);
    return CPoint(sx, sy);
}
// return: CPoint: processed screen;

//将屏幕坐标转换为世界坐标
Point2D CViewTransform::ScreenToWorld(const CPoint& pt) const {
    double wx = pt.x / m_scale - m_offsetX;
    double wy = (m_screenRect.Height() - pt.y) / m_scale - m_offsetY;
    return Point2D(wx, wy);
}
// return: Point2D: processed world;
