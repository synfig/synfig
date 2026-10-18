#ifndef fontloader_h_INCLUDED
#define fontloader_h_INCLUDED


/* === H E A D E R S ======================================================= */
 
#include <synfig/filesystemnative.h>
#include <synfig/string.h>
#include <synfig/localization.h>
#include <synfig/general.h>
 
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
 
#if HAVE_HARFBUZZ
#include <hb.h>
#endif
 
#include <map>
#include <mutex>
#include <string>
#include <vector>
 
/* === M A C R O S ========================================================= */
 
/* === T Y P E D E F S ===================================================== */
 
/* === C L A S S E S & S T R U C T S ======================================= */
 
namespace synfig
{
 
/// Metadata about a font. Used for font face cache indexing
struct FontMeta {
	synfig::String family;
	int style;
	int weight;
	//! Canvas file path if loaded font face file depends on it.
	//!  Empty string otherwise
	std::string canvas_path;
 
	explicit FontMeta(synfig::String family, int style=0, int weight=400)
		: family(std::move(family)), style(style), weight(weight)
	{}
 
	bool operator==(const FontMeta& other) const
	{
		return family == other.family && style == other.style && weight == other.weight && canvas_path == other.canvas_path;
	}
 
	bool operator<(const FontMeta& other) const
	{
		if (family < other.family)
			return true;
		if (family != other.family)
			return false;
 
		if (style < other.style)
			return true;
		if (style > other.style)
			return false;
 
		if (weight < other.weight)
			return true;
		if (weight > other.weight)
			return false;
 
		if (canvas_path < other.canvas_path)
			return true;
 
		return false;
	}
};
 
/**
 * Map font filenames or font metadata to their FreeType faces
 */
struct FaceCache
{
	FaceCache() = default;
 
	/**
	 * Get the Face associated to @a meta.
	 *
	 * Returned value should not not be freed.
	 * If you want to remove it from cache, use remove().
	 */
	FT_Face get(const FontMeta& meta) const {
		std::lock_guard<std::mutex> lock(cache_mutex_);
		auto iter = meta_cache_.find(meta);
		if (iter != meta_cache_.end())
			return iter->second;
		return nullptr;
	}
 
	/**
	 * Get the Face associated to @a path.
	 *
	 * Returned value should not not be freed.
	 * If you want to remove it from cache, use remove().
	 */
	FT_Face get(const filesystem::Path& path) const {
		std::lock_guard<std::mutex> lock(cache_mutex_);
		auto iter = file_cache_.find(path);
		if (iter != file_cache_.end())
			return iter->second;
		return nullptr;
	}
 
	void put(const FontMeta& meta, FT_Face face) {
		if (!face) {
			synfig::warning(_("Trying to cache a NULL face of font %s. Ignored."), meta.family.c_str());
			return;
		}
		std::lock_guard<std::mutex> lock(cache_mutex_);
		meta_cache_[meta] = face;
	}
 
	void put(const filesystem::Path& path, FT_Face face) {
		if (!face) {
			synfig::warning(_("Trying to cache a NULL face of font %s. Ignored."), path.u8_str());
			return;
		}
		std::lock_guard<std::mutex> lock(cache_mutex_);
		file_cache_[path] = face;
	}
 
	bool has(const FontMeta& meta) const {
		std::lock_guard<std::mutex> lock(cache_mutex_);
		auto iter = meta_cache_.find(meta);
		return iter != meta_cache_.end();
	}
 
	bool has(const filesystem::Path& path) const {
		std::lock_guard<std::mutex> lock(cache_mutex_);
		auto iter = file_cache_.find(path);
		return iter != file_cache_.end();
	}
 
	void remove(const FontMeta& meta) {
		std::lock_guard<std::mutex> lock(cache_mutex_);
		meta_cache_.erase(meta);
	}
 
	void remove(const filesystem::Path& path) {
		std::lock_guard<std::mutex> lock(cache_mutex_);
		file_cache_.erase(path);
	}
 
	void clear()
	{
		std::lock_guard<std::mutex> lock(cache_mutex_);
		for (const auto& item : file_cache_)
			FT_Done_Face(item.second);
		file_cache_.clear();
		meta_cache_.clear();
	}
 
	~FaceCache()
	{
		clear();
	}
 
