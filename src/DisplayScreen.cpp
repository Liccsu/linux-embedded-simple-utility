/*
 * Created by Administrator on 2023/8/23.
 */

#include <cstring>
#include <thread>
#include <ctime>
#include "DisplayScreen.hpp"


DisplayScreen::DisplayScreen(long _frame_rate, sync_mode_t _sync_mode) : rotation(0), fix(), var(),
                                                                         frame_rate(_frame_rate),
                                                                         sync_mode(_sync_mode),
                                                                         render_timer(Timer()),
                                                                         frame_timer(Timer()),
                                                                         ft_lib(), ft_face(),
                                                                         thread_pool(20) {
    atexit(DisplayScreen::destroy);
    this->fb_device = getenv("TSLIB_FBDEVICE") ? getenv("TSLIB_FBDEVICE") : DEFAULT_FB_DEVICE;

    this->fb_fd = open(this->fb_device.c_str(), O_RDWR);
    if (this->fb_fd == -1) {
        LOG(LOG_ERR, "open fb device failed\n");
        throw std::runtime_error("open fb device failed\n");
    }

    if (ioctl(this->fb_fd, FBIOGET_FSCREENINFO, &this->fix) < 0) {
        LOG(LOG_ERR, "ioctl FBIOGET_FSCREENINFO failed\n");
        close(this->fb_fd);
        throw std::runtime_error("ioctl FBIOGET_FSCREENINFO failed\n");
    }

    if (ioctl(this->fb_fd, FBIOGET_VSCREENINFO, &this->var) < 0) {
        LOG(LOG_ERR, "ioctl FBIOGET_VSCREENINFO failed\n");
        close(this->fb_fd);
        throw std::runtime_error("ioctl FBIOGET_VSCREENINFO failed\n");
    }

    if (this->rotation & 1) {
        /* 1 or 3 */
        this->yres = this->var.xres;
        this->xres = this->var.yres;
    } else {
        /* 0 or 2 */
        this->xres = this->var.xres;
        this->yres = this->var.yres;
    }

    this->fbuffer = static_cast<uint32_t *>(mmap(nullptr,
                                                 this->fix.smem_len,
                                                 PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED,
                                                 this->fb_fd,
                                                 0));

    if (this->fbuffer == (uint32_t * ) - 1) {
        LOG(LOG_ERR, "mmap framebuffer failed.\n");
        close(this->fb_fd);
        throw std::runtime_error("mmap framebuffer failed\n");
    }
    memset(this->fbuffer, 0, this->fix.smem_len);

    this->bytes_per_pixel = (this->var.bits_per_pixel + 7) / 8;
    this->display_fbuffer = this->fbuffer;
    this->render_fbuffer = this->fbuffer + this->yres * this->var.xres_virtual;
    this->var.xoffset = 0;
    this->var.yoffset = 0;
    ioctl(this->fb_fd, FBIOPAN_DISPLAY, &this->var);

    if (this->sync_mode) {
        this->frame_interval_time = static_cast<time_t>((1000 * 1000) / this->frame_rate);
    }

    this->ft_error = FT_Init_FreeType(&this->ft_lib);
    if (this->ft_error != FT_Err_Ok) {
        LOG(LOG_ERR, FT_Error_String(this->ft_error));
        FT_Done_FreeType(this->ft_lib);
        close(this->fb_fd);
        throw std::runtime_error(FT_Error_String(this->ft_error));
    }

    this->ft_error = FT_New_Memory_Face(this->ft_lib, reinterpret_cast<const FT_Byte *>(font_v), font_v_size, 0,
                                        &this->ft_face);
    if (this->ft_error != FT_Err_Ok) {
        LOG(LOG_ERR, FT_Error_String(this->ft_error));
        FT_Done_Face(this->ft_face);
        FT_Done_FreeType(this->ft_lib);
        close(this->fb_fd);
        throw std::runtime_error(FT_Error_String(this->ft_error));
    }

    this->render_timer.start();
    this->frame_timer.start();

    this->thread_pool.init();
}

