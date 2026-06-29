#include "text_processing.h"  
  
#include <algorithm>  
  
#if HAVE_HARFBUZZ  
#include <fribidi.h>  
#include <hb-ft.h>
#endif  
  
#include <synfig/general.h>  
#include <synfig/localization.h>  
#include <synfig/string_helper.h>
#include FT_OUTLINE_H  
namespace synfig {

namespace text_processing {  
                  
  
static const std::vector<uint32_t> line_endings{  
	'\n', '\v', '\f', '\r', 0x0085, 0x2028, 0x2029  
};  
  
static std::vector<uint32_t>  
utf8_to_utf32(const std::string& text)  
{  
	std::vector<uint32_t> unicode;  
  
#if HAVE_HARFBUZZ  
	unicode.resize(text.size() + 1);  
	FriBidiStrIndex unicode_len = fribidi_charset_to_unicode(  
		FRIBIDI_CHAR_SET_UTF8,  
		text.c_str(),  
		text.size(),  
		unicode.data());  
	unicode.resize(unicode_len);  
#else  
	for (auto iter = text.cbegin(); iter != text.cend(); ++iter) {  
		unsigned int c = (unsigned char)*iter;  
		unsigned int code = c;  
		int bytes = 0;  
  
		while ((c & 0x80) != 0) {  
			c = (c << 1) & 0xff;  
			bytes++;  
		}  
  
		bool bad_char = (bytes == 1);  
  
		if (bytes > 1)  
		{  
			bytes--;  
			code = c << (5 * bytes - 1);  
  
			while (bytes > 0) {  
				++iter;  
				bytes--;  
				c = (unsigned char)*iter;  
  
				if (iter >= text.end() || (c & 0xc0) != 0x80) {  
					bad_char = true;  
					break;  
				}  
  
				code |= (c & 0x3f) << (6 * bytes);  
			}  
		}  
  
		if (bad_char)  
		{  
			synfig::warning("TextProcessing: multibyte: %s",  
			                _("Can't parse multibyte character.\n"));  
			continue;  
		}  
  
		unicode.push_back(code);  
	}  
#endif  
  
	return unicode;  
}  
  
std::vector<TextLine>  
fetch_text_lines(const std::string& text, int direction)  
{  
	std::vector<TextLine> new_lines;  
  
	if (text.empty())  
		return new_lines;  
  
	std::vector<uint32_t> unicode;  
  
	{  
		std::string parsed_text = text;  
  
		// 0. Do some pre-parsing still in UTF-8  
		{  
			// 0.1 \r\n -> \n  
			// 0.2 \t -> 8 blank spaces  
			auto pos = parsed_text.find_first_of("\r\t");  
			while (pos != std::string::npos) {  
				if (parsed_text[pos] == '\t') {  
					const char *tab = "        ";  
					const int tab_length = 8;  
					parsed_text.replace(pos, 1, tab);  
					pos += tab_length;  
				} else if (/*parsed_text[pos] == '\r' &&*/ pos + 1 != std::string::npos && parsed_text[pos + 1] == '\n') {  
					parsed_text.erase(pos, 1);  
				}  
				pos = parsed_text.find_first_of("\r\t", pos);  
			}  
		}  
  
		// 1. Convert to unicode codepoints  
		unicode = utf8_to_utf32(parsed_text);  
	}  
  
	// 2. Split into lines  
	std::vector<std::vector<uint32_t>> base_lines;  
	{  
		auto it = unicode.begin();  
		while (true) {  
			auto new_it = std::find_first_of(it, unicode.end(), line_endings.begin(), line_endings.end());  
			std::vector<uint32_t> line{it, new_it};  
			base_lines.push_back(line);  
  
			if (new_it == unicode.end())  
				break;  
			it = new_it + 1;  
		}  
	}  
  
	// 3. Handle BiDirectional text  
#if HAVE_HARFBUZZ  
	for (auto& line : base_lines) {  
		FriBidiParType base_dir =  
			direction == TEXT_DIRECTION_AUTO ? FRIBIDI_TYPE_ON :  
			direction == TEXT_DIRECTION_LTR  ? FRIBIDI_TYPE_LTR :  
			                                   FRIBIDI_TYPE_RTL;  
  
		// FriBiDi + HarfBuzz: don't use FriBiDi simple shaper, just get the BiDi info / character reordering  
		size_t line_size = line.size();  
		std::vector<FriBidiCharType> bidi_types(line_size);  
		fribidi_get_bidi_types(line.data(), line_size, bidi_types.data());  
  
		std::vector<FriBidiLevel> bidi_levels(line_size);  
  
#if FRIBIDI_MAJOR_VERSION >= 1  
		std::vector<FriBidiBracketType> bracket_types(line_size);  
		FriBidiLevel fribidi_result = fribidi_get_par_embedding_levels_ex(  
			bidi_types.data(),  
			bracket_types.data(),  
			line_size,  
			&base_dir,  
			bidi_levels.data());  
#else  
		FriBidiLevel fribidi_result = fribidi_get_par_embedding_levels(  
			bidi_types.data(),  
			line_size,  
			&base_dir,  
			bidi_levels.data());  
#endif  
  
		if (fribidi_result == 0) {  
			synfig::error("TextProcessing: %s",  
			              _("error running FriBiDi (getting embedding levels)"));  
			return new_lines;  
		}  
  
		fribidi_result = fribidi_reorder_line(  
			FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC,  
			bidi_types.data(),  
			line_size,  
			0,  
			base_dir,  
			bidi_levels.data(),  
			line.data(),  
			nullptr);  
  
		if (fribidi_result == 0) {  
			synfig::error("TextProcessing: %s",  
			              _("error running FriBiDi (reordering line)"));  
			return new_lines;  
		}  
	}  
#endif  
  
	// 4. Split text into lines (and text spans according to their script if we know them)  
#if !HAVE_HARFBUZZ  
	for (const auto& line : base_lines) {  
		TextLine current_line;  
  
		for (uint32_t codepoint : line) {  
			if (!current_line.empty()) {  
				current_line.back().codepoints.push_back(codepoint);  
			} else {  
				current_line.push_back(TextSpan{{codepoint}});  
			}  
		}  
		new_lines.push_back(current_line);  
	}  
#else  
	hb_unicode_funcs_t* ufuncs = hb_unicode_funcs_get_default();  
  
	for (const auto& line : base_lines) {  
		TextLine current_line;  
		hb_script_t current_script = HB_SCRIPT_INVALID;  
  
		for (uint32_t codepoint : line) {  
			hb_script_t script = hb_unicode_script(ufuncs, codepoint);  
  
			if (!current_line.empty() && (script == current_script || script == HB_SCRIPT_INHERITED)) {  
				current_line.back().codepoints.push_back(codepoint);  
			} else {  
				current_line.push_back(TextSpan{  
					{codepoint},  
					script  
				});  
				current_script = script;  
			}  
		}  
  
		new_lines.push_back(current_line);  
	}  
#endif  
  
	return new_lines;  
}  
  
void  
convert_outline_to_contours(  
	const FT_OutlineGlyphRec* glyph,  
	rendering::Contour::ChunkList& chunks)  
{  
	chunks.clear();  
  
	if (!glyph) {  
		synfig::error(strprintf("TextProcessing: %s", _("Outline Glyph is null!")));  
		return;  
	}  
  
	if (glyph->outline.n_contours == 0) {  
		// No contours? OK, it can be a whitespace  
		return;  
	}  
  
	rendering::Contour contour;  
	FT_Outline outline = glyph->outline;  
	FT_Outline_Funcs outline_funcs;  
  
	outline_funcs.move_to = [](const FT_Vector* to, void* contour) -> int {  
		static_cast<rendering::Contour*>(contour)->move_to(
    	Vector(to->x, to->y));  
		return 0;  
	};  
	outline_funcs.line_to = [](const FT_Vector* to, void* contour) -> int {  
		static_cast<rendering::Contour*>(contour)->line_to(
    	Vector(to->x, to->y));  
		return 0;  
	};  
	outline_funcs.conic_to = [](const FT_Vector* control, const FT_Vector* to, void* contour) -> int {  
		static_cast<rendering::Contour*>(contour)->conic_to(
    	Vector(to->x, to->y),
    	Vector(control->x, control->y));  
		return 0;  
	};  
	outline_funcs.cubic_to = [](const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* contour) -> int {  
		static_cast<rendering::Contour*>(contour)->cubic_to(
    	Vector(to->x, to->y),
    	Vector(control1->x, control1->y),
    	Vector(control2->x, control2->y));  
		return 0;  
	};  
	outline_funcs.delta = FT_Pos(0);  
	outline_funcs.shift = 0;  
  
	FT_Outline_Decompose(&outline, &outline_funcs, &contour);  
	contour.close();  
	chunks = contour.get_chunks();  
}  
  
void  
shift_contour_chunks(  
	rendering::Contour::ChunkList& chunks,  
	const Vector& offset)  
{  
	for (auto& chunk : chunks) {  
		chunk.p1 += offset;  
		chunk.pp0 += offset;  
		chunk.pp1 += offset;  
	}  
}  

#if HAVE_HARFBUZZ  
std::vector<std::vector<ShapedGlyph>>  
shape_text(const std::vector<TextLine>& lines, hb_font_t* font)  
{  
	std::vector<std::vector<ShapedGlyph>> result;  
  
	if (!font)  
		return result;  
  
	hb_buffer_t* buffer = hb_buffer_create();  
	std::unique_ptr<hb_buffer_t, decltype(&hb_buffer_destroy)> safe_buf(buffer, hb_buffer_destroy);  
  
	for (const TextLine& line : lines)  
	{  
		std::vector<ShapedGlyph> shaped_line;  
  
		for (const TextSpan& span : line)  
		{  
			hb_buffer_clear_contents(buffer);  
  
			hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);  
			hb_buffer_set_script(buffer, span.script);  
  
			hb_buffer_add_utf32(  
				buffer,  
				span.codepoints.data(),  
				span.codepoints.size(),  
				0,  
				-1  
			);  
  
			hb_shape(font, buffer, nullptr, 0);  
  
			unsigned int glyph_count = 0;  
			hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);  
  			hb_glyph_position_t* glyph_pos  = hb_buffer_get_glyph_positions(buffer, &glyph_count);
			for (unsigned int i = 0; i < glyph_count; ++i)  
			{  
				ShapedGlyph g;  
				g.glyph_index = glyph_info[i].codepoint;  
				g.cluster = glyph_info[i].cluster; 
				g.x_advance   = glyph_pos[i].x_advance;  
    			g.y_advance   = glyph_pos[i].y_advance;  
    			g.x_offset    = glyph_pos[i].x_offset;  
    			g.y_offset    = glyph_pos[i].y_offset;  
				shaped_line.push_back(g);  

			}  
		}  
  
		result.push_back(std::move(shaped_line));  
	}  
  
	return result;  
}  
#else  
std::vector<std::vector<ShapedGlyph>>  
shape_text(const std::vector<TextLine>& lines)  
{  
	std::vector<std::vector<ShapedGlyph>> result;  
  
	for (const TextLine& line : lines)  
	{  
		std::vector<ShapedGlyph> shaped_line;  
  
		for (const TextSpan& span : line)  
		{  
			for (size_t i = 0; i < span.codepoints.size(); ++i)  
			{  
				ShapedGlyph g;  
				g.glyph_index = span.codepoints[i];  
				g.cluster = static_cast<uint32_t>(i);  
				shaped_line.push_back(g);  
				
			}  
		}  
  
		result.push_back(std::move(shaped_line));  
	}  
  
	return result;  
}  
#endif  

}
} // namespace synfig
