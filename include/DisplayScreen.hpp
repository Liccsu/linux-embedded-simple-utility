/*
 * Created by Administrator on 2023/8/23.
 */

#ifndef GREEDYSNAKE_DISPLAYSCREEN_HPP
#define GREEDYSNAKE_DISPLAYSCREEN_HPP

#define _USE_MATH_DEFINES


#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/vt.h>
#include <linux/kd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <ft2build.h>
#include <cmath>
#include <memory>
#include "Timer.hpp"
#include "Font.h"
#include "TypeDef.hpp"
#include "Button.hpp"
#include "Utils.hpp"
#include "LOG.hpp"
#include "Texture.hpp"
#include "ThreadPool.hpp"
#include FT_FREETYPE_H
#include FT_GLYPH_H


#define DEFAULT_FB_DEVICE "/dev/fb0"
constexpr long DEFAULT_FRAME_RATE = 30;

typedef enum {
    SYNC_MODE_ON = true,
    SYNC_MODE_OFF = false
} sync_mode_t;


class DisplayScreen {
private:
    static std::shared_ptr <DisplayScreen> _create; /* 屏幕对象指针 */
    long frame_rate; /* 设置渲染速率 */
    sync_mode_t sync_mode; /* 设置垂直同步 */
    int8_t rotation; /* 设置屏幕方向 */
    int32_t fb_fd; /* 帧缓冲文件描述符 */
    uint32_t xres; /* 物理屏幕宽度 */
    uint32_t yres; /* 物理屏幕高度 */
    uint32_t bytes_per_pixel; /* 单个像素点所占字节数 */
    uint32_t *fbuffer;  /* 帧缓冲的首地址 */
    uint32_t *display_fbuffer;  /* 当前显示帧缓冲首地址 */
    uint32_t *render_fbuffer; /* 当前渲染帧缓冲首地址 */
    time_t frame_interval_time; /* 每渲染两帧之间的时间间隔 微秒 */
    struct fb_fix_screeninfo fix; /* 屏幕固定参数 */
    struct fb_var_screeninfo var; /* 屏幕可变参数 */
    std::string fb_device; /* 帧缓冲设备文件路径 */
    Timer render_timer; /* 渲染帧率计时器 */
    Timer frame_timer; /* 显示帧率计时器 */
    ThreadPool thread_pool; /* 线程池 */

    FT_Error ft_error; /* freetype错误码 */
    FT_Library ft_lib; /* freetype库 */
    FT_Face ft_face;    /* 字体文件 */

    void _text(const std::string &str, int32_t x, int32_t y,
               uint32_t color = 0x000000, uint32_t degrees = 0);

    void compute_string_bbox(const std::wstring &str, FT_BBox &abbox);

    void draw_bitmap(FT_Bitmap &bitmap, int32_t x, int32_t y, uint32_t color);

    void draw_circle_8(int32_t xc, int32_t yc, int32_t x, int32_t y,
                       uint32_t color, float start_angle = 0, float end_angle = 0);

protected:
    DisplayScreen(long _frame_rate, sync_mode_t _sync_mode);

    DisplayScreen(const DisplayScreen &) = delete;

public:
    ~DisplayScreen();

    static std::shared_ptr <DisplayScreen>
    create(long _frame_rate = DEFAULT_FRAME_RATE, sync_mode_t _sync_mode = SYNC_MODE_ON);

    void refresh();

    void clear();

    void add_pixel(int32_t x, int32_t y, uint32_t color);

    void add_pixel(const Vector2D<int32_t> &vec, uint32_t color);

    void add_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color, int32_t thickness = 1);

    void add_line(const Vector2D<int32_t> &vec1, const Vector2D<int32_t> &vec2, uint32_t color, int32_t thickness = 1);

    void add_rect(int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                  uint32_t color, int32_t rad = 0, int32_t thickness = 1);

    void add_rect(const Vector2D<int32_t> &vec1, const Vector2D<int32_t> &vec2,
                  uint32_t color, int32_t rad = 0, int32_t thickness = 1);

    void add_filled_rect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color, int32_t rad = 0);

    void add_filled_rect(const Vector2D<int32_t> &vec1, const Vector2D<int32_t> &vec2, uint32_t color, int32_t rad = 0);

    void add_circle(int32_t r, int32_t x, int32_t y,
                    uint32_t color, float start_angle = 0, float end_angle = 0, int32_t thickness = 1);

    void add_circle(int32_t r, const Vector2D<int32_t> &vec,
                    uint32_t color, float start_angle = 0, float end_angle = 0, int32_t thickness = 1);

    void add_filled_circle(int32_t r, int32_t x, int32_t y,
                           uint32_t color, float start_angle = 0, float end_angle = 0);

    void add_filled_circle(int32_t r, const Vector2D<int32_t> &vec,
                           uint32_t color, float start_angle = 0, float end_angle = 0);

    void set_default_ttf(const std::string &ttf);

    void restore_default_ttf();

    void add_text(const std::string &str, int32_t x, int32_t y,
                  int32_t size, uint32_t color = 0, uint32_t degrees = 0);

    void add_text(const std::string &str, const Vector2D<int32_t> &vec,
                  int32_t size, uint32_t color = 0, uint32_t degrees = 0);

    void add_text(const std::string &ttf, const std::string &str, int32_t x, int32_t y,
                  int32_t size, uint32_t color = 0, uint32_t degrees = 0);

    void add_text(const std::string &ttf, const std::string &str, const Vector2D<int32_t> &vec,
                  int32_t size, uint32_t color = 0, uint32_t degrees = 0);

    void add_text(const TextView &textView, uint32_t degrees = 0);

    void add_button(Button &button);

    double current_frame_time() const;

    double current_frame_rate() const;

    Vector2D<uint32_t> get_screen_size() const;

    void add_texture(Texture &texture);

    static void destroy();
};

#endif //GREEDYSNAKE_DISPLAYSCREEN_HPP
