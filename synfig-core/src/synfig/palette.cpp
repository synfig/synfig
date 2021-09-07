/* === S Y N F I G ========================================================= */
/*!	\file palette.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2010 Nikita Kitaev
**	Copyright (c) 2010 Carlos López
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "palette.h"
#include "surface.h"
#include "general.h"
#include "filesystemnative.h"
#include <synfig/localization.h>
#include <fstream>
#include <iostream>
#include <sstream>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define PALETTE_SYNFIG_FILE_COOKIE	"SYNFIGPAL1.0"
#define PALETTE_SYNFIG_EXT ".spal"
#define PALETTE_GIMP_FILE_COOKIE "GIMP Palette"
#define PALETTE_GIMP_EXT ".gpl"

/* === G L O B A L S ======================================================= */

bool weight_less_than(const PaletteItem& lhs,const PaletteItem& rhs)
{
	return lhs.weight<rhs.weight;
}

bool luma_less_than(const PaletteItem& lhs,const PaletteItem& rhs)
{
	return lhs.color.get_y()<rhs.color.get_y();
}

bool luma_less_than(const PaletteItem& lhs,const float& rhs)
{
	return lhs.color.get_y()<rhs;
}

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Palette::Palette():
	name_(_("Unnamed"))
{
}

Palette::Palette(const String& name_):
	name_(name_)
{
}

void
PaletteItem::add(const Color& x,int xweight)
{
	color=(color*weight+x*xweight)/(weight+xweight);
	weight+=xweight;
}

Palette::Palette(const Surface& surface, int max_colors, const Gamma &gamma):
	name_(_("Surface Palette"))
{
	max_colors-=2;
	for(int i=0;(signed)size()<(max_colors-1) && i<max_colors*16;++i) {
        int x=rand()%surface.get_w();
        int y=rand()%surface.get_h();

			float dist;
			Color color(surface[y][x]);

			if(empty())
			{
				push_back(color);
				continue;
			}

			if(color.get_a()==0)
			{
				if(front().color.get_a()!=0)
					insert(begin(),Color(1,0,1,0));
				front().weight+=400;
				continue;
			}

			iterator iter(find_closest(color, gamma, &dist));
			if(sqrt(dist)<0.005)
			{
				iter->add(color);
				continue;
			}

			/*if(size()>=max_colors)
			{
				iterator iterlight(find_light());
				PaletteItem light(*iterlight);
				erase(iterlight);
				find_closest(light.color, gamma)->add(light.color,light.weight);
			}
			*/

			push_back(color);
			continue;
    }

/*

	max_colors-=2;
	for(int y=0;y<surface.get_h();y++)
		for(int x=0;x<surface.get_w();x++)
		{
			float dist;
			Color color(surface[y][x]);

			if(empty())
			{
				push_back(color);
				continue;
			}

			if(color.get_a()==0)
			{
				if(front().color.get_a()!=0)
					insert(begin(),Color(1,0,1,0));
				front().weight+=400;
				continue;
			}

			iterator iter(find_closest(color, gamma, &dist));
			if(sqrt(dist)<0.005)
			{
				iter->add(color);
				continue;
			}


			push_back(color);
			continue;
		}
	sort(rbegin(),rend());

	iterator iter;

	iterator best_match(begin());
	while((signed)size()>max_colors)
	{
		PaletteItem item(back());
		pop_back();
		find_closest(item.color, gamma)->add(item.color,item.weight);
	}
*/
	push_back(Color::black());
	push_back(Color::white());

//	sort(begin(),end(),&luma_less_than);
}

Palette::const_iterator
Palette::find_closest(const Color& color, const Gamma &gamma, float* dist)const
{
	// For the sake of avoiding cut-and-paste
	// bugs, we'll just use the non-const
	// find_closest()... It doesn't change anything
	// anyway.
	return const_cast<Palette*>(this)->find_closest(color, gamma, dist);
}

Palette::iterator
Palette::find_closest(const Color& color, const Gamma &gamma, float* dist)
{
	iterator iter;

	iterator best_match(begin());
	float best_dist(1000000);

	const Color prep = gamma.apply(color);
	const float prep_y(prep.get_y()*prep.get_a());
	const float prep_u(prep.get_u());
	const float prep_v(prep.get_v());

	for(iter=begin();iter!=end();++iter)
	{
		const Color ic = gamma.apply(iter->color);
		const float diff_y(prep_y - ic.get_y()*ic.get_a());
		const float diff_u(prep_u - ic.get_u());
		const float diff_v(prep_v - ic.get_v());
		const float diff_a(prep.get_a() - ic.get_a());


		const float dist(
			diff_y*diff_y*1.5f+
			diff_a*diff_a+

			diff_u*diff_u+
			diff_v*diff_v

			// cross product
			/*abs(
				prep_u*ic.get_u()-
				prep_v*ic.get_v()
			)*/
		);
		if(dist<best_dist)
		{
			best_dist=dist;
			best_match=iter;
		}
	}
	if(dist)
		*dist=best_dist;

	return best_match;
}