DisplayScreen::~DisplayScreen() {
    LOG(LOG_WARN, "%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);
    this->thread_pool.shutdown();
    FT_Done_Face(this->ft_face);
    FT_Done_FreeType(this->ft_lib);
    memset(this->fbuffer, 0, this->fix.smem_len);
    munmap(this->fbuffer, this->fix.smem_len);
    this->display_fbuffer = nullptr;
    this->render_fbuffer = nullptr;
    close(this->fb_fd);
    DisplayScreen::_create = nullptr;
    this->xres = 0;
    this->yres = 0;
    this->rotation = 0;
}

std::shared_ptr <DisplayScreen> DisplayScreen::_create = nullptr;

std::shared_ptr <DisplayScreen> DisplayScreen::create(long _frame_rate, sync_mode_t _sync_mode) {
    if (_create == nullptr) {
        _create = std::shared_ptr<DisplayScreen>(new DisplayScreen(_frame_rate, _sync_mode));
    }

    return _create;
}

void DisplayScreen::refresh() {
    if (this->sync_mode) {
        this->render_timer.end();
        time_t sleep_time = this->frame_interval_time - this->render_timer.get_interval_time_t();
        std::this_thread::sleep_for(std::chrono::microseconds(sleep_time));
        this->render_timer.start();
    }
    this->frame_timer.end();
    this->frame_timer.start();
    this->var.xoffset = 0;
    this->var.yoffset = this->var.yoffset ? 0 : this->yres;
    int ret = ioctl(this->fb_fd, FBIOPAN_DISPLAY, &this->var);
    if (ret < 0) {
        throw std::runtime_error("ioctl FBIOPAN_DISPLAY failed");
    }
    std::swap(this->display_fbuffer, this->render_fbuffer);
}

void DisplayScreen::clear() {
    memset(this->render_fbuffer, 0, this->fix.line_length * this->yres);
}

void DisplayScreen::add_pixel(int32_t x, int32_t y, uint32_t color) {
    if ((x < 0) || (x < this->xres) || (y < 0) || (y < this->yres)) {
        *(this->render_fbuffer + y * this->var.xres_virtual + x) = color;
    }
}

void DisplayScreen::add_pixel(const Vector2D <int32_t> &vec, uint32_t color) {
    this->add_pixel(vec.x, vec.y, color);
}

void DisplayScreen::add_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                             uint32_t color, int32_t thickness) {
    if (color < 0 || thickness < 0) {
        LOG(LOG_ERR, "%s:%d:%s: invalid argument.\n", __FILE__, __LINE__, __FUNCTION__);
        throw std::runtime_error(Utils::str_format("%s:%d:%s: invalid argument.\n", __FILE__, __LINE__, __FUNCTION__));
    }
    int32_t dx = abs(x2 - x1);
    int32_t dy = abs(y2 - y1);
    int32_t yy = 0;

    if (dx < dy) {
        yy = 1;
        std::swap(x1, y1);
        std::swap(x2, y2);
        std::swap(dx, dy);
    }

    int32_t ix = (x2 - x1) > 0 ? 1 : -1;
    int32_t iy = (y2 - y1) > 0 ? 1 : -1;
    int32_t cx = x1;
    int32_t cy = y1;
    int32_t n2dy = dy * 2;
    int32_t n2dydx = (dy - dx) * 2;
    int32_t d = dy * 2 - dx;

    if (yy) { /* 如果直线与 x 轴的夹角大于 45 度 */
        while (cx != x2) {
            if (d < 0) {
                d += n2dy;
            } else {
                cy += iy;
                d += n2dydx;
            }
            if (thickness > 1) {
                this->add_filled_circle(thickness, cy, cx, color);
            } else {
                this->add_pixel(cy, cx, color);
            }
            cx += ix;
        }
    } else { /* 如果直线与 x 轴的夹角小于 45 度 */
        while (cx != x2) {
            if (d < 0) {
                d += n2dy;
            } else {
                cy += iy;
                d += n2dydx;
            }
            if (thickness > 1) {
                this->add_filled_circle(thickness, cy, cx, color);
            } else {
                this->add_pixel(cx, cy, color);
            }
            cx += ix;
        }
    }
}

