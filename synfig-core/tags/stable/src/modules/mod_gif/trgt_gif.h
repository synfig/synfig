/*! ========================================================================
** Sinfg
** Template Header File
** $Id: trgt_gif.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_TRGT_GIF_H
#define __SINFG_TRGT_GIF_H

/* === H E A D E R S ======================================================= */

#include <sinfg/target_scanline.h>
#include <sinfg/string.h>
#include <sinfg/smartfile.h>
#include <cstdio>
#include <sinfg/surface.h>
#include <sinfg/palette.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class gif : public sinfg::Target_Scanline
{
	SINFG_TARGET_MODULE_EXT
private:
	// Class for abstracting the
	// output of the codes
	struct bitstream
	{
		sinfg::SmartFILE file;
		unsigned char pool;
		char curr_bit;
		bitstream():pool(0),curr_bit(0),curr_pos(0) {}
		bitstream(sinfg::SmartFILE file):file(file),pool(0),curr_bit(0),curr_pos(0) {}
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
		
		// Emptys out the current pool into
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
		// children and sibblings.
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
	sinfg::String filename;
	sinfg::SmartFILE file;
	int 
		i,			// General-purpose index
		codesize,	// Current code size
		rootsize,	// Size of pixel bits (will be recalculted)
		nextcode;	// Next code to use
	lzwcode *table,*next,*node;
	
	sinfg::Surface curr_surface;
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
	
	sinfg::Palette curr_palette;
	
	void output_curr_palette();
	
public:
	gif(const char *filename);

	virtual bool set_rend_desc(sinfg::RendDesc *desc);
	virtual bool init();
	virtual bool start_frame(sinfg::ProgressCallback *cb);
	virtual void end_frame();

	virtual ~gif();
	
	virtual sinfg::Color * start_scanline(int scanline);
	virtual bool end_scanline(void);

};

/* === E N D =============================================================== */

#endif
