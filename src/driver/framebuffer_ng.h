/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Copyright (C) 2007-2013 Stefan Seyfried

	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __framebuffer__
#define __framebuffer__
#include <config.h>

#include <stdint.h>
#include <linux/fb.h>
#include <linux/vt.h>

#include <string>
#include <map>
#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>

#if HAVE_DUCKBOX_HARDWARE || HAVE_SPARK_HARDWARE
#include <png.h>
#endif

#define fb_pixel_t uint32_t

typedef struct fb_var_screeninfo t_fb_var_screeninfo;

#define CORNER_TOP_LEFT		0x1
#define CORNER_TOP_RIGHT	0x2
#define CORNER_TOP		0x3
#define CORNER_BOTTOM_RIGHT	0x4
#define CORNER_RIGHT		0x6
#define CORNER_BOTTOM_LEFT	0x8
#define CORNER_LEFT		0x9
#define CORNER_BOTTOM		0xC
#define CORNER_ALL		0xF

#define FADE_TIME 10000
#define FADE_STEP 5
#define FADE_RESET 0xFFFF

#define NON_BIG_WINDOWS		85 // %
#define ConnectLineBox_Width	16 // px

#if HAVE_GENERIC_HARDWARE
#define USE_OPENGL 1
#endif

#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
#define DEFAULT_XRES 1280
#define DEFAULT_YRES 720
#endif

class CFrameBuffer;
class CFbAccel
{
	private:
		CFrameBuffer *fb;
		fb_pixel_t lastcol;
		OpenThreads::Mutex mutex;
#ifdef USE_NEVIS_GXA
		int		  devmem_fd;	/* to access the GXA register we use /dev/mem */
		unsigned int	  smem_start;	/* as aquired from the fbdev, the framebuffers physical start address */
		volatile uint8_t *gxa_base;	/* base address for the GXA's register access */
		void add_gxa_sync_marker(void);
#endif /* USE_NEVIS_GXA */
		void setColor(fb_pixel_t col);
	public:
		fb_pixel_t *backbuffer;
		fb_pixel_t *lbb;
		CFbAccel(CFrameBuffer *fb);
		~CFbAccel();
		void paintPixel(int x, int y, const fb_pixel_t col);
		void paintRect(const int x, const int y, const int dx, const int dy, const fb_pixel_t col);
		void paintLine(int xa, int ya, int xb, int yb, const fb_pixel_t col);
		void blitRGB2RGB(int original_width, int original_height, int height, int width, char *original_data, char *dest_data);
		void blit2FB(void *fbbuff, uint32_t width, uint32_t height, uint32_t xoff, uint32_t yoff, uint32_t xp, uint32_t yp, bool transp);
		void waitForIdle(void);
		void mark(int x, int y, int dx, int dy);
		void blit();
		void update();
#ifdef USE_NEVIS_GXA
		void setupGXA(void);
#endif
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
		bool OSDShot(const std::string &name);
		int sX, sY, eX, eY;
		int startX, startY, endX, endY;
		t_fb_var_screeninfo s;
		fb_pixel_t borderColor, borderColorOld;
		void blitIcon(int src_width, int src_height, int fb_x, int fb_y, int width, int height);
		void resChange(void);
		void blitBB2FB(int fx0, int fy0, int fx1, int fy1, int tx0, int ty0, int tx1, int ty2);
		void blitFB2FB(int fx0, int fy0, int fx1, int fy1, int tx0, int ty0, int tx1, int ty2);
		void blitBoxFB(int x0, int y0, int x1, int y1, fb_pixel_t color);
#endif
};

/** Ausfuehrung als Singleton */
class CFrameBuffer
{
	friend class CFbAccel;
	private:
		CFrameBuffer();

		struct rgbData
		{
			uint8_t r;
			uint8_t g;
			uint8_t b;
		} __attribute__ ((packed));

		struct rawHeader
		{
			uint8_t width_lo;
			uint8_t width_hi;
			uint8_t height_lo;
			uint8_t height_hi;
			uint8_t transp;
		} __attribute__ ((packed));

		struct rawIcon
		{
			uint16_t width;
			uint16_t height;
			uint8_t transp;
			fb_pixel_t * data;
		};

		std::string     iconBasePath;

