#include <argparse/argparse.hpp>
#include <fstream>
#include "../../Utils/Version.hpp"
#include "../../Utils/FileManager.hpp"
#include "EGLRuntime.h"

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

int main(int argc, char** argv)
{
  argparse::ArgumentParser program("vgg", Version::get());
  program.add_argument("-l", "--load").help("load from vgg or sketch file");
  program.add_argument("-w", "--width")
    .help("width of output image")
    .scan<'i', int>()
    .default_value(800);
  program.add_argument("-h", "--height")
    .help("height of output image")
    .scan<'i', int>()
    .default_value(1200);
  program.add_argument("-s", "--sketch").help("input sketch file");
  program.add_argument("-o", "--output")
    .help("output image file")
    .default_value(std::string("image.png"));
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

    auto w = program.get<int>("-w");
    auto h = program.get<int>("-h");
    std::map<std::string, std::vector<char>> resources;
    std::string json = GetTextFromFile(fp);
    auto j = nlohmann::json::parse(json);
    auto res = render(j, resources, 80);
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