void DisplayScreen::add_line(const Vector2D <int32_t> &vec1, const Vector2D <int32_t> &vec2,
                             uint32_t color, int32_t thickness) {
    this->add_line(vec1.x, vec1.y, vec2.x, vec2.y, color, thickness);
}

void DisplayScreen::add_rect(int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                             uint32_t color, int32_t rad, int32_t thickness) {
    if (color < 0 || thickness < 0) {
        LOG(LOG_ERR, "%s:%d:%s: invalid argument.\n", __FILE__, __LINE__, __FUNCTION__);
        throw std::runtime_error(Utils::str_format("%s:%d:%s: invalid argument.\n", __FILE__, __LINE__, __FUNCTION__));
    }
    if (rad <= 0) {
        this->add_line(x1, y1, x2, y1, color, thickness);
        this->add_line(x2, y1 + 1, x2, y2 - 1, color, thickness);
        this->add_line(x2, y2, x1, y2, color, thickness);
        this->add_line(x1, y2 - 1, x1, y1 + 1, color, thickness);
    } else {
        uint32_t dx = x2 - x1;
        uint32_t dy = y2 - y1;
        if (rad > std::min(dx, dy)) {
            rad = static_cast<int32_t>(std::min(dx, dy)) / 2;
        }
        this->add_line(x1 + rad, y1, x2 - rad, y1, color, thickness);
        this->add_line(x1 + rad, y2, x2 - rad, y2, color, thickness);
        this->add_line(x1, y1 + rad, x1, y2 - rad, color, thickness);
        this->add_line(x2, y1 + rad, x2, y2 - rad, color, thickness);

        this->add_circle(rad, x1 + rad, y1 + rad, color, 90, 180, thickness);
        this->add_circle(rad, x2 - rad, y1 + rad, color, 90, 180, thickness);
        this->add_circle(rad, x1 + rad, y2 - rad, color, 90, 180, thickness);
        this->add_circle(rad, x2 - rad, y2 - rad, color, 90, 180, thickness);
    }
}

void DisplayScreen::add_rect(const Vector2D <int32_t> &vec1, const Vector2D <int32_t> &vec2,
                             uint32_t color, int32_t rad, int32_t thickness) {
    this->add_rect(vec1.x, vec1.y, vec2.x, vec2.y, color, rad, thickness);
}

void DisplayScreen::add_filled_rect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color, int32_t rad) {
    if (color < 0 || rad < 0) {
        LOG(LOG_ERR, "%s:%d:%s: invalid argument.\n", __FILE__, __LINE__, __FUNCTION__);
        throw std::runtime_error(Utils::str_format("%s:%d:%s: invalid argument.\n", __FILE__, __LINE__, __FUNCTION__));
    }
    int32_t tmp;
    int32_t _x1 = x1;
    int32_t _y1 = y1;
    int32_t _x2 = x2;
    int32_t _y2 = y2;

    /* Clipping and sanity checking */
    if (x1 > x2) {
        tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y1 > y2) {
        tmp = y1;
        y1 = y2;
        y2 = tmp;
    }

    if (x1 < 0)
        x1 = 0;
    if (x1 >= this->xres)
        x1 = static_cast<int32_t>(this->xres) - 1;

    if (x2 < 0)
        x2 = 0;
    if (x2 >= this->xres)
        x2 = static_cast<int32_t>(this->xres) - 1;

    if (y1 < 0)
        y1 = 0;
    if (y1 >= this->yres)
        y1 = static_cast<int32_t>(this->yres) - 1;

    if (y2 < 0)
        y2 = 0;
    if (y2 >= this->yres)
        y2 = static_cast<int32_t>(this->yres) - 1;

    if ((x1 > x2) || (y1 > y2))
        return;

    for (; y1 <= y2; y1++) {
        for (tmp = x1; tmp <= x2; tmp++) {
            if (!(((tmp < _x1 + rad && y1 < _y1 + rad) &&
                   (Utils::get_distance<int32_t>(_x1 + rad, _y1 + rad, tmp, y1) > rad)) || /* 左上角 */
                  ((tmp > _x2 - rad && y1 < _y1 + rad) &&
                   (Utils::get_distance<int32_t>(_x2 - rad, _y1 + rad, tmp, y1) > rad)) || /* 右上角 */
                  ((tmp < _x1 + rad && y1 > _y2 - rad) &&
                   (Utils::get_distance<int32_t>(_x1 + rad, _y2 - rad, tmp, y1) > rad)) || /* 左下角 */
                  ((tmp > _x2 - rad && y1 > _y2 - rad) &&
                   (Utils::get_distance<int32_t>(_x2 - rad, _y2 - rad, tmp, y1) > rad)))) { /* 右下角 */
                this->add_pixel(tmp, y1, color);
            }
        }
    }
}

