#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "termbox.h"

namespace termkit {

struct Vector2 {
  int x, y;
};

struct Rect {
  int x, y, w, h;

  int top() const { return y; }
  int right() const { return x + w - 1; }
  int bottom() const { return y + h - 1; }
  int left() const { return x; }

  bool Contains(const Rect& r) const {
    return top() <= r.top() && right() >= r.right() && bottom() >= r.bottom() &&
           left() <= r.left();
  }

  bool Contains(int x, int y) {
    return left() <= x && x <= right() && top() <= y && y <= bottom();
  }

  Rect Clone() const { return *this; }

  Rect& SetLocation(int x, int y) {
    this->x = x;
    this->y = y;
    return *this;
  }

  Rect& SetSize(int w, int h) {
    this->w = w;
    this->h = h;
    return *this;
  }

  Rect& SetBounds(int x, int y, int w, int h) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    return *this;
  }

  Rect& Translate(int dx, int dy) {
    x += dx;
    y += dy;
    return *this;
  }
};

using Charactor = uint32_t;
using Color = uint16_t;

class Backend {
 public:
  virtual ~Backend() = default;
  virtual void Initialize() = 0;
  virtual void Shutdown() = 0;
  virtual void ChangeCell(int x, int y, Charactor ch, Color fg, Color bg) = 0;
  virtual void Clear() = 0;
  virtual void Present() = 0;
  virtual int GetWidth() = 0;
  virtual int GetHeight() = 0;
};

class TermboxBackend final : public Backend {
 public:
  void Initialize() override { tb_init(); }
  void Shutdown() override { tb_shutdown(); }
  void ChangeCell(int x, int y, Charactor ch, Color fg, Color bg) override {
    tb_change_cell(x, y, ch, fg, bg);
  }
  void Clear() override { tb_clear(); }
  void Present() override { tb_present(); }
  int GetWidth() override { return tb_width(); }
  int GetHeight() override { return tb_height(); }

  static const std::shared_ptr<TermboxBackend>& instance() {
    static std::shared_ptr<TermboxBackend> p;
    if (p == nullptr) {
      p.reset(new TermboxBackend());
    }
    return p;
  }

 private:
  TermboxBackend() = default;
  TermboxBackend(const TermboxBackend&) = delete;
  TermboxBackend& operator=(const TermboxBackend&) = delete;
  TermboxBackend(TermboxBackend&&) = delete;
};

class Screen {
 public:
  Screen(const std::shared_ptr<Backend>& backend)
      : Screen(backend, Rect{0, 0, backend->GetWidth(), backend->GetHeight()}) {
  }

  Screen(const std::shared_ptr<Backend>& backend, Rect region)
      : backend_(backend),
        region_(region),
        region_by_origin_(region.Clone().SetLocation(0, 0)) {}

  Screen Clone() const { return *this; }

  Screen CreateSubscreen(Rect rel_region) const {
    // assert(region_.Clone().SetLocation(0, 0).Contains(rel_region));
    return Screen(backend_, region_.Clone()
                                .Translate(rel_region.x, rel_region.y)
                                .SetSize(rel_region.w, rel_region.h));
  }

  bool Put(int x, int y, Charactor ch, Color fg, Color bg) {
    if (!region_by_origin_.Contains(x, y)) {
      return false;
    }
    backend_->ChangeCell(region_.x + x, region_.y + y, ch, fg, bg);
    return true;
  }

  int width() const { return region_.w; }
  int height() const { return region_.h; }

 private:
  std::shared_ptr<Backend> backend_;
  Rect region_;
  Rect region_by_origin_;
};

class Renderer {
 public:
  static constexpr Color kDefaultFg = TB_DEFAULT;
  static constexpr Color kDefaultBg = TB_DEFAULT;
  static constexpr Charactor kDefaultCh = ' ';
  static const Vector2 kDefaultPos;

  Renderer(Screen& screen)
      : screen_(screen),
        pos_(Renderer::kDefaultPos),
        fg_(Renderer::kDefaultFg),
        bg_(Renderer::kDefaultBg),
        ch_(Renderer::kDefaultCh) {}

  Renderer Clone() const { return *this; }

  Renderer& Reset() {
    fg_ = Renderer::kDefaultFg;
    bg_ = Renderer::kDefaultBg;
    ch_ = Renderer::kDefaultCh;
    pos_ = Renderer::kDefaultPos;
    return *this;
  }

  Renderer& Fg(Color fg = Renderer::kDefaultFg) {
    fg_ = fg;
    return *this;
  }

  Renderer& Bg(Color bg = Renderer::kDefaultBg) {
    bg_ = bg;
    return *this;
  }

  Renderer& Ch(Charactor ch = Renderer::kDefaultCh) {
    ch_ = ch;
    return *this;
  }

  Renderer& Pos(const Vector2& pos = Renderer::kDefaultPos) {
    pos_ = pos;
    return *this;
  }

