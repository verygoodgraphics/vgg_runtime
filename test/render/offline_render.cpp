#include <argparse/argparse.hpp>
#include <filesystem>
#include <fstream>
#include <optional>
#include "ConfigMananger.h"
#include "Scene/Scene.h"
#include "loader.h"
#include "Entry/EGL/EGLRuntime.h"
using namespace VGG;

template<typename F>
void writeResult(const std::vector<std::pair<std::string, std::vector<char>>>& result, F&& f)
{
  int count = 0;
  for (const auto p : result)
  {
    std::ofstream ofs(f(p.first) + ".png", std::ios::binary);
    if (ofs.is_open())
    {
      ofs.write((const char*)p.second.data(), p.second.size());
    }
  }
}

struct InputDesc
{
  fs::path filepath;
  std::string fontCollection;
  int imageQuality{ 80 };
  int resolutionLevel{ 2 };
  std::optional<fs::path> configFilePath;
  std::optional<fs::path> prefix;
};

std::vector<std::pair<std::string, std::vector<char>>> renderAndOutput(InputDesc input)
{
  auto ext = fs::path(input.filepath).extension().string();
  auto r = load(ext);
  if (r)
  {
    auto data = r->read(input.prefix.value_or(fs::path(".")) / input.filepath);
    auto result = VGGNew::render(data.Format,
                                 data.Resource,
                                 input.imageQuality,
                                 input.resolutionLevel,
                                 input.configFilePath.value_or("config.json"),
                                 input.fontCollection);
    const auto reason = std::get<0>(result);
    if (!reason.empty())
    {
      std::cout << reason << std::endl;
    }
    return std::get<1>(result);
  }
  return {};
}

int main(int argc, char** argv)
{
  argparse::ArgumentParser program("exporter", "0.1");
  program.add_argument("-l", "--load").help("load from vgg or sketch file");
  program.add_argument("-d", "--data").help("resources dir");
  program.add_argument("-p", "--prefix").help("the prefix of filename or dir");
  program.add_argument("-L", "--loaddir").help("iterates all the files in the given dir");
  program.add_argument("-s", "--scale").help("canvas scale").scan<'g', float>().default_value(1.0);
  program.add_argument("-c", "--config").help("specify config file");
  program.add_argument("-q", "--quality").help("canvas scale").scan<'i', int>().default_value(80);
  program.add_argument("-o", "--output").help("output directory");
  program.add_argument("-t").help("postfix for output filename");
  program.add_argument("-f", "--font-collection")
    .help("Specifiy font collection listed in config file");

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

  const auto outputDir = program.present("-o").value_or(".");
  if (!fs::exists(outputDir))
  {
    std::cout << "specified output directory not exists\n";
  }

  const fs::path configFilePath = program.present("-c").value_or("config.json");
  std::string outputFilePostfix = program.present("-t").value_or("");
  InputDesc desc;
  desc.prefix = program.present("-p").value_or("");
  desc.fontCollection = program.present("-f").value_or("google");
  desc.resolutionLevel = 2;
  desc.imageQuality = 80;
  desc.configFilePath = configFilePath;

  Config::readGlobalConfig(configFilePath);

  if (auto loadfile = program.present("-l"))
  {
    auto fp = loadfile.value();
    desc.filepath = fp;
    auto ext = fs::path(fp).extension().string();
    auto r = load(ext);
    if (r)
    {
      auto res = renderAndOutput(desc);
      const auto folder = fs::path(fp).filename().stem();
      const fs::path prefix = outputDir;
      fs::create_directory(prefix);
      writeResult(res, [&](auto guid) { return (prefix / (guid + outputFilePostfix)).string(); });
    }
  }
  else if (auto d = program.present("-L"))
  {
    const auto dir = desc.prefix.value_or(".") / d.value();
    if (fs::exists(dir))
    {
      for (const auto& ent : fs::recursive_directory_iterator(dir))
      {
        if (fs::is_regular_file(ent))
        {
          desc.filepath = ent;
          auto res = renderAndOutput(desc);
          const auto folder = ent.path().filename().stem();
          const fs::path prefix = outputDir;
          fs::create_directory(prefix);
          writeResult(res,
                      [&](auto guid) { return (prefix / (guid + outputFilePostfix)).string(); });
        }
      }
    }
    else
    {
      std::cout << "Directory " << dir << "not exists\n";
    }
  }
  return 0;
}
