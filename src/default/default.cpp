#include "parser.hpp"
#include "mz/ystring.h"
#include "mz/basics.h"
//#include "mz/brotli_helper.h"
#include "mz/zstd_helper.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include <string>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <tuple>
#include <set>
#include <fstream>
#include <unordered_map> 
#include <array>
#include <sstream>
#include "range.hpp"
#include "imguiWindow_dx11.h"

using namespace std;
using namespace std::mz;
using namespace rapidjson;

struct TPartDesc {
  string LongName;
  string ShortName;
  bool isActive;
  string Unit;
  float Daily;
};

array<TPartDesc, 34> Parts{
  TPartDesc{"Weight","Weight",true,"(g)"},
  TPartDesc{"Energy","Energy",true,"(kcal)",2000},
  TPartDesc{"Carbohydrate, by difference","Carbs",true,"(g)",300},
  TPartDesc{"Protein","Protein",true,"(mg)",50},
  TPartDesc{"Total lipid (fat)","fat",true,"(g)",65},
  TPartDesc{"Fatty acids, total monounsaturated","FatMono",true,"(g)"},
  TPartDesc{"Fatty acids, total polyunsaturated","FatPoly",true,"(g)"},
  TPartDesc{"Fatty acids, total saturated","FatSat",true,"(g)",20},
  TPartDesc{"Fatty acids, total trans","FatTrans",false,"(g)"},
  TPartDesc{"Caffeine","Caffeine",false,"(mg)"},
  TPartDesc{"Calcium, Ca","Calcium",false,"(mg)",1000},
  TPartDesc{"Cholesterol","Cholesterol",true,"(mg)",300},
  TPartDesc{"Fiber, total dietary","Fiber",true,"(g)",25},
  TPartDesc{"Folate, DFE","Folate",false,"(ug)",400},
  TPartDesc{"Iron, Fe","Fe",false,"(mg)",18},
  TPartDesc{"Magnesium, Mg","(mg)",false,"(mg)",400},
  TPartDesc{"Niacin","Niacin",false,"(mg)",20},
  TPartDesc{"Phosphorus, P","P",false,"(mg)"},
  TPartDesc{"Potassium, K","K",false,"(g)",3500},
  TPartDesc{"Riboflavin","Riboflavin",false,"(mg)",1.7f},
  TPartDesc{"Sodium, Na","Na",false,"(mg)",2400},
  TPartDesc{"Sugars, total","Sugar",true,"(g)"},
  TPartDesc{"Thiamin","Thiamin",false,"(mg)",1.5f},
  TPartDesc{"Vitamin A, IU","VitAiu",true,"(IU)",5000},
  TPartDesc{"Vitamin A, RAE","VitArae",true,"(ug)"},
  TPartDesc{"Vitamin B-12","VitB12",false,"(ug)",6.0f},
  TPartDesc{"Vitamin B-6","VitB6",false,"(ug)",2.0f},
  TPartDesc{"Vitamin C, total ascorbic acid","VitC",true,"(mg)",60},
  TPartDesc{"Vitamin D","VitD",false,"(IU)",400},
  TPartDesc{"Vitamin D (D2 + D3)","VitD23",false,"(ug)"},
  TPartDesc{"Vitamin E (alpha-tocopherol)","VitE",false,"(mg)",30},
  TPartDesc{"Vitamin K (phylloquinone)","VitK",false,"(ug)",80},
  TPartDesc{"Water","Water",true,"(g)"},
  TPartDesc{"Zinc, Zn","Zn",false,"(g)",15},
};

unordered_map<string, size_t> PartNameOrder;   // Not fully implemented
unordered_map<string, TPartDesc> PartNameInfo;
string PartNames0 = "";

void InitPartNames() {
  for (size_t i : indices(Parts)) {
    PartNameOrder[Parts[i].LongName] = i;
    PartNameInfo[Parts[i].LongName] = Parts[i];
    PartNames0 += Parts[i].ShortName+" ";
    PartNames0.back() = '\0';
  }
}