void DisplayScreen::add_filled_rect(const Vector2D <int32_t> &vec1, const Vector2D <int32_t> &vec2, uint32_t color,
                                    int32_t rad) {
    this->add_filled_rect(vec1.x, vec1.y, vec2.x, vec2.y, color, rad);
}

void DisplayScreen::add_circle(int32_t r, int32_t x, int32_t y,
                               uint32_t color, float start_angle, float end_angle, int32_t thickness) {
    if (r < 0 || color < 0 || thickness < 0) {
        LOG(LOG_ERR, "%s:%d:%s: invalid argument.\n", __FILE__, __LINE__, __FUNCTION__);
        throw std::runtime_error(Utils::str_format("%s:%d:%s: invalid argument.\n", __FILE__, __LINE__, __FUNCTION__));
    }

    int32_t _x = 0, _y = static_cast<int32_t>(r);
    int32_t d = 3 - 2 * static_cast<int32_t>(r);

    if (thickness <= 0) {
        thickness = 1;
    }
    if (thickness > r) {
        thickness = r;
    }

    while (_x <= _y) {
        for (int32_t yi = _x; yi < _y && yi >= _y - thickness; yi++)
            this->draw_circle_8(x, y, _x, yi, color, start_angle, end_angle);

        if (d < 0) {
            d = d + 4 * _x + 6;
        } else {
            d = d + 4 * (_x - _y) + 10;
            _y--;
        }
        _x++;
    }
}

void DisplayScreen::add_circle(int32_t r, const Vector2D <int32_t> &vec,
                               uint32_t color, float start_angle, float end_angle, int32_t thickness) {
    this->add_circle(r, vec.x, vec.y, color, start_angle, end_angle, thickness);
}

void DisplayScreen::add_filled_circle(int32_t r, int32_t x, int32_t y,
                                      uint32_t color, float start_angle, float end_angle) {
    if (r < 0 || color < 0) {
        LOG(LOG_ERR, "%s:%d:%s: invalid argument.\n", __FILE__, __LINE__, __FUNCTION__);
        throw std::runtime_error(Utils::str_format("%s:%d:%s: invalid argument.\n", __FILE__, __LINE__, __FUNCTION__));
    }

    int32_t _x = 0, _y = static_cast<int32_t>(r);
    int32_t d = 3 - 2 * static_cast<int32_t>(r);

    while (_x <= _y) {
        for (int32_t yi = _x; yi < _y; yi++)
            this->draw_circle_8(x, y, _x, yi, color, start_angle, end_angle);

        if (d < 0) {
            d = d + 4 * _x + 6;
        } else {
            d = d + 4 * (_x - _y) + 10;
            _y--;
        }
        _x++;
    }
}

void DisplayScreen::add_filled_circle(int32_t r, const Vector2D <int32_t> &vec,
                                      uint32_t color, float start_angle, float end_angle) {
    this->add_filled_circle(r, vec.x, vec.y, color, start_angle, end_angle);
}

double DisplayScreen::current_frame_time() const {
    return static_cast<double>(this->frame_timer.get_interval_time_t()) / 1000.0;
}

double DisplayScreen::current_frame_rate() const {
    return 1000000.0 / static_cast<double>(this->frame_timer.get_interval_time_t());
}

