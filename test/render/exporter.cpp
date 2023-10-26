#include "loader.hpp"
#include "Utility/ConfigManager.hpp"
#include "Layer/Scene.hpp"

#include "VGG/Exporter/ImageExporter.hpp"
#include "VGG/Exporter/SVGExporter.hpp"
#include "VGG/Exporter/PDFExporter.hpp"
#include "VGG/Exporter/Type.hpp"

#include <argparse/argparse.hpp>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
using namespace VGG;

template<typename InputIterator, typename F>
void write(InputIterator iter, F&& f, const std::string& ext = ".png")
{
  std::string key;
  std::vector<char> image;
  auto ok = iter.next(key, image);
  while (ok)
  {
    std::ofstream ofs(f(key) + ext, std::ios::binary);
    if (ofs.is_open())
    {
      ofs.write((const char*)image.data(), image.size());
    }
    ok = iter.next(key, image);
  }
}

void writeDoc(const std::string& extension,
              const fs::path& prefix,
              const std::string& outputFilePostfix,
              nlohmann::json design,
              nlohmann::json layout,
              Resource res)
{
  if (extension == ".pdf")
  {
    auto iter = exporter::pdf::PDFIterator(std::move(design), std::move(layout), std::move(res));
    write(
      std::move(iter),
      [&](auto guid) { return (prefix / (guid + outputFilePostfix)).string(); },
      extension);
  }
  else
  {
    auto iter = exporter::svg::SVGIterator(std::move(design), std::move(layout), std::move(res));
    write(
      std::move(iter),
      [&](auto guid) { return (prefix / (guid + outputFilePostfix)).string(); },
      extension);
  }
}

struct InputDesc
{
  fs::path filepath;
  int imageQuality{ 80 };
  int resolutionLevel{ 2 };
  std::optional<fs::path> configFilePath;
  std::optional<fs::path> prefix;
};

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
  program.add_argument("-q", "--quality")
    .help("image quality [0(low),100(high)]")
    .scan<'i', int>()
    .default_value(80);
  program.add_argument("-o", "--output").help("output directory");
  program.add_argument("-f", "--file-format").help("imageformat: png, jpg, webp, svg, pdf");
  program.add_argument("-t").help("postfix for output filename");

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
  bool isBitmap = true;
  std::string extension = ".png";
  if (auto v = program.present("-f"))
  {
    auto ext = v.value();
    if (ext == "png")
    {
      opts.type = VGG::exporter::PNG;
      extension = ".png";
      isBitmap = true;
    }
    else if (ext == "jpg")
    {
      opts.type = VGG::exporter::JPEG;
      extension = ".jpg";
      isBitmap = true;
    }
    else if (ext == "webp")
    {
      opts.type = VGG::exporter::WEBP;
      extension = ".webp";
      isBitmap = true;
    }
    else if (ext == "pdf")
    {
      extension = ".pdf";
      isBitmap = false;
    }
    else if (ext == "svg")
    {
      extension = ".svg";
      isBitmap = false;
    }
    else
    {
      isBitmap = true;
      extension = ".png";
      std::cout << "unsupported format: use png as default\n";
    }
  }
  opts.resolutionLevel = 2;
  opts.imageQuality = 80;
  int s = program.get<int>("-q");
  opts.imageQuality = s;

  if (auto cfg = program.present("-c"))
  {
    exporter::setGlobalConfig(cfg.value());
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
      const fs::path prefix = outputDir;
      fs::create_directory(prefix);
      const auto folder = fs::path(fp).filename().stem();
      if (isBitmap)
      {
        auto iter = exporter.render(data.Format, nlohmann::json{}, data.Resource, opts);
        write(
          std::move(iter),
          [&](auto guid) { return (prefix / (guid + outputFilePostfix)).string(); },
          extension);
      }
      else
      {
        writeDoc(extension,
                 prefix,
                 outputFilePostfix,
                 data.Format,
                 nlohmann::json{},
                 data.Resource);
      }
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
            const fs::path prefix = outputDir;
            fs::create_directory(prefix);
            if (isBitmap)
            {
              auto iter = exporter.render(data.Format, nlohmann::json{}, data.Resource, opts);
              write(
                std::move(iter),
                [&](auto guid) { return (prefix / (guid + outputFilePostfix)).string(); },
                extension);
            }
            else
            {
              writeDoc(extension,
                       prefix,
                       outputFilePostfix,
                       data.Format,
                       nlohmann::json{},
                       data.Resource);
            }
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
