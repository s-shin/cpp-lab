#include <memory>
#include <string>
#include <vector>

#include "termbox.h"

class Cell {
public:
  Cell(uint32_t ch, uint16_t fg, uint16_t bg): tb_({ch, fg, bg}) {}
  Cell(uint32_t ch): Cell(ch, TB_DEFAULT, TB_DEFAULT) {}

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
  tb_clear();
  DrawHorizontalLine(10, 0, 10, Cell('='));
  tb_present();
  tb_event ev;
  tb_poll_event(&ev);
  tb_shutdown();
  return 0;
}
