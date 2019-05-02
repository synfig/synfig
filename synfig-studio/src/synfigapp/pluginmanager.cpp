/* === S Y N F I G ========================================================= */
/*!	\file synfigapp/pluginmanager.cpp
**	\brief  Plugin Manager responsible for loading plugins
**
**	$Id$
**
**	\legal
**	Copyright (c) 2012-2013 Konstantin Dmitriev
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

#include "pluginmanager.h"

#include <iostream>

#include <libxml++/libxml++.h>

#include <dirent.h>
#include <sys/stat.h>

#include <synfig/general.h>
#include <synfig/savecanvas.h>
#include <synfig/filesystemnative.h>
#include <synfigapp/main.h>

#include <synfigapp/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;

/* === M A C R O S ========================================================= */


/* === G L O B A L S ======================================================= */

/* === M E T H O D S ======================================================= */

PluginLauncher::PluginLauncher(synfig::Canvas::Handle canvas)
{
	// Save the original filename
	filename_original = canvas->get_file_name();

	String filename_base;
	if (is_absolute_path(filename_original))
	{
		filename_base = filename_original;
	} else {
		filename_base = synfigapp::Main::get_user_app_directory()+ETL_DIRECTORY_SEPARATOR+"tmp"+ETL_DIRECTORY_SEPARATOR+filename_original;
	}

	// Make random filename and ensure there's no file with such name exist
	struct stat buf;

	// Filename to save the file for processing
	do {
		synfig::GUID guid;
		filename_processed = filename_base+"."+guid.get_string().substr(0,8)+".sif"; // without .sif suffix it won't be read back
	} while (stat(filename_processed.c_str(), &buf) != -1);

	/* The plugin could die with nonzero exit code
	 * synfig could crash loading the modified file (should not happen)
	 * having a backup file should protect against both cases
	 */
	do {
		synfig::GUID guid;
		filename_backup = filename_base+"."+guid.get_string().substr(0,8)+".sif";
	} while (stat(filename_backup.c_str(), &buf) != -1);

	save_canvas(FileSystemNative::instance()->get_identifier(filename_processed),canvas);
	// copy file would be faster ..
	save_canvas(FileSystemNative::instance()->get_identifier(filename_backup),canvas);

	//canvas=0;
	exitcode=-1;
	output="";
}

bool
PluginLauncher::check_python_version(String path)
{
	String command;
	String result;
	command = path + " --version 2>&1";
	FILE* pipe = popen(command.c_str(), "r");
	if (!pipe) {
		return false;
	}
	char buffer[128];
	while(!feof(pipe)) {
		if(fgets(buffer, 128, pipe) != NULL)
				result += buffer;
	}
	pclose(pipe);
	// Output is like: "Python 3.3.0"
	if (result.substr(7,1) != "3"){
		return false;
	}
	return true;
}

bool
#ifdef _WIN32
PluginLauncher::execute( std::string script_path, const std::string& synfig_root )
#else
PluginLauncher::execute( std::string script_path, const std::string& /* synfig_root */ )
#endif
{
	String command = "";
	
	// Path to python binary can be overridden
	// with SYNFIG_PYTHON_BINARY env variable:
	char* custom_python_binary=getenv("SYNFIG_PYTHON_BINARY");
	if(custom_python_binary) {
		command=custom_python_binary;
		if (!check_python_version(command)) {
			output="Error: You need to have Python 3 installed.";
			return false;
		}
	} else {
	// Set path to python binary depending on the os type.
	// For Windows case Python binary is expected
	// at INSTALL_PREFIX/python/python.exe
		std::list< String > binary_choices;
		binary_choices.push_back("python");
		binary_choices.push_back("python3");
		std::list< String >::iterator iter;
		for(iter=binary_choices.begin();iter!=binary_choices.end();iter++)
		{
			String python_path;
#ifdef _WIN32
			python_path = "\"" + synfig_root+ETL_DIRECTORY_SEPARATOR+"python"+ETL_DIRECTORY_SEPARATOR+*iter+".exe" + "\"";
#else
			python_path = *iter;
#endif
			if (check_python_version(python_path))
			{
				command = python_path;
				break;
			}
			
		}
		if (command == "")
		{
			output=_("Error: No Python 3 binary found.\n\nHint: You can set SYNFIG_PYTHON_BINARY environment variable pointing at your custom python installation.");
			return false;
		}
	}
	synfig::info("Python 3 binary found: "+command);
	
	
	// Construct the full command:
	command = command+" \""+script_path+"\" \""+filename_processed+"\" 2>&1";
#ifdef _WIN32
	// This covers the dumb cmd.exe behavior.
	// See: http://eli.thegreenplace.net/2011/01/28/on-spaces-in-the-paths-of-programs-and-files-on-windows/
	command = "\"" + command + "\"";
#endif
	
	FILE* pipe = popen(command.c_str(), "r");
	if (!pipe) {
		output = "ERROR: pipe failed!";
		return false;
	}
	char buffer[128];
	while(!feof(pipe)) {
		if(fgets(buffer, 128, pipe) != NULL)
				output += buffer;
	}
	
	if (output != "" ){
		synfig::info(output);
	}
	
	exitcode=pclose(pipe);

	if (0==exitcode){
		return true;
	} else {
		return false;
	}
}

