#include "pch.h"
#include "framework.h"
#include "CAD.h"
#include "CADDlg.h"

// 渲染模块 / rendering module

namespace {
const COLORREF kCadColorWhite = RGB(255, 255, 255);
const COLORREF kCadColorRed = RGB(255, 0, 0);
const COLORREF kCadColorYellow = RGB(255, 255, 0);
const COLORREF kCadColorGreen = RGB(0, 255, 0);
const COLORREF kCadColorCyan = RGB(0, 255, 255);
const COLORREF kCadColorBlue = RGB(0, 0, 255);
const COLORREF kCadColorMagenta = RGB(255, 0, 255);

const COLORREF kCadColorDarkPanel = RGB(33, 33, 33);
const COLORREF kCadColorFrameDark = RGB(30, 30, 30);
const COLORREF kCadColorFrameLight = RGB(220, 220, 220);
const COLORREF kCadColorSquareBorder = RGB(20, 20, 20);

const COLORREF kCadColorAboutPen = RGB(0, 85, 170);
const COLORREF kCadColorAboutFill = RGB(0, 122, 204);

const int kPaletteInnerMargin = 2;
const int kAboutIconInnerMargin = 1;
const int kAboutFontHeight = 11;

// 功能：根据按钮 ID 取对应调色板颜色。
bool TryGetPaletteColor(int ctrlId, COLORREF& color) {
    switch (ctrlId) {
    case IDC_COLOR_WHITE: color = kCadColorWhite; return true;
    case IDC_COLOR_RED: color = kCadColorRed; return true;
    case IDC_COLOR_YELLOW: color = kCadColorYellow; return true;
    case IDC_COLOR_GREEN: color = kCadColorGreen; return true;
    case IDC_COLOR_CYAN: color = kCadColorCyan; return true;
    case IDC_COLOR_BLUE: color = kCadColorBlue; return true;
    case IDC_COLOR_MAGENTA: color = kCadColorMagenta; return true;
    default: return false;
    }
}
}

// 功能：自绘按钮（颜色按钮与关于图标按钮）。
void CCADDlg::DrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {
    COLORREF paletteColor = kCadColorFrameDark;
    if (TryGetPaletteColor(nIDCtl, paletteColor)) {
        CDC dc;
        dc.Attach(lpDrawItemStruct->hDC);

        CRect rc = lpDrawItemStruct->rcItem;
        dc.FillSolidRect(&rc, GetSysColor(COLOR_3DFACE));

        dc.Draw3dRect(&rc, kCadColorFrameDark, kCadColorFrameLight);

        CRect square = rc;
        square.DeflateRect(kPaletteInnerMargin, kPaletteInnerMargin);
        CBrush brush(paletteColor);
        CPen pen(PS_SOLID, 1, kCadColorSquareBorder);
        CPen* oldPen = dc.SelectObject(&pen);
        CBrush* oldBrush = dc.SelectObject(&brush);
        dc.Rectangle(&square);
        dc.SelectObject(oldBrush);
        dc.SelectObject(oldPen);

        if ((lpDrawItemStruct->itemState & ODS_FOCUS) != 0) {
            dc.DrawFocusRect(&rc);
        }

        dc.Detach();
        return;
    }

    if (nIDCtl == IDC_ABOUT_ICON) {
        CDC dc;
        dc.Attach(lpDrawItemStruct->hDC);

        CRect rc = lpDrawItemStruct->rcItem;
        dc.FillSolidRect(&rc, GetSysColor(COLOR_3DFACE));

        CRect circleRc = rc;
        circleRc.DeflateRect(kAboutIconInnerMargin, kAboutIconInnerMargin);

        CPen pen(PS_SOLID, 1, kCadColorAboutPen);
        CBrush brush(kCadColorAboutFill);
        CPen* oldPen = dc.SelectObject(&pen);
        CBrush* oldBrush = dc.SelectObject(&brush);
        dc.Ellipse(&circleRc);
        dc.SelectObject(oldBrush);
        dc.SelectObject(oldPen);

        int oldBkMode = dc.SetBkMode(TRANSPARENT);
        COLORREF oldTextColor = dc.SetTextColor(kCadColorWhite);

        CFont font;
        font.CreateFont(kAboutFontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));
        CFont* oldFont = dc.SelectObject(&font);

        dc.DrawText(_T("?"), &circleRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        dc.SelectObject(oldFont);
        dc.SetTextColor(oldTextColor);
        dc.SetBkMode(oldBkMode);

        if ((lpDrawItemStruct->itemState & ODS_FOCUS) != 0) {
            dc.DrawFocusRect(&rc);
        }

        dc.Detach();
        return;
    }

    UNREFERENCED_PARAMETER(lpDrawItemStruct);
}

// 功能：统一绘制 CAD 画布（双缓冲），减少闪烁。
// 交互步骤（OnPaint）：
// 1) 先在内存 DC 里绘制背景和模型（draw to memory DC）。
// 2) 再一次性拷贝到屏幕（blit to screen）。
// 3) 这样界面更稳定，拖动和预览时不会明显闪烁。
void CCADDlg::OnPaint() {
    CPaintDC dc(this);

    CRect rect = m_transform.GetScreenRect();
    if (rect.IsRectEmpty()) return;

    CDC memDC;
    memDC.CreateCompatibleDC(&dc);
    CBitmap memBitmap;
    memBitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
    CBitmap* pOldBmp = memDC.SelectObject(&memBitmap);

    memDC.FillSolidRect(0, 0, rect.Width(), rect.Height(), kCadColorDarkPanel);

    DrawModel(&memDC);
    DrawPreview(&memDC);
    DrawSelection(&memDC);
    DrawCursor(&memDC);

    dc.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);
    memDC.SelectObject(pOldBmp);
}

// 功能：窗口尺寸变化时，同步更新绘图区矩形。
void CCADDlg::OnSize(UINT nType, int cx, int cy) {
    CDialogEx::OnSize(nType, cx, cy);

    if (GetDlgItem(IDC_DRAW_AREA)) {
        CRect rect;
        GetDlgItem(IDC_DRAW_AREA)->GetWindowRect(&rect);
        ScreenToClient(&rect);
        m_transform.SetScreenRect(rect);
        RefreshCanvas();
    }
}