void DisplayScreen::set_default_ttf(const std::string &ttf) {
    this->ft_error = FT_New_Face(this->ft_lib, ttf.c_str(), 0, &this->ft_face);
    if (this->ft_error != FT_Err_Ok) {
        LOG(LOG_ERR, FT_Error_String(this->ft_error));
        throw std::runtime_error(FT_Error_String(this->ft_error));
    }
}

void DisplayScreen::restore_default_ttf() {
    this->ft_error = FT_New_Memory_Face(this->ft_lib, reinterpret_cast<const FT_Byte *>(font_v), font_v_size, 0,
                                        &this->ft_face);
    if (this->ft_error != FT_Err_Ok) {
        LOG(LOG_ERR, FT_Error_String(this->ft_error));
        throw std::runtime_error(FT_Error_String(this->ft_error));
    }
}

void DisplayScreen::compute_string_bbox(const std::wstring &str, FT_BBox &abbox) {
    FT_BBox bbox;
    FT_BBox glyph_bbox;
    FT_Vector pen;
    FT_Glyph glyph;
    FT_GlyphSlot slot = this->ft_face->glyph;

    /* 初始化 */
    bbox.xMin = bbox.yMin = 32000;
    bbox.xMax = bbox.yMax = -32000;

    /* 指定原点为(0, 0) */
    pen.x = 0;
    pen.y = 0;

    /* 计算每个字符的bounding box */
    /* 先translate, 再load char, 就可以得到它的外框了 */
    for (wchar_t i: str) {
        /* 转换：transformation */
        FT_Set_Transform(this->ft_face, nullptr, &pen);

        /* 加载位图: load glyph image into the slot (erase previous one) */
        this->ft_error = FT_Load_Char(this->ft_face, i, FT_LOAD_RENDER);
        if (this->ft_error != FT_Err_Ok) {
            LOG(LOG_ERR, FT_Error_String(this->ft_error));
            throw std::runtime_error(FT_Error_String(this->ft_error));
        }

        /* 取出glyph */
        this->ft_error = FT_Get_Glyph(this->ft_face->glyph, &glyph);
        if (this->ft_error != FT_Err_Ok) {
            LOG(LOG_ERR, FT_Error_String(this->ft_error));
            throw std::runtime_error(FT_Error_String(this->ft_error));
        }

        /* 从glyph得到外框: bbox */
        FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_TRUNCATE, &glyph_bbox);

        /* 更新外框 */
        if (glyph_bbox.xMin < bbox.xMin)
            bbox.xMin = glyph_bbox.xMin;

        if (glyph_bbox.yMin < bbox.yMin)
            bbox.yMin = glyph_bbox.yMin;

        if (glyph_bbox.xMax > bbox.xMax)
            bbox.xMax = glyph_bbox.xMax;

        if (glyph_bbox.yMax > bbox.yMax)
            bbox.yMax = glyph_bbox.yMax;

        /* 计算下一个字符的原点: increment pen position */
        pen.x += slot->advance.x;
        pen.y += slot->advance.y;

        this->thread_pool.submit([glyph] {
            FT_Done_Glyph(glyph);
        });
    }

    /* return string bbox */
    abbox = bbox;
}

void DisplayScreen::draw_bitmap(FT_Bitmap &bitmap, int32_t x, int32_t y, uint32_t color) {
    if (color < 0) {
        LOG(LOG_ERR, "%s:%d:%s: invalid argument.\n", __FILE__, __LINE__, __FUNCTION__);
        throw std::runtime_error(Utils::str_format("%s:%d:%s: invalid argument.\n", __FILE__, __LINE__, __FUNCTION__));
    }

    uint32_t x_max = x + bitmap.width;
    uint32_t y_max = y + bitmap.rows;

    for (int32_t j = y, q = 0; j < y_max; j++, q++) {
        for (int32_t i = x, p = 0; i < x_max; i++, p++) {
            if (i < 0 || j < 0 || i >= this->var.xres || j >= this->var.yres) {
                continue;
            }
            uint8_t gray = bitmap.buffer[q * bitmap.width + p];
            if (gray != 0) {
                uint32_t _r = (((color >> 16) & 0xff) * gray) >> 8;
                uint32_t _g = (((color >> 8) & 0xff) * gray) >> 8;
                uint32_t _b = (((color >> 0) & 0xff) * gray) >> 8;
                this->add_pixel(i, j, _r << 16 | _g << 8 | _b);
            }
        }
    }
}

