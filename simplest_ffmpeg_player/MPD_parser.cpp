#include "MPD_parser.h"
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <functional>
#include <locale>
#include <cctype>
#include <cstring>
#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <vector>
#include <sstream>
#include <vector>
#include <iomanip>

using namespace std;

MPD_parser::MPD_parser()
{
}

MPD_parser::~MPD_parser()
{
}




#ifdef LIBXML_READER_ENABLED

/**
* processNode:
* @reader: the xmlReader
*
* Dump information about the current node
*/
static void processNode(xmlTextReaderPtr reader) {
	const xmlChar *name, *value;

	name = xmlTextReaderConstName(reader);
	if (name == NULL)
		name = BAD_CAST "--";

	value = xmlTextReaderConstValue(reader);

	printf("%d %d %s %d %d",
		xmlTextReaderDepth(reader),
		xmlTextReaderNodeType(reader),
		name,
		xmlTextReaderIsEmptyElement(reader),
		xmlTextReaderHasValue(reader));
	if (value == NULL)
		printf("\n");
	else {
		/*if (xmlStrlen(value) > 40)
			printf(" %.40s...\n", value);
		else*/
			printf(" %s\n", value);
	}
}

/**
* example1Func:
* @filename: a filename or an URL
*
* Parse the resource and free the resulting tree
*/
//static void
//example1Func(const char *filename) {
//	
//	
//	
//	xmlParserCtxtPtr ptr;
//	ptr = xmlNewParserCtxt();
//	xmlDocPtr doc;
//	if (ptr == NULL) {
//		fprintf(stderr, "Failed to allocate parser context\n");
//		return;
//	}
//	doc = xmlCtxtReadFile(ptr, filename, NULL, XML_PARSE_DTDVALID);
//	if (doc == NULL) {
//		fprintf(stderr, "Failed to parse %s\n", filename);
//	}
//	else {
//		/* check if validation suceeded */
//		if (ptr->valid == 0)
//			fprintf(stderr, "Failed to validate %s\n", filename);
//		
//		while (true){
//			if (doc->children->children->name == "BaseURL"){
//				MPD_parser::baseURL = doc->children->children->children->content;
//			} else{
//				doc->children->children = doc->children->children->next;
//			}
//		}
//		
//		
//		
//		/* free up the resulting document */
//		xmlFreeDoc(doc);
//	}
//	/* free up the parser context */
//	xmlFreeParserCtxt(ptr);
//		
////	xmlDocPtr doc; /* the resulting document tree */
//
//	//doc = xmlReadFile(filename, NULL, 0);
//	//if (doc == NULL) {
////		fprintf(stderr, "Failed to parse %s\n", filename);
//	//	return;
//	//}
//	// the details in the doc!!!
//
//	//xmlFreeDoc(doc);
//}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
	return written;
}

int MPD_parser::mpd2file(string mpdpath, string* fname_in){
	CURL *curl_handle;
	static const char *pagefilename = "mpd.out";
	FILE *pagefile;

	curl_global_init(CURL_GLOBAL_ALL);

	/* init the curl session */
	curl_handle = curl_easy_init();

	/* set URL to get here */
	curl_easy_setopt(curl_handle, CURLOPT_URL, mpdpath);

	/* Switch on full protocol/debug output while testing */
	curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);

	/* disable progress meter, set to 0L to enable and disable debug output */
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

	/* send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

	/* open the file */
	pagefile = fopen(pagefilename, "wb");
	if (pagefile) {

		/* write the page body to this file handle */
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);

		/* get it! */
		curl_easy_perform(curl_handle);

		/* close the header file */
		fclose(pagefile);
	}

	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);

	string fname_in_t(pagefilename,7);
	*fname_in = fname_in_t;

	return 0;
}


/*
this is for check if children has a member named target.
*/
_xmlNode* str(_xmlNode * child, string target){
	while (child != NULL){
		ostringstream os1;
		os1 << child->name;
		string str = os1.str();
		if (str == target){
			break;
		}
		else{
			child = child->next;
		}
	}
	return child;
}

/*
to check if there exist some member of _xmlNode
*/
bool member(_xmlNode * child, string target){
	while (child->children != NULL){
		ostringstream os1;
		os1 << child->children->name;
		string str = os1.str();
		if (str == target){
			return true;
		}
		else{
			child->children = child->children->next;
		}
	}
	return false;
}

