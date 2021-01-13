#include <iostream>
#include <string>
#include <fstream>
#include <Windows.h>
#include <WinINet.h>
#include <vector>
#include <tuple>
#include <set>
#include "mz/ystring.h"
#include "parser.hpp"
#include "range.hpp"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "mz/zstd_helper.h"
//#include "mz/brotli_helper.h"

using namespace std;
using namespace std::mz;
using namespace aria::csv;
using namespace rapidjson;

// Next: An executable, which turns all csv-downloads into a single JSON file


/*

Sources: Probably, its best to just download csv's of all food. This can be done on this website:
https://ndb.nal.usda.gov/ndb/foods/show/18064?fgcd=&manu=&format=&count=&max=25&offset=&sort=default&order=asc&qlookup=bread&ds=SR&qt=&qp=&qa=&qn=&q=&ing=

More specifically:
https://ndb.nal.usda.gov/ndb/foods/show/18064?format=Abridged&reportfmt=csv&Qv=1

The IDs can be grabbed from the "FOOD_DES.txt" file in that SR-download. Overall, there are just 7793 entries, so a simple script should easily be able to grab all files from the website.
These csv-files seem to have a non-regular format, but perhaps the csv-reader will be able to handle it anyway.
The next conversion/processing steps have yet to be determined, but ultimately, it should converge towards something
*/

#pragma comment(lib, "WinINet.lib")

vector<tuple<string, string>> ExtractIDsAndNames() {
  vector<tuple<string, string>> vID;

  ifstream fileIDs(string(PROJECT_SOURCE_DIR)+"/data/FOOD_DES/FOOD_DES.txt");
  string str;
  while (getline(fileIDs, str)) {
    vID.push_back(mt(str.substr(1, 5), str.substr(16, str.find("~", 16)-16)));
  }
  return vID;
}

void DownloadSRAbridged(vector<string> vID) {
  for (auto&xID:vID) {
    yprintf("% ", xID);
    ofstream fout(string(PROJECT_SOURCE_DIR)+"/data/SR Abridged/"+xID+".csv");

    string url = ystr("https://ndb.nal.usda.gov/ndb/foods/show/%?format=Abridged&reportfmt=csv&Qv=1", xID);
    HINTERNET hopen = InternetOpen("MyAppName", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (hopen) {
      DWORD flags = INTERNET_FLAG_DONT_CACHE;
      if (url.find("https://")==0) flags |= INTERNET_FLAG_SECURE;
      HINTERNET hinternet = InternetOpenUrl(hopen, url.c_str(), NULL, 0, flags, 0);
      if (hinternet) {
        char buf[1024];
        DWORD received = 0;
        while (InternetReadFile(hinternet, buf, sizeof(buf), &received)) {
          if (!received) break;
          fout.write(buf, received);
        }
        InternetCloseHandle(hinternet);
      }
      InternetCloseHandle(hopen);
    }
    Sleep(1000);
  }
}

int ConvertCSVToJSON(vector<tuple<string, string>> vID) {
  StringBuffer os;
  Writer<StringBuffer> Writer(os);
  set<string> Keys;
  set<string> Units;

  Writer.StartObject();
  yprintf("Start...");
  for (auto& xID:vID) {
    ifstream fin(string(PROJECT_SOURCE_DIR)+"/data/SR Abridged/"+gt(xID, 0)+".csv");
    CsvParser parser(fin);
    bool Started = false;
    Writer.Key(gt(xID, 0).c_str());
    Writer.StartObject();
    Writer.Key("Name");
    Writer.String(gt(xID, 1).c_str());
    Writer.Key("Parts");
    Writer.StartObject();
    for (auto&row:parser) {
      if (row[0][0]!='(') if (row.size()>2) {
        if (Started) {
          Writer.Key(row[0].c_str());
          Keys.insert(row[0]);
          Writer.StartArray();
          Writer.String(row[1].c_str());
          Units.insert(row[1]);
          Writer.Double(stod(row[2]));
          Writer.EndArray();
        } else Started = true;
      }
    }
    Writer.EndObject();
    Writer.EndObject();
  }
  Writer.EndObject();
  string CompressedJSON;
  ZstdOfBuffer(CompressedJSON, os.GetString(), os.GetSize(), 20);
  ofstream fpBrot(string(PROJECT_SOURCE_DIR)+"/data/SR Abridged.zstd", ios_base::out+ios_base::binary);
  fpBrot<<CompressedJSON;

  yprintf("Finished!\n");

  yprintf("Original: %\nCompressed: %\n\nList of parts:\n", os.GetSize(), CompressedJSON.size());
  for (auto &x:Keys) yprintf("%\n",x);

  yprintf("\nList of units:\n");
  for (auto &x:Units) yprintf("%\n", x);

  return 0;
}

int main() {
  auto vID = ExtractIDsAndNames();
  //DownloadSRAbridged(vID);

  if (ConvertCSVToJSON(vID)!=0) return 1;
  system("pause");

  return 0;
}