Palette::iterator
Palette::find_heavy()
{
	iterator iter;

	iterator best_match(begin());

	for(iter=begin();iter!=end();++iter)
	{
		if(iter->weight>best_match->weight)
			best_match=iter;
	}

	return best_match;
}

Palette::iterator
Palette::find_light()
{
	iterator iter;

	iterator best_match(begin());

	for(iter=begin();iter!=end();++iter)
	{
		if(iter->weight<best_match->weight)
			best_match=iter;
	}

	return best_match;
}

Palette
Palette::grayscale(int steps, ColorReal gamma)
{
	Palette ret;
	for(int i=0;i<steps;i++)
	{
		ColorReal amount = i/ColorReal(steps-1);
		ColorReal y = Gamma::calculate(amount, gamma);
		ret.push_back(
			PaletteItem(
				Color(y, y, y),
				strprintf(_("%0.2f%% Gray"), amount) ));
	}
	return ret;
}

void
Palette::save_to_file(const synfig::String& filename) const
{
	FileSystem::WriteStream::Handle file = FileSystemNative::instance()->get_write_stream(filename);

	if(!file)
		throw strprintf(_("Unable to open %s for write"),filename.c_str());

	*file << PALETTE_SYNFIG_FILE_COOKIE << "\n" << name_ << "\n";
	for (const_iterator iter=begin();iter!=end();++iter) {
		*file << iter->name << "\n"
		      << iter->color.get_r() << "\n"
			  << iter->color.get_g() << "\n"
			  << iter->color.get_b() << "\n"
			  << iter->color.get_a() << "\n";
	}
}

Palette
Palette::load_from_file(const synfig::String& filename)
{
	FileSystem::ReadStream::Handle file = FileSystemNative::instance()->get_read_stream(filename);

	if(!file)
		throw strprintf(_("Unable to open %s for read"),filename.c_str());

	Palette ret;
	std::string line("");
	const std::string ext(filename_extension(filename));


	if (ext==PALETTE_SYNFIG_EXT)
	{
		getline(*file, line);

		if(line!=PALETTE_SYNFIG_FILE_COOKIE)
			throw strprintf(_("%s does not appear to be a valid %s palette file"),filename.c_str(),"Synfig");

		getline(*file, ret.name_);

		while(!file->eof())	{
			PaletteItem item;
			String n;
			float r, g, b, a;
			getline(*file, item.name);
			*file >> r >> g >> b >> a;
			item.color.set_r(r);
			item.color.set_g(g);
			item.color.set_b(b);
			item.color.set_a(a);

			// file ends in new line
			if (!file->eof())
				ret.push_back(item);
		}
	}
	else if (ext==PALETTE_GIMP_EXT)
	{
		/*
		file format: GPL (GIMP Palette) file should have the following layout:
		GIMP Palette
		Name: <palette name>
		[Columns: <number>]
		[#]
		[# Optional comments]
		[#]
		<value R> <value G> <value B> <swatch name>
		<value R> <value G> <value B> <swatch name>
		... ...
		[<new line>]
		*/

		do {
			getline(*file, line);
		} while (!file->eof() && line != PALETTE_GIMP_FILE_COOKIE);

		if (line != PALETTE_GIMP_FILE_COOKIE)
			throw strprintf(_("%s does not appear to be a valid %s palette file"),filename.c_str(),"GIMP");


		bool has_color = false;

		do
		{
			getline(*file, line);

			if (!line.empty() && line.substr(0,5) == "Name:")
				ret.name_ = String(line.substr(6));
			else if (!line.empty() && line.substr(0,8) == "Columns:")
				; // Ignore columns
			else if (!line.empty() && line.substr(0,1) == "#")
				; // Ignore comments
			else if (!line.empty())
			{
				// not empty line not part of the header => color
				has_color = true;
				// line contains the first color so we put it back in (including \n)
				for (int i = line.length()+1; i; i--)
					file->unget();
			}
		} while (!file->eof() && !has_color);

		while(!file->eof() && has_color)
		{
			PaletteItem item;
			float r, g, b;

			std::stringstream ss;
			getline(*file, line);

			if (!line.empty())
			{
				ss << line.c_str();

			 	ss >> r >> g >> b;
				getline(ss, item.name);

				item.color.set_r(r/255);
				item.color.set_g(g/255);
				item.color.set_b(b/255);
				// Alpha is 1 by default
				item.color.set_a(1);

				ret.push_back(item);
			}
		}
	}
	else
		throw strprintf(_("%s does not appear to be a supported palette file"),filename.c_str());

	return ret;
}