/*
count the number of children
*/
int counter(_xmlNode * child){
	int count = 0;
	while (child->children != NULL){
		ostringstream os1;
		os1 << child->children->name;
		string str = os1.str();
		if (str == "InbandEventStream" || str == "Role" || str == "BaseURL" || str == "SegmentTemplate" || str == "Representation" || str == "AudioChannelConfiguration"){
			count++;
		}
		child->children = child->children->next;
	}
	return count;
}

/*
this is to check if current children name is target.
*/
bool this_one(_xmlNode * child, string target){
	ostringstream os1;
	os1 << child->name;
	string str = os1.str();
	if (str == target) return true;
	return false;
}

/*
this is for returning xmlchar as std::string
*/
string eql(_xmlNode* xml, string mpdpath){
	if (xml == NULL){
		size_t found = mpdpath.find_last_of("/");
		string root;
		root = mpdpath.substr(0, found + 1);
		return root;
	}
	ostringstream os;
	os << xml->children->content;
	string name = os.str();
	return name;
}


string eql1(_xmlNode* xml, string mpdpath){
	if (xml == NULL){
		return "";
	}
	ostringstream os;
	os << xml->children->content;
	string name = os.str();
	return name;
}

/*
this is to return the properties named target of a current children
such as, return audio's codec, mimeType etc.
*/
string type(_xmlNode* xml, string target){
	while (true){
		ostringstream os;
		os << xml->properties->name;
		string name = os.str();
		if (name == target){
			ostringstream os1;
			os1 << xml->properties->children->content;
			string value = os1.str();
			return value;
		} else{
			xml->properties = xml->properties->next;
		}
	}

}


/*
this is a boolean function to check if children has a type of properties named target.
i.e, find if current audio has a type named codec, mimeType etc. in AdapatiationSet or Representation
*/
bool exist(_xmlNode* xml, string target){
	while (xml->properties != NULL){
		ostringstream os;
		os << xml->properties->name;
		string name = os.str();
		if (name == target){
			return true;
		}
		else{
			xml->properties = xml->properties->next;
		}
	}
	return false;
}

/*
this is to return how many times representation occurs in current children.
*/
int times(_xmlNode * xml){
	int counter = 0;
	while (xml != NULL){
		ostringstream os;
		os << xml->name;
		string name = os.str();
		if (name == "Representation"){
			counter++;
			xml = xml->next;
		}
		else{
			xml = xml->next;
		}
	}
	return counter;
}


/*
do mutation to segmentTemplate case
*/
Video_info mutate(Video_info vd){
	if (vd.media.substr(0, 18) == "$RepresentationID$"){
		int duration = stoi(vd.duration);
		for (int i = 0; i < duration + 1; i++){
			if (i == 0){
				string str = vd.id + "/" + vd.initialization.substr(19, vd.initialization.size());
				vd.URL.push_back(str);
			}
			else{
				size_t found = vd.media.find_first_of("0123456789");
				string sub = vd.media.substr(found, vd.media.size());
				size_t found1 = sub.find_first_not_of("0123456789");
				string digi = vd.media.substr(found, found1);
				int dig = stoi(digi);
				stringstream ss;
				ss << setfill('0') << setw(dig) << (i - 1);
				string s = ss.str();
				size_t found2 = vd.media.find_first_of(".");
				string type = vd.media.substr(found2, vd.media.size());
				string str = vd.id + "/" + s + type;
				vd.URL.push_back(str);
			}
		}
	}
	return vd;
}

Audio_info mutate1(Audio_info vd){
	if (vd.media.substr(0, 18) == "$RepresentationID$"){
		int duration = stoi(vd.duration);
		for (int i = 0; i < duration + 1; i++){
			if (i == 0){
				string str = vd.id + "/" + vd.initialization.substr(19, vd.initialization.size());
				vd.URL.push_back(str);
			}
			else{
				size_t found = vd.media.find_first_of("0123456789");
				string sub = vd.media.substr(found, vd.media.size());
				size_t found1 = sub.find_first_not_of("0123456789");
				string digi = vd.media.substr(found, found1);
				int dig = stoi(digi);
				stringstream ss;
				ss << setfill('0') << setw(dig) << (i - 1);
				string s = ss.str();
				size_t found2 = vd.media.find_first_of(".");
				string type = vd.media.substr(found2, vd.media.size());
				string str = vd.id + "/" + s + type;
				vd.URL.push_back(str);
			}
		}
	}
	return vd;
}


