#include "stdafx.h"
#include <WeaselUI.h>
#include "WeaselPanel.h"

using namespace weasel;

class weasel::UIImpl {
public:
	WeaselPanel panel;

	UIImpl(weasel::UI &ui)
		: panel(ui), shown(false) {}
	~UIImpl()
	{
		// ensure destroy heap resources of panel
		panel.CleanUp();
	}
	void Refresh() {
		if (!panel.IsWindow()) return;
		if (timer)
		{
			Hide();
			KillTimer(panel.m_hWnd, AUTOHIDE_TIMER);
			timer = 0;
		}
		panel.Refresh();
	}
	void Show();
	void Hide();
	void ShowWithTimeout(DWORD millisec);
	bool IsShown() const { return shown; }

	static VOID CALLBACK OnTimer(
		  _In_  HWND hwnd,
		  _In_  UINT uMsg,
		  _In_  UINT_PTR idEvent,
		  _In_  DWORD dwTime
	);
	static const int AUTOHIDE_TIMER = 20121220;
	static UINT_PTR timer;
	bool shown;
};

UINT_PTR UIImpl::timer = 0;

void UIImpl::Show()
{
	if (!panel.IsWindow()) return;
	panel.ShowWindow(SW_SHOWNA);
	shown = true;
	if (timer)
	{
		KillTimer(panel.m_hWnd, AUTOHIDE_TIMER);
		timer = 0;
	}
}

void UIImpl::Hide()
{
	if (!panel.IsWindow()) return;
	panel.ShowWindow(SW_HIDE);
	shown = false;
	if (timer)
	{
		KillTimer(panel.m_hWnd, AUTOHIDE_TIMER);
		timer = 0;
	}
}

void UIImpl::ShowWithTimeout(DWORD millisec)
{
	if (!panel.IsWindow()) return;
	DLOG(INFO) << "ShowWithTimeout: " << millisec;
	panel.ShowWindow(SW_SHOWNA);
	shown = true;
	SetTimer(panel.m_hWnd, AUTOHIDE_TIMER, millisec, &UIImpl::OnTimer);
	timer = UINT_PTR(this);
}
VOID CALLBACK UIImpl::OnTimer(
  _In_  HWND hwnd,
  _In_  UINT uMsg,
  _In_  UINT_PTR idEvent,
  _In_  DWORD dwTime
)
{
	DLOG(INFO) << "OnTimer:";
	KillTimer(hwnd, idEvent);
	UIImpl* self = (UIImpl*)timer;
	timer = 0;
	if (self)
	{
		self->Hide();
		self->shown = false;
	}
}

bool UI::Create(HWND parent)
{
	if (pimpl_)
	{
		// re create panel cause destroied before
#ifdef USE_CAPTURE_BY_CLICK
		if(style_.capture_by_click)
			pimpl_->panel.Create(parent, 0, 0, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE  | WS_EX_LAYERED, 0U, 0);
		else
#endif
			pimpl_->panel.Create(parent, 0, 0, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT | WS_EX_LAYERED, 0U, 0);

		return true;
	}

	pimpl_ = new UIImpl(*this);
	if (!pimpl_)
		return false;

#ifdef USE_CAPTURE_BY_CLICK
	if(style_.capture_by_click)
		pimpl_->panel.Create(parent, 0, 0, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE  | WS_EX_LAYERED, 0U, 0);
	else
#endif
		pimpl_->panel.Create(parent, 0, 0, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT | WS_EX_LAYERED, 0U, 0);
	return true;
}

// for ending of composition
void UI::Destroy()
{
	if (pimpl_)
	{
		if (style().layout_type == UIStyle::LAYOUT_HORIZONTAL_FULLSCREEN || style().layout_type == UIStyle::LAYOUT_VERTICAL_FULLSCREEN)
		{
			ctx_.clear();
			Refresh();
		}
		// destroy panel not delete it, avoiding re initialization font resources
		if (pimpl_->panel.IsWindow())
		{
			pimpl_->panel.DestroyWindow();
		}
	}
}
// for ending of app, destroy pimpl_
void UI::DestroyAll()
{
	if (pimpl_)
	{
		if (pimpl_->panel.IsWindow())
			pimpl_->panel.DestroyWindow();
		delete pimpl_;
		pimpl_ = 0;
	}
}

void UI::Show()
{
	if (pimpl_)
	{
		pimpl_->Show();
	}
}

void UI::Hide()
{
	if (pimpl_)
	{
		pimpl_->Hide();
	}
}

void UI::ShowWithTimeout(DWORD millisec)
{
	if (pimpl_)
	{
		pimpl_->ShowWithTimeout(millisec);
	}
}

bool UI::IsCountingDown() const
{
	return pimpl_ && pimpl_->timer != 0;
}

bool UI::IsShown() const
{
	return pimpl_ && pimpl_->IsShown();
}

void UI::Refresh()
{
	if (pimpl_)
	{
		pimpl_->Refresh();
	}
}

void UI::UpdateInputPosition(RECT const& rc)
{
	if (pimpl_ && pimpl_->panel.IsWindow())
	{
		pimpl_->panel.MoveTo(rc);
	}
}

void UI::Update(const Context &ctx, const Status &status)
{
	ctx_ = ctx;
	status_ = status;
	Refresh();
}