std::string
PluginLauncher::get_result_path()
{
	if (0==exitcode){
		return filename_processed;
	} else {
		return filename_backup;
	}
}

PluginLauncher::~PluginLauncher()
{
	remove( filename_processed.c_str() );
	remove( filename_backup.c_str() );
}

PluginManager::PluginManager():
	list_()
{
} // END of synfigapp::PluginManager::PluginManager()

void
PluginManager::load_dir( const std::string &pluginsprefix )
{
	
	synfig::info("Loading plugins from %s", pluginsprefix.c_str());
	
	DIR *dir;
	struct dirent *entry;
	
	dir = opendir(pluginsprefix.c_str());
	if(dir) {
		while ( (entry = readdir(dir)) != NULL) {
			if ( std::string(entry->d_name) != std::string(".") && std::string(entry->d_name) != std::string("..") ) {
				std::string pluginpath;
				pluginpath = pluginsprefix+ETL_DIRECTORY_SEPARATOR+entry->d_name;
				struct stat sb;
				stat(pluginpath.c_str(), &sb);
				// error handling if stat failed
				if (S_ISDIR(sb.st_mode)) {
					// checking if directory contains a plugin...
					DIR *plugindir;
					struct dirent *plugindirentry;
					
					plugindir = opendir(pluginpath.c_str());
					if(plugindir) {
						while ( (plugindirentry = readdir(plugindir)) != NULL) {
							if ( std::string(plugindirentry->d_name) == std::string("plugin.xml") ){
								std::string pluginfilepath;
								pluginfilepath = pluginpath+ETL_DIRECTORY_SEPARATOR+plugindirentry->d_name;
								
								load_plugin(pluginfilepath);
							}
						}
						closedir(plugindir);
					} 
					else 
						synfig::warning("Can't read plugin directory!");

					/*plugindir = opendir(pluginpath.c_str());
					if(!plugindir) {
						synfig::warning("Can't read plugin directory!");
						return;
					}
					
					while ( (plugindirentry = readdir(plugindir)) != NULL) {
						if ( std::string(plugindirentry->d_name) == std::string("plugin.xml") ){
							std::string pluginfilepath;
							pluginfilepath = pluginpath+ETL_DIRECTORY_SEPARATOR+plugindirentry->d_name;
							
							load_plugin(pluginfilepath);
						}
					}*/
					
				}
			}

		};
		
		closedir(dir);
	}
} // END of synfigapp::PluginManager::load_dir()

void
PluginManager::load_plugin( const std::string &path )
{
	// Get locale
	std::string current_locale = setlocale(LC_ALL, NULL);
	
	synfig::info("   Loading plugin: %s", basename(dirname(path)).c_str());
							
	PluginManager::plugin p;
	std::string plugindir = dirname(path);
	p.id=plugindir;
	
	// parse xml file
	try
	{
		xmlpp::DomParser parser;
		//parser.set_validate();
		parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
		parser.parse_file(path);
		if(parser)
		{
			//Walk the tree:
			const xmlpp::Node* pNode = parser.get_document()->get_root_node(); //deleted by DomParser.
			if ( std::string(pNode->get_name()) == std::string("plugin") ){
				//Recurse through child nodes:
				xmlpp::Node::NodeList list = pNode->get_children();
				
				unsigned int name_relevance = 0;
				
				for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
				{
					const xmlpp::Node* node = *iter;
					if ( std::string(node->get_name()) == std::string("name") ) {

						const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
						
						xmlpp::Node::NodeList l = nodeElement->get_children();
						xmlpp::Node::NodeList::iterator i = l.begin();
						xmlpp::Node* n = *i;
						
						const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(n);
						
						if(nodeText)
						{
							// Get the language attribute
							const xmlpp::Attribute* langAttribute = nodeElement->get_attribute("lang", "xml");

							if (langAttribute) {
								// Element have language attribute,
								std::string lang = langAttribute->get_value();
								// let's compare it with current locale
								 if (!current_locale.compare(0, lang.size(), lang)) {
									 if (lang.size() > name_relevance){
										 p.name=nodeText->get_content();
									 }
								 }
							} else {
								// Element have no language attribute - use as fallback
								if (name_relevance == 0){
									p.name=nodeText->get_content();
								}
							}
						}
						
					} else if ( std::string(node->get_name()) == std::string("exec") ) {
						
						xmlpp::Node::NodeList l = node->get_children();
						xmlpp::Node::NodeList::iterator i = l.begin();
						xmlpp::Node* n = *i;
						
						const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(n);
						
						if(nodeText)
						{
							p.path=plugindir+ETL_DIRECTORY_SEPARATOR+nodeText->get_content();
						}
					}
				}
			} else {
				synfig::info("Invalid plugin.xml file.");
			}
		}
	}
	catch(const std::exception& ex)
	{
		std::cout << "Exception caught: " << ex.what() << std::endl;
	}
	
	if ( p.id != "" && p.name != "" && p.path != ""){
		list_.push_back(p);
	} else {
		synfig::warning("Invalid plugin.xml file!");
	}
}

PluginManager::~PluginManager()
{
}
