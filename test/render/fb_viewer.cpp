#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string>
#include <../include/Utility/Log.hpp>

class LinuxFrameBuffer
{
  const std::string        m_device;
  struct fb_var_screeninfo m_vinfo;
  struct fb_fix_screeninfo m_finfo;
  int                      m_fbfd{ -1 };
  size_t                   m_screensize{ 0 };
  char*                    m_fbp{ nullptr };

protected:
  void init()
  {
    ASSERT(!m_device.empty());

    m_fbfd = open(m_device.c_str(), O_RDWR);
    if (m_fbfd < 0)
    {
      FAIL("Failed to open framebuffer device");
      return;
    }
    if (ioctl(m_fbfd, FBIOGET_FSCREENINFO, &m_finfo) < 0)
    {
      FAIL("Failed to read framebuffer's fixed information");
      return;
    }
    if (ioctl(m_fbfd, FBIOGET_VSCREENINFO, &m_vinfo) < 0)
    {
      FAIL("Failed to read framebuffer's variable information");
      return;
    }
    INFO(
      "Framebuffer (%s): %dx%d, %dbpp",
      m_device.c_str(),
      m_vinfo.xres,
      m_vinfo.yres,
      m_vinfo.bits_per_pixel);

    m_screensize = m_finfo.smem_len; // m_vinfo.yres * m_finfo.line_length;
    m_fbp = (char*)mmap(0, m_screensize, PROT_READ | PROT_WRITE, MAP_SHARED, m_fbfd, 0);
    if (reinterpret_cast<intptr_t>(m_fbp) < 0)
    {
      FAIL("Failed to map framebuffer device to memory");
      return;
    }
  }

  void deinit()
  {
    if (m_fbp)
    {
      munmap(m_fbp, m_screensize);
    }
    if (m_fbfd > 0)
    {
      close(m_fbfd);
    }
  }

public:
  LinuxFrameBuffer(const char* device = "/dev/fb0")
    : m_device(device)
  {
    init();
  }
  virtual ~LinuxFrameBuffer()
  {
    deinit();
  }

  int getBPP()
  {
    if (m_fbfd > 0)
    {
      return m_vinfo.bits_per_pixel;
    }
    return -1;
  }

  char* getBuffer()
  {
    if (m_fbfd > 0)
    {
      return m_fbp;
    }
    return nullptr;
  }

  size_t getBufferSize()
  {
    if (m_fbfd > 0)
    {
      return m_screensize;
    }
    return 0;
  }

  size_t getBufferWidth()
  {
    if (m_fbfd > 0)
    {
      return m_vinfo.yres;
    }
    return 0;
  }

  size_t getBufferHeight()
  {
    if (m_fbfd > 0)
    {
      return m_finfo.line_length;
    }
    return 0;
  }
};

int main()
{
  LinuxFrameBuffer fb;
  int              bpp = fb.getBPP();
  if (bpp != 32)
  {
    FAIL("Only 32 bits per pixel is supported for now");
    return -1;
  }

  char*  buf = fb.getBuffer();
  size_t size = fb.getBufferSize();
  for (size_t i = 0; i < size; i++)
  {
    switch (i % 4)
    {
      case 0: // Blue Channel
        buf[i] = 0;
        break;
      case 1: // Green Channel
        buf[i] = 0;
        break;
      case 2: // Red Channel
        buf[i] = 0xff;
        break;
      case 3: // Alpha Channel
        buf[i] = 0;
        break;
      default:
        break;
    }
  }
  return 0;
}
