#include <argparse/argparse.hpp>
#include <filesystem>
#include <fstream>
#include "../../Utils/Version.hpp"
#include "../../Utils/FileManager.hpp"
#include "EGLRuntime.h"
#include <vgg_sketch_parser/src/analyze_sketch_file/analyze_sketch_file.h>

using namespace VGG;

std::string GetTextFromFile(const std::string& fileName)
{
  std::ifstream in(fileName, std::ios::in);
  if (in.is_open() == false)
  {
    exit(-1);
  }
  return std::string{ std::istreambuf_iterator<char>{ in }, std::istreambuf_iterator<char>{} };
}

std::vector<char> GetBinFromFile(const std::string& filename)
{
  std::ifstream in(filename, std::ios::binary);
  if (in.is_open() == false)
  {
    exit(-1);
  }
  return std::vector<char>{ std::istreambuf_iterator<char>{ in },
                            std::istreambuf_iterator<char>{} };
}

int main(int argc, char** argv)
{
  argparse::ArgumentParser program("vgg", Version::get());
  program.add_argument("-s", "--sketch").help("input sketch file");
  program.add_argument("-l").help("input vgg file");
  program.add_argument("-d").help("input resources file");
  try
  {
    program.parse_args(argc, argv);
  }
  catch (const std::exception& err)
  {
    std::cout << err.what() << std::endl;
    std::cout << program << std::endl;
    exit(0);
  }

  if (auto loadfile = program.present("-s"))
  {
    auto fp = loadfile.value();
    auto size = std::filesystem::file_size(fp);
    std::vector<char> file_buf(size);
    std::ifstream ifs(fp, std::ios_base::binary);
    if (ifs.is_open() == false)
    {
      exit(1);
    }
    ifs.read(file_buf.data(), size);
    assert(ifs.gcount() == size);
    nlohmann::json json_out;
    std::map<std::string, std::vector<char>> resources;
    analyze_sketch_file::analyze(file_buf.data(), size, "hello-sketch", json_out, resources);
    auto res = render(json_out, resources, 80);
    auto reason = std::get<0>(res);
    std::cout << "Reason: " << std::endl;
    int count = 0;
    for (const auto p : std::get<1>(res))
    {
      count++;
      std::stringstream ss;
      ss << "image" << count << ".png";
      std::string name;
      ss >> name;

      std::ofstream ofs(name, std::ios::binary);
      if (ofs.is_open())
      {
        ofs.write((const char*)p.second->bytes(), p.second->size());
      }
    }
  }

  if (auto loadfile = program.present("-l"))
  {
    auto fp = loadfile.value();
    auto json = GetTextFromFile(fp);
    nlohmann::json json_out = json::parse(json);
    std::map<std::string, std::vector<char>> resources;
    if (auto datafile = program.present("-d"))
    {
      for (const auto& entry : std::filesystem::recursive_directory_iterator(datafile.value()))
      {
        std::string key = string("./image/") + entry.path().filename().string();
        std::cout << "read image: " << entry.path() << " which key is " << key << std::endl;
        resources[key] = GetBinFromFile(entry.path());
      }
    }
    auto res = render(json_out, resources, 80);
    auto reason = std::get<0>(res);
    std::cout << "Reason: " << std::endl;
    int count = 0;
    for (const auto p : std::get<1>(res))
    {
      count++;
      std::stringstream ss;
      ss << "image" << count << ".png";
      std::string name;
      ss >> name;

      std::ofstream ofs(name, std::ios::binary);
      if (ofs.is_open())
      {
        ofs.write((const char*)p.second->bytes(), p.second->size());
      }
    }
  }
  return 0;
}