void DisplayScreen::draw_circle_8(int32_t xc, int32_t yc, int32_t x, int32_t y,
                                  uint32_t color, float start_angle, float end_angle) {
    if (color < 0) {
        LOG(LOG_ERR, "%s:%d:%s: invalid argument.\n", __FILE__, __LINE__, __FUNCTION__);
        throw std::runtime_error(Utils::str_format("%s:%d:%s: invalid argument.\n", __FILE__, __LINE__, __FUNCTION__));
    }

    float tmp_angle;
    float _start_angle = Utils::to_simple_angle(start_angle);
    float _end_angle = Utils::to_simple_angle(end_angle);
    if (_start_angle == _end_angle) {
        if (_start_angle == 0 && _end_angle == 0) {
            this->add_pixel(xc + x, yc + y, color);
            this->add_pixel(xc - x, yc + y, color);
            this->add_pixel(xc + x, yc - y, color);
            this->add_pixel(xc - x, yc - y, color);
            this->add_pixel(xc + y, yc + x, color);
            this->add_pixel(xc - y, yc + x, color);
            this->add_pixel(xc + y, yc - x, color);
            this->add_pixel(xc - y, yc - x, color);
        }
        return;
    }
    if (_start_angle > _end_angle) {
        std::swap(_start_angle, _end_angle);
    }
    tmp_angle = Utils::get_angle(xc, yc, xc + x, yc + y);
    if (tmp_angle >= _start_angle && tmp_angle <= _end_angle) {
        this->add_pixel(xc + x, yc + y, color);
    }
    tmp_angle = Utils::get_angle(xc, yc, xc - x, yc + y);
    if (tmp_angle >= _start_angle && tmp_angle <= _end_angle) {
        this->add_pixel(xc - x, yc + y, color);
    }
    tmp_angle = Utils::get_angle(xc, yc, xc + x, yc - y);
    if (tmp_angle >= _start_angle && tmp_angle <= _end_angle) {
        this->add_pixel(xc + x, yc - y, color);
    }
    tmp_angle = Utils::get_angle(xc, yc, xc - x, yc - y);
    if (tmp_angle >= _start_angle && tmp_angle <= _end_angle) {
        this->add_pixel(xc - x, yc - y, color);
    }
    tmp_angle = Utils::get_angle(xc, yc, xc + y, yc + x);
    if (tmp_angle >= _start_angle && tmp_angle <= _end_angle) {
        this->add_pixel(xc + y, yc + x, color);
    }
    tmp_angle = Utils::get_angle(xc, yc, xc - y, yc + x);
    if (tmp_angle >= _start_angle && tmp_angle <= _end_angle) {
        this->add_pixel(xc - y, yc + x, color);
    }
    tmp_angle = Utils::get_angle(xc, yc, xc + y, yc - x);
    if (tmp_angle >= _start_angle && tmp_angle <= _end_angle) {
        this->add_pixel(xc + y, yc - x, color);
    }
    tmp_angle = Utils::get_angle(xc, yc, xc - y, yc - x);
    if (tmp_angle >= _start_angle && tmp_angle <= _end_angle) {
        this->add_pixel(xc - y, yc - x, color);
    }
}