  Renderer& Offset(int dx, int dy) {
    pos_.x += dx;
    pos_.y += dy;
    return *this;
  }

  Renderer& Put() {
    screen_.Put(pos_.x, pos_.y, ch_, fg_, bg_);
    return *this;
  }

  Renderer& Put(Charactor ch) {
    ch_ = ch;
    screen_.Put(pos_.x, pos_.y, ch_, fg_, bg_);
    return *this;
  }

  Renderer& Put(Vector2 pos, Charactor ch) {
    pos_ = pos;
    ch_ = ch;
    screen_.Put(pos_.x, pos_.y, ch_, fg_, bg_);
    return *this;
  }

  Renderer& HLine(int len, Charactor ch) {
    ch_ = ch;
    int sign = len > 0 ? 1 : -1;
    int abs_len = len * sign;
    for (int i = 0; i < abs_len; ++i) {
      if (!screen_.Put(pos_.x + i * sign, pos_.y, ch_, fg_, bg_)) {
        break;
      }
    }
    pos_.x += len;
    return *this;
  }

  Renderer& HLine(int len) { return HLine(len, ch_); }

  Renderer& VLine(int len, Charactor ch) {
    ch_ = ch;
    int sign = len > 0 ? 1 : -1;
    int abs_len = len * sign;
    for (int i = 0; i < abs_len; ++i) {
      if (!screen_.Put(pos_.x, pos_.y + i * sign, ch_, fg_, bg_)) {
        break;
      }
    }
    pos_.y += len;
    return *this;
  }

  Renderer& VLine(int len) { return VLine(len, ch_); }

  Renderer& Text(const std::string& text) {
    auto len = text.size();
    for (int i = 0; i < len; ++i) {
      if (!screen_.Put(pos_.x + i, pos_.y, text[i], fg_, bg_)) {
        break;
      }
    }
    pos_.x += len;
    return *this;
  }

 private:
  Screen& screen_;
  Vector2 pos_;
  Color fg_, bg_;
  Charactor ch_;
};

const Vector2 Renderer::kDefaultPos = {0, 0};

struct KeyEvent {
  uint8_t mod;
  uint16_t key;
  Charactor ch;
};

#if 0
struct Event {
  enum class Type { kKey = 1, kResize, kMouse };
  Type type;
  uint8_t mod;
  uint16_t key;
  uint32_t ch;
  int32_t w;
  int32_t h;
  int32_t x;
  int32_t y;
};

struct MouseEvent {
  uint8_t mod;
  int32_t x;
  int32_t y;
};

struct ResizeEvent {
  int32_t w;
  int32_t h;
};

class EventHandler {
 public:
  virtual ~EventHandler() = default;
  virtual void OnKeyEvent(const KeyEvent& e) = 0;
  virtual void OnMouseEvent(const MouseEvent& e) = 0;
  virtual void OnResizeEvent(const ResizeEvent& e) = 0;
};

void DispatchEvent(const Event& e, EventHandler& handler) {
  switch (e.type) {
    case Event::Type::kKey: {
      handler.OnKeyEvent({e.mod, e.key, e.ch});
      return;
    }
    case Event::Type::kMouse: {
      handler.OnMouseEvent({e.mod, e.x, e.y});
      return;
    }
    case Event::Type::kResize: {
      handler.OnResizeEvent({e.w, e.h});
      return;
    }
  }
  std::abort();
}

namespace view {

class View {
 public:
  virtual ~View() = defualt;
  virtual void Render(const Screen& screen) = 0;
};

class ViewManager {
 public:
  void HandleEvent(const Event& e) {}
  void RequestRedraw() { needs_redraw_ = true; }
  void Render() { root_view_->Render(screen_); }

 private:
  Screen screen_;
  std::shared_ptr<View> root_view_;
  bool needs_redraw_;
};

class CommonView : public EventHandler {
 public:
  virtual ~CommonView() = default;
  virtual void OnKeyEvent(const KeyEvent& e) override { (void)e; }
  virtual void OnMouseEvent(const MouseEvent& e) override { (void)e; }
  virtual void OnResizeEvent(const ResizeEvent& e) override { (void)e; }
  virtual bool NeedsUpdate() = 0;
  virtual void Update(const Screen& screen) = 0;

 protected:
  void RequestUpdate() { needs_update_ = true; }

 private:
  bool needs_update_;
};

}  // namespace view
#endif

}  // namespace termkit

class Box {
 public:
  void Render(termkit::Screen screen) {
    for (int y = 0; y < screen.height(); ++y) {
      for (int x = 0; x < screen.width(); ++x) {
        screen.Put(x, y, '*', TB_DEFAULT, TB_BLUE);
      }
    }
  }
};

class TextBox {
 public:
  void OnKeyEvent(termkit::KeyEvent e) {
    if (e.ch != 0) {
      Utf8Ch uc;
      uc.size = tb_utf8_unicode_to_char(uc.buf, e.ch);
      text_.push_back(std::move(uc));
    } else {
      switch (e.key) {
        case TB_KEY_BACKSPACE:
        case TB_KEY_BACKSPACE2: {
          text_.pop_back();
          break;
        }
        default: {
          // TODO
        }
      }
    }
  }