	FaceCache(const FaceCache&) = delete; // Copy prohibited
	void operator=(const FaceCache&) = delete; // Assignment prohibited
	FaceCache(FaceCache&&) = delete; // Move constructor prohibited
	FaceCache& operator=(FaceCache&&) = delete; // Move assignment prohibited
 
private:
	std::map<filesystem::Path, FT_Face> file_cache_;
	std::map<FontMeta, FT_Face> meta_cache_;
	mutable std::mutex cache_mutex_;
};
 
/**
 * Metadata to be stored in FT_Face->generic field
 */
struct FaceMetaData
{
	filesystem::Path path;
#if HAVE_HARFBUZZ
	hb_font_t* font{nullptr};
#endif
 
	static FaceMetaData&
	get_from_face(FT_Face face)
	{
		return *static_cast<FaceMetaData*>(face->generic.data);
	}
 
	static void
	add_to_face(FT_Face face, filesystem::Path path)
	{
		if (face->generic.data)
			face->generic.finalizer(face);
		face->generic.data = new FaceMetaData{path};
		face->generic.finalizer = FaceMetaData::self_destroy;
	}
 
#if HAVE_HARFBUZZ
	static void
	add_to_face(FT_Face face, const filesystem::Path& path, hb_font_t* font)
	{
		if (face->generic.data)
			face->generic.finalizer(face);
		face->generic.data = new FaceMetaData{path, font};
		face->generic.finalizer = FaceMetaData::self_destroy;
	}
#endif
private:
	explicit FaceMetaData(filesystem::Path path)
		: path(path)
	{ }
 
#if HAVE_HARFBUZZ
	FaceMetaData(filesystem::Path path, hb_font_t* font)
		: path(path), font(font)
	{ }
#endif
 
	static void
	self_destroy(void* object)
	{
		FT_Face face = static_cast<FT_Face>(object);
		FaceMetaData* meta_data = static_cast<FaceMetaData*>(face->generic.data);
		face->generic.data = nullptr;
#if HAVE_HARFBUZZ
		hb_font_destroy(meta_data->font);
#endif
		delete meta_data;
	}
};
 
//!	\brief Shared font-loading component.
//!
//! Holds the font face cache and the family/style/weight -> font file
//! resolution logic that both Layer_Freetype and Layer_TextGroup rely on,
//! so the two layers no longer keep separate copies of this logic.
class FontLoader
{
public:
	//! The face cache shared by every user of the font loader.
	static FaceCache face_cache;
 
	static bool has_valid_font_extension(const std::string &filename);
 
	/// Try to map a font family to a filename (without extension nor directory)
	static void get_possible_font_filenames(synfig::String family, int style, int weight, std::vector<std::string>& list);
 
	static std::vector<std::string> get_possible_font_directories(const std::string& canvas_path);
	static std::vector<std::string> get_possible_font_files(const std::string& newfont, const synfig::filesystem::Path& canvas_path);
 
#ifdef WITH_FONTCONFIG
	static std::string fontconfig_get_filename(const std::string& font_fam, int style, int weight);
#endif
 
	//! Result of a font resolution. Owns nothing: the FT_Face lives in
	//! face_cache and the hb_font_t in the face's FaceMetaData.
	struct LoadedFont
	{
		FT_Face face = nullptr;
#if HAVE_HARFBUZZ
		hb_font_t* font = nullptr;
#endif
		//! true when the file was found relative to the canvas directory
		bool path_from_canvas = false;

		explicit operator bool() const { return face != nullptr; }
	};

	//! Load a face from a filename/path, searching canvas dir + system font dirs.
	static LoadedFont load_face(const std::string& newfont,
	                            const synfig::filesystem::Path& canvas_path);

	//! Resolve family/style/weight exactly once, no fallbacks.
	static LoadedFont load_font_once(const synfig::String& family, int style, int weight,
	                                 const synfig::filesystem::Path& canvas_path);

	//! Resolve family/style/weight, progressively simplifying style/weight and
	//! finally falling back to "sans serif". This is the entry point layers use.
	static LoadedFont load_font(const synfig::String& family, int style = 0, int weight = 400,
	                            const synfig::filesystem::Path& canvas_path = synfig::filesystem::Path());


	//! Canvas file path -> directory prefix with trailing separator.
	static std::string canvas_font_dir(const synfig::filesystem::Path& canvas_path); 
};
 
} // namespace synfig
 
/* === E N D =============================================================== */


#endif // fontloader_h_INCLUDED