		int             fd, tty;
		fb_pixel_t *    lfb;
		int		available;
		fb_pixel_t *    background;
		fb_pixel_t *    backupBackground;
		fb_pixel_t      backgroundColor;
		std::string     backgroundFilename;
		bool            useBackgroundPaint;
		unsigned int	xRes, yRes, stride, bpp;
		t_fb_var_screeninfo screeninfo, oldscreen;
		fb_cmap cmap;
		__u16 red[256], green[256], blue[256], trans[256];

		void paletteFade(int i, __u32 rgb1, __u32 rgb2, int level);

		int 	kd_mode;
		struct	vt_mode vt_mode;
		bool	active;
		static	void switch_signal (int);
		fb_fix_screeninfo fix;
		bool locked;
		std::map<std::string, rawIcon> icon_cache;
		int cache_size;
		void * int_convertRGB2FB(unsigned char *rgbbuff, unsigned long x, unsigned long y, int transp, bool alpha);
		int m_transparent_default, m_transparent;
		CFbAccel *accel;

	public:
		fb_pixel_t realcolor[256];

		~CFrameBuffer();

		static CFrameBuffer* getInstance();
#ifdef USE_NEVIS_GXA
		void setupGXA(void);
#endif

#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
		void init(const char * const fbDevice = "/dev/fb0");
#else
		void init(const char * const fbDevice = "/dev/fb/0");
#endif
		int setMode(unsigned int xRes, unsigned int yRes, unsigned int bpp);


		int getFileHandle() const; //only used for plugins (games) !!
		t_fb_var_screeninfo *getScreenInfo();

#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
		fb_pixel_t * getFrameBufferPointer(bool real = false); // pointer to framebuffer
#else
		fb_pixel_t * getFrameBufferPointer() const; // pointer to framebuffer
#endif
		fb_pixel_t * getBackBufferPointer() const;  // pointer to backbuffer
		unsigned int getStride() const;             // size of a single line in the framebuffer (in bytes)
		unsigned int getScreenWidth(bool real = false);
		unsigned int getScreenHeight(bool real = false); 
		unsigned int getScreenWidthRel();
		unsigned int getScreenHeightRel();
		unsigned int getScreenX();
		unsigned int getScreenY();
		
		bool getActive() const;                     // is framebuffer active?
		void setActive(bool enable);                     // is framebuffer active?

		void setTransparency( int tr = 0 );
		void setBlendMode(uint8_t mode = 1);
		void setBlendLevel(int level);

		//Palette stuff
		void setAlphaFade(int in, int num, int tr);
		void paletteGenFade(int in, __u32 rgb1, __u32 rgb2, int num, int tr=0);
		void paletteSetColor(int i, __u32 rgb, int tr);
		void paletteSet(struct fb_cmap *map = NULL);

		//paint functions
		inline void paintPixel(fb_pixel_t * const dest, const uint8_t color) const
			{
				*dest = realcolor[color];
			};
		void paintPixel(int x, int y, const fb_pixel_t col);

		void paintBoxRel(const int x, const int y, const int dx, const int dy, const fb_pixel_t col, int radius = 0, int type = CORNER_ALL);
		inline void paintBox(int xa, int ya, int xb, int yb, const fb_pixel_t col) { paintBoxRel(xa, ya, xb - xa, yb - ya, col); }
		inline void paintBox(int xa, int ya, int xb, int yb, const fb_pixel_t col, int radius, int type) { paintBoxRel(xa, ya, xb - xa, yb - ya, col, radius, type); }

		void paintBoxFrame(const int x, const int y, const int dx, const int dy, const int px, const fb_pixel_t col, const int rad = 0);
		void paintLine(int xa, int ya, int xb, int yb, const fb_pixel_t col);

		void paintVLine(int x, int ya, int yb, const fb_pixel_t col);
		void paintVLineRel(int x, int y, int dy, const fb_pixel_t col);

		void paintHLine(int xa, int xb, int y, const fb_pixel_t col);
		void paintHLineRel(int x, int dx, int y, const fb_pixel_t col);


		void setIconBasePath(const std::string & iconPath);