Video_info copy(Video_info sour){
	Video_info dest;
	dest.bandwidth = sour.bandwidth;
	dest.codec = sour.codec;
	dest.contentType = sour.contentType;
	dest.framerate = sour.framerate;
	dest.height = sour.height;
	dest.id = sour.id;
	dest.maxframerate = sour.maxframerate;
	dest.maxheight = sour.maxheight;
	dest.maxwidth = sour.maxwidth;
	dest.mimeType = sour.mimeType;
	dest.par = sour.par;
	dest.sar = sour.sar;
	dest.subsegmentAli = sour.subsegmentAli;
	dest.subsegmentStartwithSAP = sour.subsegmentStartwithSAP;
	dest.URL = sour.URL;
	dest.width = sour.width;
	return dest;
}

Audio_info copy_audio(Audio_info sour){
	Audio_info dest;
	dest.bandwidth = sour.bandwidth;
	dest.codec = sour.codec;
	dest.lang = sour.lang;
	dest.audiosamplerate = sour.audiosamplerate;
	dest.id = sour.id;
	dest.startwithSAP = sour.startwithSAP;
	dest.subsegmentAli = sour.subsegmentAli;
	dest.subsegmentStartwithSAP = sour.subsegmentStartwithSAP;
	dest.URL = sour.URL;
	dest.mimeType = sour.mimeType;
	return dest;
}


