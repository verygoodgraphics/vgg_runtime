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
  program.add_argument("-s", "--scale").help("canvas scale").scan<'g', float>().default_value(1.0);
  program.add_argument("-c", "--config").help("specify config file");
  program.add_argument("-q", "--quality").help("canvas scale").scan<'i', int>().default_value(80);

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
  std::filesystem::path prefix;
  std::filesystem::path respath;
  if (auto configfile = program.present("-c"))
  {
    auto file = configfile.value();
    Config::readGlobalConfig(file);
  }

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

    auto r = load("." + ext);
    if (r)
    {
      auto data = r->read(prefix / fp);
      auto result = render(data.Format, data.Resource, 80, 2);
      auto reason = std::get<0>(result);
      std::cout << "Reason: " << reason << std::endl;
      writeResult(std::get<1>(result));
    }
  }

  return 0;
}