struct TPart {
  float v;   // Part value
  float vT;  // Transformed value
  string vS; // Value as string
};

struct TFood {
  string NameU; // Name in uppercase
  string Name;
  string ID;
  bool isMarked = false;
  array<TPart, 34> Parts;
};

vector<TFood> vFood;
vector<ui32> vFilterID;

Document document;

char SearchString[256] = "";
bool FirstExec = true;
int idxRef = 0;
float vRef = 100;
bool ScaleToDaily = false;
float FoodNameWidth = 400;

// Es fehlt noch:
// - Optional: Farbliche Hervorhebung von hohen+niedrigen Werten
// - Option, nach Spalten zu sortieren
// - Optional: Speichern/Persistenz
// - Optional: Zeilenhervorhebung, damit man auch bei sehr breiten Zeilen nicht den Ueberblick verliert

void ApplyFoodScaling() {
  for (auto &x:vFood) {
    float factor = vRef/(float)x.Parts[idxRef].v;
    if (ScaleToDaily) {
      for (auto i:range<size_t>(1,x.Parts.size())) {
        if (Parts[i].Daily!=0) {
          x.Parts[i].vT = factor*x.Parts[i].v/Parts[i].Daily*100;
          snprintf(&x.Parts[i].vS[0], 30, "%.4g%%", x.Parts[i].vT);
        } else {
          x.Parts[i].vT = -1;
          x.Parts[i].vS = "-";
        }
      }
    } else {
      for (auto i:range<size_t>(1, x.Parts.size())) {
        x.Parts[i].vT = factor*x.Parts[i].v;
        snprintf(&x.Parts[i].vS[0], 30, "%.4g", x.Parts[i].vT);
      }
    }
    snprintf(&x.Parts[0].vS[0], 30, "%.4g", factor*x.Parts[0].v);
  }
}

// Split string, and make it uppercase
vector<string> SplitStr(string strInput) {
  for (auto&x:strInput) x = (char)toupper(x);
  stringstream ssInput(strInput);
  vector<string> tokens; // Create vector to hold our words
  string buf;
  while (ssInput>>buf) tokens.push_back(buf);
  return tokens;
}

void FilterFood() {
  vFilterID.clear();
  auto vSearches = SplitStr(SearchString);
  for (auto i:indices(vFood)) if (vFood[i].isMarked) vFilterID.push_back((ui32)i);
  if (vSearches.size()==0) {
    for (auto i:indices(vFood)) if (!vFood[i].isMarked) vFilterID.push_back((ui32)i);
  } else {
    for (auto i:indices(vFood)) if (!vFood[i].isMarked) {
      for (auto&x:vSearches) {
        if (vFood[i].NameU.find(x)==string::npos) break;
        if (x==vSearches.back())vFilterID.push_back((ui32)i);
      }
    }
  }
}