int MPD_parser::mpdparser_libxml2(string path, string* fname, string mpdpath){
	/*
	* this initialize the library and check potential ABI mismatches
	* between the version it was compiled for and the actual shared
	* library used.
	*/
	LIBXML_TEST_VERSION

		xmlDocPtr doc; /* the resulting document tree */

	doc = xmlReadFile(path.c_str(), NULL, 0);
	if (doc == NULL) {
		fprintf(stderr, "Failed to parse %s\n", path.c_str());
		return -1;
	}



	/*
	Saving BaseURL into baseURL field in class MPD_parser
	*/
	_xmlNode* child = doc->children->children;
	_xmlNode child2 = * child;
	_xmlNode* child1 = str(child, "BaseURL");
	masterURL = eql(child1, mpdpath);
	* child = child2;
	
	child1 = str(child, "Period");
	while (child1->children != NULL){
		/*
		Saving Audio
		*/

		//string audio = eql((xmlChar *)childAudio);
		if (this_one(child1->children, "AdaptationSet") && type(str(child1->children, "AdaptationSet"), "mimeType").substr(0, 5) == "audio"){
			vector<Audio_info> ado;
			_xmlNode* childAudio = str(child1->children, "AdaptationSet");
			_xmlNode childAudio0 = *childAudio;
			int number = counter(childAudio);
			*childAudio = childAudio0;
			Audio_info ad;
			_xmlNode childAudio1 = *childAudio;
			if (exist(childAudio, "mimeType")){
				ad.mimeType = type(childAudio, "mimeType");
			}
			*childAudio = childAudio1;
			if (exist(childAudio, "codecs")){
				ad.codec = type(childAudio, "codecs");
			}
			if (exist(childAudio, "lang")){
				ad.lang = type(childAudio, "lang");
			}
			*childAudio = childAudio1;
			if (exist(childAudio, "subsegmentAlignment")){
				ad.subsegmentAli = type(childAudio, "subsegmentAlignment");
			}
			*childAudio = childAudio1;
			if (exist(childAudio, "subsegmentStartsWithSAP")){
				ad.subsegmentStartwithSAP = type(childAudio, "subsegmentStartsWithSAP");
			}
			*childAudio = childAudio1;
			if (exist(childAudio, "id")){
				ad.id = type(childAudio, "id");
			}
			*childAudio = childAudio1;
			if (exist(childAudio, "bandwidth")){
				ad.bandwidth = type(childAudio, "bandwidth");
			}
			*childAudio = childAudio1;
			if (exist(childAudio, "audioSamplingRate")){
				ad.audiosamplerate = type(childAudio, "audioSamplingRate");
			}
			*childAudio = childAudio1;
			if (exist(childAudio, "startWithSAP")){
				ad.startwithSAP = type(childAudio, "startWithSAP");
			}
			*childAudio = childAudio1;
			Audio_info ad2;
			if (member(childAudio, "SegmentTemplate")){
				*childAudio = childAudio1;
				_xmlNode * childAudio4 = str(childAudio->children, "SegmentTemplate");
				_xmlNode childAudio5 = *childAudio4;
				if (exist(childAudio4, "startNumber")) {
					ad2.startNumber = type(childAudio4, "startNumber");
				}
				*childAudio4 = childAudio5;
				if (exist(childAudio4, "timescale")) {
					ad2.timescale = type(childAudio4, "timescale");
				}
				*childAudio4 = childAudio5;
				if (exist(childAudio4, "duration")) {
					ad2.duration = type(childAudio4, "duration");
				}
				*childAudio4 = childAudio5;
				if (exist(childAudio4, "media")) {
					ad2.media = type(childAudio4, "media");
				}
				*childAudio4 = childAudio5;
				if (exist(childAudio4, "initialization")) {
					ad2.initialization = type(childAudio4, "initialization");
				}
				*childAudio4 = childAudio5;
			}
			*childAudio = childAudio1;
			for (int i = 0; i < number; i++){
				Audio_info ad1;
				ad1 = copy_audio(ad);
				_xmlNode * childAudio3 = str(childAudio->children, "Representation");
				_xmlNode  childAudio2 = *childAudio3;
				ad1.startNumber = ad2.startNumber;
				ad1.timescale = ad2.timescale;
				ad1.duration = ad2.duration;
				ad1.media = ad2.media;
				ad1.initialization = ad2.initialization;
				if (exist(childAudio3, "mimeType")) {
					ad1.mimeType = type(childAudio3, "mimeType");
				}
				*childAudio3 = childAudio2;

				if (exist(childAudio3, "codecs")) {
					ad1.codec = type(childAudio3, "codecs");
				}
				*childAudio3 = childAudio2;

				if (exist(childAudio3, "lang")) {
					ad1.lang = type(childAudio3, "lang");
				}
				*childAudio3 = childAudio2;

				if (exist(childAudio3, "subsegmentAlignment")) {
					ad1.subsegmentAli = type(childAudio3, "subsegmentAlignment");
				}
				*childAudio3 = childAudio2;

				if (exist(childAudio3, "subsegmentStartsWithSAP")) {
					ad1.subsegmentStartwithSAP = type(childAudio3, "subsegmentStartsWithSAP");
				}
				*childAudio3 = childAudio2;

				if (exist(childAudio3, "id")) {
					ad1.id = type(childAudio3, "id");
				}
				*childAudio3 = childAudio2;

				if (exist(childAudio3, "bandwidth")) {
					ad1.bandwidth = type(childAudio3, "bandwidth");
				}
				*childAudio3 = childAudio2;

				if (exist(childAudio3, "audioSamplingRate")) {
					ad1.audiosamplerate = type(childAudio3, "audioSamplingRate");
				}
				*childAudio3 = childAudio2;

				if (exist(childAudio3, "startwithSAP")) {
					ad1.startwithSAP = type(childAudio3, "startWithSAP");
				}
				*childAudio3 = childAudio2;
				
				_xmlNode * childAudio6 = str(childAudio3->children, "BaseURL");
				_xmlNode childAudio7 = childAudio1;
				if (member(&childAudio7, "SegmentTemplate")){
					ad1 = mutate1(ad1);
				}
				else{
					ad1.URL.push_back(eql1(childAudio6, mpdpath));
				}
				ado.push_back(ad1);
				childAudio->children = childAudio->children->next->next;
			}
			int time = times(childAudio0.children);
			for (int i = 0; i < ado.size(); i++){
				if (i > ado.size()) break;
				Audio_info vec = ado[i];
				if (i <= (time - ado.size()) && i > 0){
					ado.erase(ado.begin() + i);
					i--;
				}
			}
			audio.insert(audio.end(), ado.begin(), ado.end());
			child1->children = child1->children->next;
		}

		/*
		Save Video
		*/
		else if (this_one(child1->children, "AdaptationSet") && type(str(child1->children, "AdaptationSet"), "mimeType").substr(0, 5) == "video"){
			vector<Video_info> vdo;
			_xmlNode * childVideo = str(child1->children, "AdaptationSet");
			_xmlNode childVideo0 = *childVideo;
			int number = counter(childVideo);
			*childVideo = childVideo0;
			Video_info vd;
			_xmlNode childVideo1 = *childVideo;
			if (exist(childVideo, "mimeType")){
				vd.mimeType = type(childVideo, "mimeType");
			}
			*childVideo = childVideo1;
			if (exist(childVideo, "codecs")){
				vd.codec = type(childVideo, "codecs");
			}
			*childVideo = childVideo1;
			if (exist(childVideo, "contentType")){
				vd.contentType = type(childVideo, "contentType");
			}
			*childVideo = childVideo1;
			if (exist(childVideo, "subsegmentAlignment")){
				vd.subsegmentAli = type(childVideo, "subsegmentAlignment");
			}
			*childVideo = childVideo1;
			if (exist(childVideo, "subsegmentStartsWithSAP")){
				vd.subsegmentStartwithSAP = type(childVideo, "subsegmentStartsWithSAP");
			}
			*childVideo = childVideo1;
			if (exist(childVideo, "id")){
				vd.id = type(childVideo, "id");
			}
			*childVideo = childVideo1;
			if (exist(childVideo, "bandwidth")){
				vd.id = type(childVideo, "bandwidth");
			}
			*childVideo = childVideo1;
			if (exist(childVideo, "width")){
				vd.id = type(childVideo, "width");
			}
			*childVideo = childVideo1;
			if (exist(childVideo, "height")){
				vd.id = type(childVideo, "height");
			}
			*childVideo = childVideo1;
			if (exist(childVideo, "sar")){
				vd.id = type(childVideo, "sar");
			}
			*childVideo = childVideo1;
			if (exist(childVideo, "framerate")){
				vd.id = type(childVideo, "framerate");
			}
			*childVideo = childVideo1;
			if (exist(childVideo, "maxwidth")){
				vd.id = type(childVideo, "maxwidth");
			}
			*childVideo = childVideo1;
			if (exist(childVideo, "maxheight")){
				vd.id = type(childVideo, "maxheight");
			}
			*childVideo = childVideo1;
			if (exist(childVideo, "maxframerate")){
				vd.id = type(childVideo, "maxframerate");
			}
			*childVideo = childVideo1;
			if (exist(childVideo, "par")){
				vd.id = type(childVideo, "par");
			}
			*childVideo = childVideo1;
			Video_info vd2;

			if (member(childVideo, "SegmentTemplate")){
				*childVideo = childVideo1;
				_xmlNode *childVideo4 = str(childVideo->children, "SegmentTemplate");
				_xmlNode childVideo5 = *childVideo4;
				if (exist(childVideo4, "startNumber")) {
					vd2.startNumber = type(childVideo4, "startNumber");
				}
				*childVideo4 = childVideo5;
				if (exist(childVideo4, "timescale")) {
					vd2.timescale = type(childVideo4, "timescale");
				}
				*childVideo4 = childVideo5;
				if (exist(childVideo4, "duration")) {
					vd2.duration = type(childVideo4, "duration");
				}
				*childVideo4 = childVideo5;
				if (exist(childVideo4, "media")) {
					vd2.media = type(childVideo4, "media");
				}
				*childVideo4 = childVideo5;
				if (exist(childVideo4, "initialization")) {
					vd2.initialization = type(childVideo4, "initialization");
				}
				*childVideo4 = childVideo5;
			}
			*childVideo = childVideo1;
			for (int i = 0; i < number; i++){
				Video_info vd1;
				vd1 = copy(vd);
				_xmlNode * childVideo3 = str(childVideo->children, "Representation");
				_xmlNode  childVideo2 = *childVideo3;
				vd1.startNumber = vd2.startNumber;
				vd1.timescale = vd2.timescale;
				vd1.media = vd2.media;
				vd1.duration = vd2.duration;
				vd1.initialization = vd2.initialization;
				if (exist(childVideo3, "mimeType")){
					vd1.mimeType = type(childVideo, "mimeType");
				}
				*childVideo3 = childVideo2;
				if (exist(childVideo3, "codecs")){
					vd1.codec = type(childVideo3, "codecs");
				}
				*childVideo3 = childVideo2;
				if (exist(childVideo3, "contentType")){
					vd1.contentType = type(childVideo3, "contentType");
				}
				*childVideo3 = childVideo2;
				if (exist(childVideo3, "subsegmentAlignment")){
					vd1.subsegmentAli = type(childVideo3, "subsegmentAlignment");
				}
				*childVideo3 = childVideo2;
				if (exist(childVideo3, "subsegmentStartsWithSAP")){
					vd1.subsegmentStartwithSAP = type(childVideo3, "subsegmentStartsWithSAP");
				}
				*childVideo3 = childVideo2;
				if (exist(childVideo3, "id")){
					vd1.id = type(childVideo3, "id");
				}
				*childVideo3 = childVideo2;
				if (exist(childVideo3, "bandwidth")){
					vd1.bandwidth = type(childVideo3, "bandwidth");
				}
				*childVideo3 = childVideo2;
				if (exist(childVideo3, "width")){
					vd1.width = type(childVideo3, "width");
				}
				*childVideo3 = childVideo2;
				if (exist(childVideo3, "height")){
					vd1.height = type(childVideo3, "height");
				}
				*childVideo3 = childVideo2;
				if (exist(childVideo3, "sar")){
					vd1.sar = type(childVideo3, "sar");
				}
				*childVideo3 = childVideo2;
				if (exist(childVideo3, "frameRate")){
					vd1.framerate = type(childVideo3, "frameRate");
				}
				*childVideo3 = childVideo2;
				if (exist(childVideo3, "maxwidth")){
					vd1.maxwidth = type(childVideo3, "maxwidth");
				}
				*childVideo3 = childVideo2;
				if (exist(childVideo3, "maxheight")){
					vd1.maxheight = type(childVideo3, "maxheight");
				}
				*childVideo3 = childVideo2;
				if (exist(childVideo3, "maxframerate")){
					vd1.maxframerate = type(childVideo3, "maxframerate");
				}
				*childVideo3 = childVideo2;
				if (exist(childVideo3, "par")){
					vd1.par = type(childVideo3, "par");
				}
				
				_xmlNode * childVideo6 = str(childVideo3->children, "BaseURL");
				_xmlNode childVideo7 = childVideo1;
				if (member(&childVideo7, "SegmentTemplate")){
					vd1 = mutate(vd1);
				}
				else{
					vd1.URL.push_back(eql1(childVideo6, mpdpath));
				}
				vdo.push_back(vd1);
				childVideo->children = childVideo->children->next->next;
			}
			int time = times(childVideo0.children);
			for (int i = 0; i < vdo.size(); i++){
				if (i > vdo.size()) break;
				Video_info vec = vdo[i];
				if (i <= (time - vdo.size()) && i > 0){
					vdo.erase(vdo.begin() + i);
					i--;
				}
			}
			video.insert(video.end(), vdo.begin(), vdo.end());
			child1->children = child1->children->next;
		}
		else{
			child1->children = child1->children->next;
		}
	}

	xmlFreeDoc(doc);

	/*
	* Cleanup function for the XML library.
	*/
	xmlCleanupParser();
	/*
	* this is to debug memory for regression tests
	*/
	xmlMemoryDump();
	return (0);
}


