#include <argparse/argparse.hpp>
#include <filesystem>
#include <fstream>
#include <optional>
#include <Utility/interface/ConfigMananger.h>
#include "Scene/Scene.h"
#include "loader.h"
#include "Entry/Exporter/interface/ImageExporter.hpp"
using namespace VGG;

// template<typename F>
// void writeResult(const std::vector<std::pair<std::string, std::vector<char>>>& result, F&& f)
// {
//   int count = 0;
//   for (const auto p : result)
//   {
//     std::ofstream ofs(f(p.first) + ".png", std::ios::binary);
//     if (ofs.is_open())
//     {
//       ofs.write((const char*)p.second.data(), p.second.size());
//     }
//   }
// }

template<typename F>
void write(exporter::Exporter::Iterator iter, F&& f)
{
  std::string key;
  std::vector<char> image;
  while (iter.next(key, image))
  {
    std::ofstream ofs(f(key) + ".png", std::ios::binary);
    if (ofs.is_open())
    {
      ofs.write((const char*)image.data(), image.size());
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

// std::vector<std::pair<std::string, std::vector<char>>> renderAndOutput(InputDesc input)
// {
//   auto ext = fs::path(input.filepath).extension().string();
//   auto r = load(ext);
//   if (r)
//   {
//     auto data = r->read(input.prefix.value_or(fs::path(".")) / input.filepath);
//     DEBUG("Image Quality: %d", input.imageQuality);
//     DEBUG("Resolution Level: %d", input.resolutionLevel);
//     auto result = VGG::exporter::render(data.Format,
//                                         data.Resource,
//                                         input.imageQuality,
//                                         input.resolutionLevel,
//                                         input.configFilePath.value_or(""),
//                                         input.fontCollection);
//     const auto reason = std::get<0>(result);
//     if (!reason.empty())
//     {
//       std::cout << reason << std::endl;
//     }
//     return std::get<1>(result);
//   }
//   return {};
// }

constexpr char POS_ARG_INPUT_FILE[] = "fig/ai/sketch/json";

int main(int argc, char** argv)
{
  argparse::ArgumentParser program("exporter", "0.1");
  program.add_argument(POS_ARG_INPUT_FILE).help("input fig/ai/sketch/json file");
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

  std::string outputFilePostfix = program.present("-t").value_or("");
  InputDesc desc;
  desc.prefix = program.present("-p").value_or("");

  exporter::ImageOption opts;
  opts.resolutionLevel = 2;
  opts.imageQuality = 80;
  // desc.fontCollection = program.present("-f").value_or("google");
  //  desc.resolutionLevel = 2;
  //  desc.imageQuality = 80;

  if (auto cfg = program.present("-c"))
  {
    // desc.configFilePath = cfg.value();
    exporter::setGlobalConfig(desc.configFilePath->string());
  }
  exporter::Exporter exporter;

  if (auto loadfile = program.present(POS_ARG_INPUT_FILE))
  {
    auto fp = loadfile.value();
    desc.filepath = fp;
    auto ext = fs::path(fp).extension().string();
    auto r = load(ext);
    if (r)
    {
      auto data = r->read(desc.prefix.value_or(fs::path(".")) / desc.filepath);
      auto iter = exporter.render(data.Format, data.Resource, opts);
      const auto folder = fs::path(fp).filename().stem();
      const fs::path prefix = outputDir;
      fs::create_directory(prefix);
      write(std::move(iter),
            [&](auto guid) { return (prefix / (guid + outputFilePostfix)).string(); });
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
          auto r = load(desc.filepath.extension().string());
          if (r)
          {
            auto data = r->read(desc.prefix.value_or(fs::path(".")) / desc.filepath);
            auto iter = exporter.render(data.Format, data.Resource, opts);
            const fs::path prefix = outputDir;
            fs::create_directory(prefix);
            write(std::move(iter),
                  [&](auto guid) { return (prefix / (guid + outputFilePostfix)).string(); });
          }
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
