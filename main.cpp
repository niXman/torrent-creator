
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/progress.hpp>

#include <libtorrent/create_torrent.hpp>
#include <libtorrent/bencode.hpp>

/***************************************************************************/

struct progress_indicator {
	progress_indicator(int max)
		:progress(max)
	{}
	/**  */
	boost::progress_display progress;

	void operator()(int v) {
		progress += v;
	}
};

/***************************************************************************/

int main(int argc, char** argv) {
	try {
		std::string path;
		std::string fname;
		std::string torrent_name;
		std::string tracker;
		std::string creator = "libtorrent-rasterbar";
		std::string comment;
		libtorrent::size_type piece_size = 0;
		/**  */
		boost::program_options::options_description description("allowed options");
		description.add_options()
			("help,h"   ,                                                                         "print help message")
			("dir,d"    , boost::program_options::value<std::string>(&path)                     , "read directory tree")
			("file,f"   , boost::program_options::value<std::string>(&fname)->required()        , "use this file")
			("name,n"   , boost::program_options::value<std::string>(&torrent_name)->required() , "set torrent name")
			("tracker,t", boost::program_options::value<std::string>(&tracker)                  , "set tracker url")
			("creator,c", boost::program_options::value<std::string>(&creator)                  , "set creator id")
			("comment,m", boost::program_options::value<std::string>(&comment)                  , "set comments")
			("piece,p"  , boost::program_options::value<libtorrent::size_type>(&piece_size)     , "set piece size");
		/**  */
		boost::program_options::variables_map options;
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), options);
		boost::program_options::notify(options);
		/**  */
		if ( options.count("help") ) {
			std::cout << description << std::endl;
			return 0;
		}

		if ( !fname.empty() && !path.empty() ) {
			std::cout << "only one of \"file\" or \"path\" must be specified" << std::endl;
			return 0;
		}

		std::string full_path;
		libtorrent::file_storage fs;

		if ( !path.empty() ) {
			libtorrent::add_files(fs, path);
		} else {
			libtorrent::add_files(fs, fname);
		}

		libtorrent::create_torrent torrent(fs, piece_size);

		if ( !tracker.empty() ) { torrent.add_tracker(tracker); }
		if ( !creator.empty() ) { torrent.set_creator(creator.c_str()); }
		if ( !comment.empty() ) { torrent.set_comment(comment.c_str()); }

		progress_indicator progress(torrent.piece_length());

		libtorrent::set_piece_hashes(torrent, ".", boost::ref(progress));

		std::ofstream file(torrent_name.c_str(), std::ios::binary|std::ios::trunc);
		if ( !file ) {
			throw std::runtime_error("can`t create torrent file!");
		}

		libtorrent::bencode(std::ostream_iterator<char>(file), torrent.generate());

	} catch (const std::exception& ex) {
		std::cout << "exception: " << ex.what() << std::endl;
		return 1;
	}
	return 0;
}

/***************************************************************************/