		void getIconSize(const char * const filename, int* width, int *height);
		/* h is the height of the target "window", if != 0 the icon gets centered in that window */
		bool paintIcon (const std::string & filename, const int x, const int y, 
				const int h = 0, const unsigned char offset = 1, bool paint = true, bool paintBg = false, const fb_pixel_t colBg = 0);
		bool paintIcon8(const std::string & filename, const int x, const int y, const unsigned char offset = 0);
		void loadPal   (const std::string & filename, const unsigned char offset = 0, const unsigned char endidx = 255);

		bool loadPicture2Mem        (const std::string & filename, fb_pixel_t * const memp);
		bool loadPicture2FrameBuffer(const std::string & filename);
		bool loadPictureToMem       (const std::string & filename, const uint16_t width, const uint16_t height, const uint16_t stride, fb_pixel_t * const memp);
		bool savePictureFromMem     (const std::string & filename, const fb_pixel_t * const memp);

		int getBackgroundColor() { return backgroundColor;}
		void setBackgroundColor(const fb_pixel_t color);
		bool loadBackground(const std::string & filename, const unsigned char col = 0);
		void useBackground(bool);
		bool getuseBackground(void);

		void saveBackgroundImage(void);  // <- implies useBackground(false);
		void restoreBackgroundImage(void);

		void paintBackgroundBoxRel(int x, int y, int dx, int dy);
		inline void paintBackgroundBox(int xa, int ya, int xb, int yb) { paintBackgroundBoxRel(xa, ya, xb - xa, yb - ya); }

		void paintBackground();

		void SaveScreen(int x, int y, int dx, int dy, fb_pixel_t * const memp);
		void RestoreScreen(int x, int y, int dx, int dy, fb_pixel_t * const memp);

		void Clear();
		void showFrame(const std::string & filename);
		bool loadBackgroundPic(const std::string & filename, bool show = true);
		bool Lock(void);
		void Unlock(void);
		bool Locked(void) { return locked; };
		void waitForIdle(void);
		void* convertRGB2FB(unsigned char *rgbbuff, unsigned long x, unsigned long y, int transp = 0xFF);
		void* convertRGBA2FB(unsigned char *rgbbuff, unsigned long x, unsigned long y);
		void displayRGB(unsigned char *rgbbuff, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs, bool clearfb = true, int transp = 0xFF);
		void blitRGB2RGB(int original_width, int original_height, int height, int width, char *original_data, char *dest_data);
		void blit2FB(void *fbbuff, uint32_t width, uint32_t height, uint32_t xoff, uint32_t yoff, uint32_t xp = 0, uint32_t yp = 0, bool transp = false);
		bool blitToPrimary(unsigned int * data, int dx, int dy, int sw, int sh);

		void mark(int x, int y, int dx, int dy) { accel->mark(x, y, dx, dy); };
		void blit() { accel->blit(); };
		void paintMuteIcon(bool paint, int ax, int ay, int dx, int dy, bool paintFrame=true);

		enum 
			{
				TM_EMPTY  = 0,
				TM_NONE   = 1,
				TM_BLACK  = 2,
				TM_INI    = 3
			};
		void SetTransparent(int t){ m_transparent = t; }
		void SetTransparentDefault(){ m_transparent = m_transparent_default; }
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
		bool OSDShot(const std::string &name) { accel->OSDShot(name); };
		enum Mode3D { Mode3D_off = 0, Mode3D_SideBySide, Mode3D_TopAndBottom, Mode3D_Tile, Mode3D_SIZE };
		void set3DMode(Mode3D);
		Mode3D get3DMode(void);
	private:
		enum Mode3D mode3D;

	public:
		void blitIcon(int src_width, int src_height, int fb_x, int fb_y, int width, int height);
		fb_pixel_t getBorderColor(void) { return accel->borderColor; };
		void setBorderColor(fb_pixel_t col = 0);
		void setBorder(int sx, int sy, int ex, int ey);
		void getBorder(int &sx, int &sy, int &ex, int &ey) { sx = accel->startX, sy = accel->startY, ex = accel->endX, ey = accel->endY;};
		void resChange(void) { this->accel->resChange(); };
		void ClearFB(void) { accel->blitBoxFB(0, 0, accel->s.xres - 1, accel->s.yres - 1, 0); };
#endif
};

#endif
