//Twittervibe v0.0.0.0.0.0.2
//Very litte by qDot - http://www.slashdong.org
//Driver code taken from http://dqd.com/%7Emayoff/tools/trance-vibrator.c
//CURL/XPath ideas/code taken from libCURL and libXML tutorials
//Basically, very little of this is mine. I just smacked a bunch of
//shit together and took it to a bar (where I fixed a few things)
//But it's still awesome
//Switch TWITTER_NAME and TWITTER_PASS with your name/pass
//If you want to use the public timeline instead of friends, 
//set PUBLIC_TIMELINE equal to TRUE, otherwise FALSE
//compile line:
//g++ -o tv -I/usr/include/curl -I/usr/include/libxml2 \
// -lusb -lcurl -lxml2 twittervibe.cpp
//
//History
//v0.0.0.0.0.0.1 - First version
//v0.0.0.0.0.0.2 - Fixed while drinking at Side Bar in Austin edition 
//   - Fixed a couple of crashes, set infinite loop



#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <cerrno>
#include <usb.h>
#include <curl.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

using namespace std;

const string TWITTER_NAME = "NOT";
const string TWITTER_PASS = "LIKELY";
const bool USE_PUBLIC_TIMELINE = false;

string twitter_string;
string twitter_parsed[100][2];
struct usb_bus* bus = 0;
struct usb_device* dev = 0;
struct usb_dev_handle* h;
 
size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  twitter_string.append((char*)ptr);
  return size * nmemb;
}

int xpathParse(std::string xmlString, std::string xmlSearch)
{
  xmlDocPtr doc;
  doc = xmlParseMemory(xmlString.c_str(), xmlString.length());
  if(!doc)
    {
      cout << "ohshit no xml" << endl;
    }
  xmlXPathContextPtr context;
  xmlXPathObjectPtr result;
  xmlNodeSetPtr nodeset;
  xmlChar *keyword;
  xmlChar *xpath = (xmlChar*) xmlSearch.c_str();
  context = xmlXPathNewContext(doc);
  if (context == NULL) {
    printf("Error in xmlXPathNewContext\n");
    return 0;
  }
  result = xmlXPathEvalExpression(xpath, context);
  xmlXPathFreeContext(context);
  if (result == NULL) {
    printf("Error in xmlXPathEvalExpression\n");
    return 0;
  }
  if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
    xmlXPathFreeObject(result);
    printf("No result\n");
    return 0;
  }
    int j = 0;
  if (result) {
    nodeset = result->nodesetval;

    for (int i=0; i < nodeset->nodeNr; ++i) {
      keyword = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode, 1);
      if(!(i%2))
	{
	  twitter_parsed[j][1] = (char*)keyword;
	}
      else
	{
	  twitter_parsed[j][0] = (char*)keyword;
	  ++j;
	}
      xmlFree(keyword);
    }
    xmlXPathFreeObject (result);
  }
  xmlFreeDoc(doc);
  xmlCleanupParser();
  return j;
}

int runVibe(string pair[2])
{
  cout << "Running vibe from " << pair[0] << endl;
  cout << "Input: " << pair[1] << endl;
  for(int i = 0; i < pair[1].length(); ++i)
    {
      int speed;
      
      // The vibrator is flaky, so try the request a few times...
      for (int j = 0; j < 3; ++j) {
	if (usb_control_msg(h, 
	    USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_INTERFACE,
	    1, (pair[1])[i], 0, 0, 0, 0) < 0)
	  {
	    fprintf(stderr,
		    strerror(errno));
	  }
	else break;
      }

      usleep(100000);
    }
}

int
main(int argc, char** argv)
{
    usb_init();
    usb_find_busses();
    usb_find_devices();

    for (bus = usb_get_busses(); bus != 0; bus = bus->next) {
	for (dev = bus->devices; dev != 0; dev = dev->next) {
	    if (dev->descriptor.idVendor == 0x0b49
		&& dev->descriptor.idProduct == 0x064f
	    )
		goto done;
	}
    }

done:
    if (dev == 0) {
	fprintf(stderr, "error: no trance vibrator found\n");
	return 1;
    }

    h = usb_open(dev);
    if (h == 0) {
	fprintf(stderr, "error: could not open device\n");
	return 1;
    }	      

    //open curl streamCURL *curl;
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
      twitter_string = "";
      string twitter_url = "http://";
      twitter_url.append(TWITTER_NAME);
      twitter_url.append(":");
      twitter_url.append(TWITTER_PASS);
      if(USE_PUBLIC_TIMELINE)
	{
	  twitter_url.append("@twitter.com/statuses/public_timeline.xml");
	}
      else
	{
	  twitter_url.append("@twitter.com/statuses/friends_timeline.xml");
	}
      curl_easy_setopt(curl, CURLOPT_URL, twitter_url.c_str());
      curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

    while(1)
      {
    if(curl) {
      twitter_string = "";
      res = curl_easy_perform(curl);
      int parseCount = xpathParse(twitter_string, "//name | //text");
      --parseCount;
      while( parseCount >= 0)
	{
	  if(twitter_parsed[parseCount][0] == "") break;
	  runVibe(twitter_parsed[parseCount]);
	  --parseCount;
	}
      /* always cleanup */
      
    }

      }
curl_easy_cleanup(curl); 
    return 0;
}