void DisplayScreen::_text(const std::string &str, int32_t x, int32_t y,
                          uint32_t color, uint32_t degrees) {
    if (color < 0) {
        LOG(LOG_ERR, "%s:%d:%s: invalid argument.\n", __FILE__, __LINE__, __FUNCTION__);
        throw std::runtime_error(Utils::str_format("%s:%d:%s: invalid argument.\n", __FILE__, __LINE__, __FUNCTION__));
    }

    FT_BBox bbox;
    FT_Vector pen;
    FT_Matrix matrix;
    FT_GlyphSlot slot = this->ft_face->glyph;

    /* 把LCD坐标转换为笛卡尔坐标 */
    int32_t _x = x;
    int32_t _y = static_cast<int32_t>(this->var.yres) - y;

    /* std::string 转 std::wstring */
    std::wstring w_str = Utils::to_wide_string(str);

    /* 计算外框 */
    this->compute_string_bbox(w_str, bbox);

    /* 反推原点 */
    pen.x = (_x - bbox.xMin) * 64; /* 单位: 1/64像素 */
    pen.y = (_y - bbox.yMax) * 64; /* 单位: 1/64像素 */

    if (degrees > 0) {
        double angle = (static_cast<double>(degrees) / 180.0) * M_PI;

        matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
        matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
        matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
        matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);
    }

    /* 处理每个字符 */
    for (wchar_t i: w_str) {
        /* 转换：transformation */
        FT_Set_Transform(this->ft_face, (degrees > 0) ? &matrix : nullptr, &pen);

        /* 加载位图: load glyph image into the slot (erase previous one) */
        this->ft_error = FT_Load_Char(this->ft_face, i, FT_LOAD_RENDER);
        if (this->ft_error != FT_Err_Ok) {
            LOG(LOG_ERR, FT_Error_String(this->ft_error));
            break;
        }

        /* 在LCD上绘制: 使用LCD坐标 */
        draw_bitmap(slot->bitmap, slot->bitmap_left, static_cast<int32_t>(var.yres) - slot->bitmap_top, color);

        /* 计算下一个字符的原点: increment pen position */
        pen.x += slot->advance.x;
        pen.y += slot->advance.y;
    }
}

void DisplayScreen::add_text(const std::string &str, int32_t x, int32_t y,
                             int32_t size, uint32_t color, uint32_t degrees) {
    FT_Set_Pixel_Sizes(this->ft_face, size, size);
    this->_text(str, x, y, color, degrees);
}

void DisplayScreen::add_text(const std::string &str, const Vector2D <int32_t> &vec,
                             int32_t size, uint32_t color, uint32_t degrees) {
    this->add_text(str, vec.x, vec.y, size, color, degrees);
}

void DisplayScreen::add_text(const std::string &ttf, const std::string &str, int32_t x, int32_t y,
                             int32_t size, uint32_t color, uint32_t degrees) {
    this->set_default_ttf(ttf);
    FT_Set_Pixel_Sizes(this->ft_face, size, size);
    this->_text(str, x, y, color, degrees);
    this->restore_default_ttf();
}

void DisplayScreen::add_text(const std::string &ttf, const std::string &str, const Vector2D <int32_t> &vec,
                             int32_t size, uint32_t color, uint32_t degrees) {
    this->add_text(ttf, str, vec.x, vec.y, size, color, degrees);
}

void DisplayScreen::add_text(const TextView &textView, uint32_t degrees) {
    if (textView.text.length() > 0) {
        bool is_set_ttf = (textView.ttf.length() > 0);
        if (is_set_ttf) {
            this->set_default_ttf(textView.ttf);
        }
        FT_Set_Pixel_Sizes(this->ft_face, textView.size, textView.size);
        this->_text(textView.text, textView.x, textView.y, textView.color, degrees);
        if (is_set_ttf) {
            this->restore_default_ttf();
        }
    }
}

void DisplayScreen::add_button(Button &button) {
    this->add_filled_rect(button.x1, button.y1, button.x2, button.y2, button.color, button.rad);
    this->add_text(button.textView);
}

Vector2D <uint32_t> DisplayScreen::get_screen_size() const {
    return Vector2D < uint32_t > {
            this->xres,
            this->yres
    };
}

void DisplayScreen::add_texture(Texture &texture) {
    constexpr
    int32_t num = 8;
    int32_t w = texture.get_width();
    int32_t h = texture.get_height();
    uint32_t *color_list = texture.get_texture().get();
    for (int32_t i = 0; i < h; i++) {
        for (int32_t j = 0; j < w; j++) {
            this->add_pixel(j, i, color_list[i * w + j]);
        }
    }
}

void DisplayScreen::destroy() {
    delete _create.get();
}
