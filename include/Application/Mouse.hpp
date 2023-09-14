#pragma once

struct SDL_Cursor;

namespace VGG
{

class Mouse
{
public:
  enum class ECursor
  {
    ARROW,     /**< Arrow */
    IBEAM,     /**< I-beam */
    WAIT,      /**< Wait */
    CROSSHAIR, /**< Crosshair */
    WAITARROW, /**< Small wait cursor (or Wait if not available) */
    SIZENWSE,  /**< Double arrow pointing northwest and southeast */
    SIZENESW,  /**< Double arrow pointing northeast and southwest */
    SIZEWE,    /**< Double arrow pointing west and east */
    SIZENS,    /**< Double arrow pointing north and south */
    SIZEALL,   /**< Four pointed arrow pointing north, south, east, and west */
    NO,        /**< Slashed circle or crossbones */
    HAND       /**< Hand */
  };

public:
  virtual ~Mouse() = default;

  virtual void setCursor(ECursor type) = 0;
  virtual void resetCursor() = 0;
};

} // namespace VGG