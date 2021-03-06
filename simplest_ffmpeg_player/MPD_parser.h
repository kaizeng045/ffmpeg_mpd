#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>
//#include <unistd.h>
#include <vector>

#include <curl/curl.h>

using namespace std;

struct Audio_info{
	string mimeType;
	string codec;
	string lang;
	string subsegmentAli;
	string subsegmentStartwithSAP;
	string id;
	string bandwidth;
	vector<string> URL;
	string audiosamplerate;
	string startwithSAP;
	string startNumber;
	string timescale;
	string duration;
	string media;
	string initialization;
};

struct Video_info{
	string mimeType;
	string codec;
	string subsegmentAli;
	string subsegmentStartwithSAP;
	string contentType;
	string id;
	string bandwidth;
	string width;
	string height;
	string sar;
	string framerate;
	string maxwidth;
	string maxheight;
	string maxframerate;
	string par;
	vector<string> URL;
	string startNumber;
	string timescale;
	string duration;
	string media;
	string initialization;
};
class MPD_parser
{
public:
	MPD_parser();
	~MPD_parser();	

//<<<<<<< Updated upstream
//	string master_link;
//
//	int mpdparser(char* path, std::string* fname);
//=======
	string masterURL;
	vector<Audio_info> audio;
	vector<Video_info> video;
	int mpd2file(string mpdpath, string* fname_in);
	int mpdparser(string path, string* fname);
	int mpdparser_libxml2(string path, string* fname, string mpdpath);
//>>>>>>> Stashed changes
};

