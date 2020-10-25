/* === S Y N F I G ========================================================= */
/*!	\file trgt_gif.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TRGT_GIF_H
#define __SYNFIG_TRGT_GIF_H

/* === H E A D E R S ======================================================= */

#include <synfig/target_scanline.h>
#include <synfig/string.h>
#include <synfig/smartfile.h>
#include <cstdio>
#include <synfig/surface.h>
#include <synfig/palette.h>
#include <synfig/targetparam.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class gif : public synfig::Target_Scanline
{
	SYNFIG_TARGET_MODULE_EXT
private:
	// Class for abstracting the
	// output of the codes
	struct bitstream
	{
		synfig::SmartFILE file;
		unsigned char pool;
		char curr_bit;
		bitstream():pool(0),curr_bit(0),curr_pos(0) {}
		bitstream(synfig::SmartFILE file):file(file),pool(0),curr_bit(0),curr_pos(0) {}
		unsigned char buffer[256];
		int curr_pos;

		// Pushes a single bit onto the bit
		void push_bit(bool bit)
		{
			if(bit)
				pool|=(1<<(curr_bit));
			curr_bit++;
			if(curr_bit==8)
				empty();
		}

		// Empties out the current pool into
		// the buffer. Calls 'dump()' if the
		// buffer is full.
		void empty()
		{
			buffer[curr_pos++]=pool;
			curr_bit=0;
			pool=0;
			if(curr_pos==255)dump();
		}

		// If there is anything in the
		// buffer or in the pool, it
		// dumps it to the filestream.
		// Buffer and pool are cleared.
		void dump()
		{
			if(curr_bit)
				empty();
			if(curr_pos || curr_bit)
			{
				fputc(curr_pos,file.get());
				fwrite(buffer,curr_pos,1,file.get());
				curr_pos=0;
			}
		}

		// Pushes a symbol of the given size
		// onto the bitstream.
		void push_value(int value, int size)
		{
			int i;
			for(i=0;i<size;i++)
				push_bit((value>>(i))&1);
		}
	};

	// Class for dealing with the LZW codes
	struct lzwcode
	{
		int value; // the data element or character
		int code; // lzwcode
		struct lzwcode* kids; // children of this node
		struct lzwcode* next; // siblings of this node

		lzwcode():value(0),code(0),kids(0),next(0) { }

		lzwcode *FindCode(int value)
		{
			lzwcode *node=this;

			// check the children (kids) of the node for the value
			for (node = node->kids; node != 0; node = node->next)
				if (node->value == value)
					return(node);
			return(0);
		}

		void AddNode(unsigned short code, unsigned short value)
		{
			lzwcode *n = new lzwcode;

			// add a new child to node; the child will have code and value
			n->value = value;
			n->code = code;
			n->kids = 0;
			n->next = this->kids;
			this->kids = n;
		}

		static lzwcode * NewTable(int values)
		{
			int i;
			lzwcode * table = new lzwcode;

			table->kids = 0;
			for (i = 0; i < values; i++)
				table->AddNode( i, i);

			return(table);
		}

		// Destructor just deletes any
		// children and siblings.
		~lzwcode()
		{
			if(kids)
				delete kids;
			if(next)
				delete next;
		}
	};

private:
	bitstream bs;
	synfig::String filename;
	synfig::SmartFILE file;
	int
		codesize,	// Current code size
		rootsize,	// Size of pixel bits (will be recalculated)
		nextcode;	// Next code to use
	lzwcode *table,*next,*node;

	synfig::Surface curr_surface;
	etl::surface<unsigned char> curr_frame;
	etl::surface<unsigned char> prev_frame;

	int imagecount;
	int cur_scanline;


	// GIF compression parameters
	bool lossy;
	bool multi_image;
	bool dithering;
	int color_bits;
	int iframe_density;
	int loop_count;
	bool local_palette;

	synfig::Palette curr_palette;

	void output_curr_palette();

public:
	gif(const char *filename, const synfig::TargetParam& /* params */);

	virtual bool set_rend_desc(synfig::RendDesc *desc);
	virtual bool init(synfig::ProgressCallback *cb);
	virtual bool start_frame(synfig::ProgressCallback *cb);
	virtual void end_frame();

	virtual ~gif();

	virtual synfig::Color * start_scanline(int scanline);
	virtual bool end_scanline(void);

};

/* === E N D =============================================================== */

#endif
