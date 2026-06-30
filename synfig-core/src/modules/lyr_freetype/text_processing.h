#ifndef SYNFIG_TEXT_PROCESSING_H
#define SYNFIG_TEXT_PROCESSING_H
  
#include <synfig/rendering/common/task/taskcontour.h>  
#include <synfig/vector.h>  
  
#include <ft2build.h>  
#include FT_GLYPH_H  
  
#if HAVE_HARFBUZZ  
#include <hb.h>  
#endif  
  
#include <cstdint>  
#include <string>  
#include <vector>  
  
namespace synfig {  



/* === M A C R O S ========================================================= */

// Copy of PangoStyle
// It is necessary to keep original values if Pango ever change them
//  - because it would change layer rendering as Synfig stores the parameter
//     value as an integer (ie. not a weight name string)
enum TextStyle{
	TEXT_STYLE_NORMAL = 0,
	TEXT_STYLE_OBLIQUE = 1,
	TEXT_STYLE_ITALIC = 2
};

// Copy of PangoWeight
// It is necessary to keep original values if Pango ever change them
//  - because it would change layer rendering as Synfig stores the parameter
//     value as an integer (ie. not a weight name string)
enum TextWeight{
	TEXT_WEIGHT_THIN = 100,
	TEXT_WEIGHT_ULTRALIGHT = 200,
	TEXT_WEIGHT_LIGHT = 300,
	TEXT_WEIGHT_SEMILIGHT = 350,
	TEXT_WEIGHT_BOOK = 380,
	TEXT_WEIGHT_NORMAL = 400,
	TEXT_WEIGHT_MEDIUM = 500,
	TEXT_WEIGHT_SEMIBOLD = 600,
	TEXT_WEIGHT_BOLD = 700,
	TEXT_WEIGHT_ULTRABOLD = 800,
	TEXT_WEIGHT_HEAVY = 900,
	TEXT_WEIGHT_ULTRAHEAVY = 1000
};

enum TextDirection{
	TEXT_DIRECTION_AUTO = 0,
	TEXT_DIRECTION_LTR = 1,
	TEXT_DIRECTION_RTL = 2,
};


/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
struct TextSpan  
{  
	std::vector<uint32_t> codepoints;  
#if HAVE_HARFBUZZ  
	hb_script_t script; 
	hb_direction_t direction;
#endif  
};  
  
using TextLine = std::vector<TextSpan>;  

struct ShapedGlyph  
{  
	uint32_t glyph_index;  
	uint32_t cluster;
	int32_t  x_advance = 0, y_advance = 0;  
    int32_t  x_offset  = 0, y_offset  = 0;  
	  
};  
  

namespace text_processing { 
using ShapedLine = std::vector<ShapedGlyph>;  

std::vector<TextLine> fetch_text_lines(const std::string& text, int direction);  

#if HAVE_HARFBUZZ  
std::vector<std::vector<ShapedGlyph>>  
shape_text(const std::vector<TextLine>& lines, hb_font_t* font);  
#else  
std::vector<std::vector<ShapedGlyph>>  
shape_text(const std::vector<TextLine>& lines);  
#endif  

void convert_outline_to_contours(  
	const FT_OutlineGlyphRec* glyph,  
	rendering::Contour::ChunkList& chunks);  
  
void shift_contour_chunks(  
	rendering::Contour::ChunkList& chunks,  
	const Vector& offset);  
  
} // namespace text_processing  
} // namespace synfig  
  
#endif