int MPD_parser::mpdparser(string path, string* fname){
	string addr = path;
	string act_line;
	string act_rep = "";
	string act_rep1 = "";
	ifstream infile;
	infile.open(addr.c_str(), ifstream::in);
	if (infile.is_open()){
		while (infile.good()){
			getline(infile, act_line);
			act_line.erase(act_line.begin(), find_if(act_line.begin(), act_line.end(), not1(ptr_fun<int, int>(isspace))));
			if (act_line.substr(0, 13) == "<BaseURL>http"){
				size_t found = act_line.find_last_of("<");
				act_rep += act_line.substr(9, found - 9);
			}
			else if ((act_line.substr(0, 19) == "<Representation id=" &&
				act_line.substr(23, 9) == "mimeType=") && act_line.substr(33, 5) == "video"){
				getline(infile, act_line);
				act_line.erase(act_line.begin(), find_if(act_line.begin(), act_line.end(), not1(ptr_fun<int, int>(isspace))));
				size_t found = act_line.find_last_of("<");
				act_rep1 += act_line.substr(9, found - 9);
				break;
			}//<Representation> case
			else if (act_line.substr(0, 24) == "<AdaptationSet mimeType=" &&
				act_line.substr(25, 5) == "video"){
				getline(infile, act_line);
				getline(infile, act_line);
				act_line.erase(act_line.begin(), find_if(act_line.begin(), act_line.end(), not1(ptr_fun<int, int>(isspace))));
				size_t found = act_line.find_last_of("<");
				act_rep1 += act_line.substr(9, found - 9);
				break;
			}//<AdaptationSet> case
		}
		infile.close();
		act_rep += act_rep1;
		//act_rep += master_link;
		cout << act_rep << endl;
	}
	char* array = (char*)act_rep.c_str();
	*fname = act_rep;
	return 0;
}
   
#endif                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 