  void Render(termkit::Screen screen) {
    int x = 0;
    int y = 0;
    for (auto& uc : text_) {
      for (int i = 0; i < uc.size; ++i) {
        if (!screen.Put(x, y, uc.buf[i], TB_DEFAULT, TB_DEFAULT)) {
          x = 0;
          ++y;
          if (!screen.Put(x, y, uc.buf[i], TB_DEFAULT, TB_DEFAULT)) {
            return;
          }
        }
        ++x;
      }
    }
  }

 private:
  struct Utf8Ch {
    constexpr static int kMaxBufSize = 6;
    char buf[kMaxBufSize];
    int size;
    Utf8Ch() : buf{}, size(0) {}
  };

  std::vector<Utf8Ch> text_;
};

int main(int, char**) {
  auto backend = termkit::TermboxBackend::instance();
  backend->Initialize();
  bool end = false;
  termkit::Screen screen(backend);
  TextBox box;
  while (!end) {
    backend->Clear();
    box.Render(screen);
    // termkit::Renderer r(screen);
    // Box box;
    // box.Render(screen.CreateSubscreen(termkit::Rect{x, 10, 10, 10}));
    backend->Present();

    tb_event ev;
    auto ev_type = tb_peek_event(&ev, 10);
    switch (ev_type) {
      case -1: {
        // error
        break;
      }
      case 0: {
        // timeout
        break;
      }
      case TB_EVENT_KEY: {
        if (ev.key == TB_KEY_ESC) {
          end = true;
          break;
        }
        box.OnKeyEvent({ev.mod, ev.key, ev.ch});
        break;
      }
      case TB_EVENT_RESIZE: {
        break;
      }
      case TB_EVENT_MOUSE: {
        break;
      }
      default: { std::abort(); }
    }
  }
  backend->Shutdown();
  return 1;
}

#if 0
namespace tmp {

struct Vector2 {
  int x, y;
};

class View {
 public:
  int width() const { return size_.x; }
  int height() const { return size_.y; }

  virtual void Set(int x, int y, uint16_t ch, uint32_t fb, uint32_t bg) {}

 private:
  Vector2 size_;
};

class SubView : public View {
 public:
  int offset_x() const { return offset_.x; }
  int offset_y() const { return offset_.y; }

  virtual void Set(int x, int y, uint16_t ch, uint32_t fb,
                   uint32_t bg) override {}

 private:
  Vector2 offset_;
};

}  // namespace tmp

namespace gui {

struct Cell {
  int x, y;
  uint16_t ch;
  uint32_t fg, bg;
};

struct DrawContext {
  int width, height;
};

class View {
 public:
  virtual ~View() = default;
  virtual std::vector<Cell> Draw(const DrawContext& ctx) = 0;
};

class Layer : public View {
 public:
  std::vector<Cell> Draw(const DrawContext& ctx) override {
    std::vector<Cell> cells;
    for (auto& view : views_) {
      auto r = view->Draw(ctx);
      cells
    }
  }

  template <class ViewT, class... Args>
  std::vector<Cell> Add(Args&&... args) {
    views_.emplace_back(
        std::unique_ptr<ViewT>(new ViewT(std::forward<Args>(args)...)));
  }

 private:
  std::vector<std::unique_ptr<View>> views_;
};

}  // namespace gui

class Cell {
 public:
  Cell(uint32_t ch, uint16_t fg, uint16_t bg) : tb_({ch, fg, bg}) {}
  Cell(uint32_t ch) : Cell(ch, TB_DEFAULT, TB_DEFAULT) {}

  const tb_cell& AsTB() const { return tb_; }

 private:
  tb_cell tb_;
};

void Draw(int x, int y, const std::string& str) {
  tb_width();
  for (size_t i = 0; i < str.size(); ++i) {
    tb_change_cell(x + i, y, str[i], TB_DEFAULT, TB_DEFAULT);
  }
}

void DrawVerticalLine(int x, int src_y, int dst_y, Cell cell) {
  for (int y = src_y; y <= dst_y; ++y) {
    tb_put_cell(x, y, &(cell.AsTB()));
  }
}

void DrawHorizontalLine(int y, int src_x, int dst_x, Cell cell) {
  for (int x = src_x; x <= dst_x; ++x) {
    tb_put_cell(x, y, &(cell.AsTB()));
  }
}

int main(int, char**) {
  tb_init();

  while (true) {
    tb_event ev;
    auto ev_type = tb_peek_event(&ev, 10);
    switch (ev_type) {
      case TB_EVENT_KEY: {
        break;
      }
      case TB_EVENT_RESIZE: {
        break;
      }
      case TB_EVENT_MOUSE: {
        break;
      }
      default: { std::abort(); }
    }
  }

  tb_shutdown();
  return 0;
}
#endif