void DoMainWindow(TImGuiWindowDX11& ImGuiWindow) {
  auto locWindow = ImGuiWindow.GetWindowLocation();
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2((float)locWindow.xSz, (float)locWindow.ySz));
  if (locWindow.xSz<=0) return;

  ImGui::Begin("Invisible title", nullptr, ImGuiWindowFlags_NoTitleBar+ImGuiWindowFlags_NoResize+ImGuiWindowFlags_NoMove+ImGuiWindowFlags_NoSavedSettings);
  {
    ImGui::Columns(max(1, locWindow.xSz/150), "mycolumns", false);
    int nActiveParts = 0;
    for (auto&x:Parts) {
      ImGui::Checkbox(x.ShortName.c_str(), &x.isActive);
      ImGui::NextColumn();
      if (x.isActive) nActiveParts++;
    }
    ImGui::Columns(1);

    ImGui::Separator();
    if (ImGui::Combo("Reference choice", &idxRef, PartNames0.c_str(), 6)) ApplyFoodScaling();
    if (ImGui::InputFloat("Reference value", &vRef)) ApplyFoodScaling();
    if (ImGui::InputText("Filter Text", SearchString, IM_ARRAYSIZE(SearchString))) FilterFood();
    if (ImGui::Checkbox("Daily intake", &ScaleToDaily)) ApplyFoodScaling();
    ImGui::SameLine();
    ImGui::SliderFloat("Food name width", &FoodNameWidth, 0, 1200);

    ImGui::Separator();
    ImGui::Columns(nActiveParts+1);
    ImGui::SetColumnWidth(0, FoodNameWidth);
    float szXtot = ImGui::GetWindowWidth();
    float szX0 = ImGui::GetColumnWidth(0);
    float szXitem = (szXtot-szX0-32)/nActiveParts;
    for (int i : range(1, nActiveParts)) ImGui::SetColumnWidth(i, szXitem);
    ImGui::Text("Food name");
    for (auto j:range(34)) if (Parts[j].isActive) {
      ImGui::NextColumn();
      ImGui::TextUnformatted(Parts[j].ShortName.c_str());
      ImGui::TextUnformatted(Parts[j].Unit.c_str());
    }

    ImGui::Columns(1);
    ImGui::BeginChild("##ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::Columns(nActiveParts+1);
    ImGui::SetColumnWidth(0, FoodNameWidth);
    for (int i : range(1, nActiveParts)) ImGui::SetColumnWidth(i, szXitem);

    auto ITEMS_COUNT = vFilterID.size();
    ImGuiListClipper clipper((int)ITEMS_COUNT);
    while (clipper.Step()) {
      for (int iID = clipper.DisplayStart; iID<clipper.DisplayEnd; iID++) {
        auto i = vFilterID[iID];
        ImGui::Checkbox(vFood[i].Name.c_str(), &vFood[i].isMarked);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", vFood[i].Name.c_str());
        ImGui::NextColumn();
        for (auto j:range(34)) if (Parts[j].isActive) {
          ImGui::TextUnformatted(vFood[i].Parts[j].vS.c_str());
          ImGui::NextColumn();
        }
      }
    }
    ImGui::Columns(1);
    ImGui::EndChild();
  }
  ImGui::End();
  FirstExec = false;
}

#undef GetObject

void ReadData() {
  string Data, JSONData;
  yprintf("Reading data...\n");
  StringOfFile(Data, string(PROJECT_SOURCE_DIR)+"/data/SR Abridged.zstd");
  yprintf("Decompressing data...\n");
  StringOfZstd(JSONData, Data);
  yprintf("Parsing data...\n");
  document.Parse(JSONData.c_str());
}

void MakeFood() {
  yprintf("Preparing food...\n");
  vFood.reserve(document.GetObject().MemberCount());
  for (auto& x:document.GetObject()) {
    TFood foo;
    for (auto &y:foo.Parts) y.v = 0;
    foo.ID = x.name.GetString();
    foo.NameU = foo.Name = x.value.GetObject()["Name"].GetString();
    for (auto&y:foo.NameU) y = (char)toupper(y);
    for (auto& y:x.value.GetObject()["Parts"].GetObject()) {
      float v = (float)y.value.GetArray()[1].GetDouble();
      foo.Parts[PartNameOrder[y.name.GetString()]] = {v,v,"                              "};
    }
    foo.Parts[0] = {100,100,"100"};
    vFood.push_back(foo);
  }
  FilterFood();
  ApplyFoodScaling();
}

void MainWindow() {
  yprintf("Opening window...\n");
  TImGuiWindowDX11 ImDx;
  ImDx.Init("nutrition", 1.5f, 100, 50, 1601, 800);
  while (!ImDx.ProcessMessagesAndCheckIfQuit(false)) {
    ImDx.NewFrame();

    DoMainWindow(ImDx);

    ImDx.Render(true);
  }
}

int main() {
  InitPartNames();
  ReadData();
  MakeFood();
  MainWindow();
}

