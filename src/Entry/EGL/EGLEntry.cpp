#include <argparse/argparse.hpp>
#include <filesystem>
#include <fstream>
#include "../../Utils/Version.hpp"
#include "../../Utils/FileManager.hpp"
#include "EGLRuntime.h"
#include <Reader/LoadUtil.hpp>
using namespace VGG;

void writeResult(const std::map<int, std::vector<char>>& result)
{
  int count = 0;
  for (const auto p : result)
  {
    count++;
    std::stringstream ss;
    ss << "image" << count << ".png";
    std::string name;
    ss >> name;

    std::ofstream ofs(name, std::ios::binary);
    if (ofs.is_open())
    {
      ofs.write((const char*)p.second.data(), p.second.size());
    }
  }
}

int main(int argc, char** argv)
{
  argparse::ArgumentParser program("vgg", Version::get());
  program.add_argument("-l", "--load").help("load from vgg or sketch file");
  program.add_argument("-d", "--data").help("resources dir");
  program.add_argument("-p", "--prefix").help("the prefix of filename or dir");
  program.add_argument("-L", "--loaddir").help("iterates all the files in the given dir");

  try
  {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error& err)
  {
    std::cout << err.what() << std::endl;
    std::cout << program;
    exit(0);
  }

  auto scene = std::make_shared<Scene>();
  std::map<std::string, std::vector<char>> resources;
  std::filesystem::path prefix;
  std::filesystem::path respath;

  if (auto p = program.present("-p"))
  {
    prefix = p.value();
  }
  if (auto loadfile = program.present("-l"))
  {
    auto fp = loadfile.value();
    auto ext = FileManager::getLoweredFileExt(fp);
    if (ext == "json")
    {
      respath = std::filesystem::path(fp).stem(); // same with filename as default
      if (auto res = program.present("-d"))
      {
        respath = res.value();
      }
    }

    load(fp,
         respath,
         prefix,
         [&](const auto& json, auto res)
         {
           auto result = render(json, resources, 80);
           auto reason = std::get<0>(result);
           std::cout << "Reason: " << std::endl;
           writeResult(std::get<1>(result));
         });
  }

  return 0;
}
