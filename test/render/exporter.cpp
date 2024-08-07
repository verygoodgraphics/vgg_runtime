#include "Layer/Core/ResourceManager.hpp"
#include "Layer/Core/DefaultResourceProvider.hpp"
#include "Layer/GlobalSettings.hpp"
#include "loader.hpp"
#include "Utility/ConfigManager.hpp"
#include "Utility/Log.hpp"

#include "VGG/Exporter/ImageExporter.hpp"
#include "VGG/Exporter/SVGExporter.hpp"
#include "VGG/Exporter/PDFExporter.hpp"
#include "VGG/Exporter/Type.hpp"

#include <argparse/argparse.hpp>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <ranges>
using namespace VGG;

template<typename InputIterator, typename F>
void write(InputIterator iter, F&& f, const std::string& ext = ".png")
{
  auto res = iter.next();
  while (res)
  {
    INFO("Render Time Cost: [%f]", res.timeCost->render);
    INFO("Encode Time Cost: [%f]", res.timeCost->encode);
    std::ofstream ofs(f(res.data->first) + ext, std::ios::binary);
    if (ofs.is_open())
    {
      ofs.write((const char*)res.data->second.data(), res.data->second.size());
    }
    res = iter.next();
  }
}

void writeDoc(
  const std::string&            extension,
  const fs::path&               prefix,
  const std::string&            outputFilePostfix,
  nlohmann::json                design,
  nlohmann::json                layout,
  const exporter::ExportOption& opts)
{
  exporter::BuilderResult buildResult;
  if (extension == ".pdf")
  {
    auto iter = exporter::PDFIterator(std::move(design), std::move(layout), opts, buildResult);
    if (buildResult.type)
    {
      DEBUG("Version mismatch: ");
    }
    INFO("Expand Time Cost: [%f]", buildResult.timeCost->expand);
    INFO("Layout Time Cost: [%f]", buildResult.timeCost->layout);
    write(
      std::move(iter),
      [&](auto guid) { return (prefix / (guid + outputFilePostfix)).string(); },
      extension);
  }
  else
  {
    auto iter = exporter::SVGIterator(std::move(design), std::move(layout), opts, buildResult);
    INFO("Expand Time Cost: [%f]", buildResult.timeCost->expand);
    INFO("Layout Time Cost: [%f]", buildResult.timeCost->layout);
    write(
      std::move(iter),
      [&](auto guid) { return (prefix / (guid + outputFilePostfix)).string(); },
      extension);
  }
}

struct InputDesc
{
  fs::path                filepath;
  int                     imageQuality{ 80 };
  int                     resolutionLevel{ 2 };
  std::optional<fs::path> configFilePath;
  std::optional<fs::path> prefix;
};

constexpr char POS_ARG_INPUT_FILE[] = "fig/ai/sketch/json";

int main(int argc, char** argv)
{
  layer::setupEnv();
  argparse::ArgumentParser program("exporter", "0.1");
  program.add_argument(POS_ARG_INPUT_FILE).help("input fig/ai/sketch/json file").default_value("");
  program.add_argument("-d", "--data").help("resources dir");
  program.add_argument("-p", "--prefix").help("the prefix of filename or dir");
  program.add_argument("-L", "--loaddir").help("iterates all the files in the given dir");
  program.add_argument("-s", "--scale").help("canvas scale").scan<'g', float>().default_value(1.0f);
  program.add_argument("-w", "--width").help("canvas width").scan<'g', float>();
  program.add_argument("-h", "--height").help("canvas height").scan<'g', float>();
  program.add_argument("-l", "--level").help("canvas level").scan<'i', int>();
  program.add_argument("-c", "--config").help("specify config file");
  program.add_argument("-r", "--repeat")
    .help("repeat rendering (DEBUG)")
    .scan<'i', int>()
    .default_value(1);
  program.add_argument("-q", "--quality")
    .help("image quality [0(low),100(high)]")
    .scan<'i', int>()
    .default_value(80);
  program.add_argument("-o", "--output").help("output directory");
  program.add_argument("-f", "--file-format").help("imageformat: png, jpg, webp, svg, pdf");
  program.add_argument("-t").help("postfix for output filename");
  program.add_argument("--disable-layout").help("disable layout").implicit_value(true);
  program.add_argument("--disable-expand")
    .help("disable replace for symbol instance")
    .implicit_value(true);

  program.add_argument("--repl").help("run as REPL mode").implicit_value(true);

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
  InputDesc   desc;
  desc.prefix = program.present("-p").value_or("");

  exporter::ImageOption opts;
  bool                  isBitmap = true;
  std::string           extension = ".png";
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
  opts.imageQuality = 80;
  // priority of size: width > height > level > scale
  if (auto w = program.present<float>("-w"))
  {
    opts.size = exporter::ImageOption::WidthDetermine{ *w };
  }
  else if (auto h = program.present<float>("-h"))
  {
    opts.size = exporter::ImageOption::HeightDetermine{ *h };
  }
  else if (auto l = program.present<int>("-l"))
  {
    opts.size = exporter::ImageOption::LevelDetermine{ *l };
  }
  else
  {
    opts.size = exporter::ImageOption::ScaleDetermine{ program.get<float>("-s") };
  }
  int s = program.get<int>("-q");
  opts.imageQuality = s;

  if (auto cfg = program.present("-c"))
  {
    exporter::setGlobalConfig(cfg.value());
  }

  exporter::ExportOption exportOpt;
  if (auto cfg = program.present<bool>("--disable-expand"))
    exportOpt.enableExpand = false;

  if (auto cfg = program.present<bool>("--disable-layout"))
    exportOpt.enableLayout = false;

  exporter::ExporterInfo info;
  exporter::Exporter     exporter;
  exporter.info(&info);
  INFO("%s", info.graphicsInfo.c_str());
  auto loadfile = program.get<std::string>(POS_ARG_INPUT_FILE);
  if (!loadfile.empty())
  {
    auto fp = loadfile;
    desc.filepath = fp;
    auto ext = fs::path(fp).extension().string();
    auto r = load(ext);
    if (r)
    {
      auto data = r->read(desc.prefix.value_or(fs::path(".")) / desc.filepath);
      layer::setGlobalResourceProvider(std::move(data.provider));
      const fs::path prefix = outputDir;
      fs::create_directory(prefix);
      const auto folder = fs::path(fp).filename().stem();
      if (isBitmap)
      {
        for (auto i = 0; i < program.get<int>("-r"); i++)
        {
          exporter::BuilderResult res;
          auto iter = exporter.render(data.format, data.layout, opts, exportOpt, res);
          if (res.type)
          {
            DEBUG("Version mismatch");
          }
          INFO("Expand Time Cost: [%f]", res.timeCost->expand);
          INFO("Layout Time Cost: [%f]", res.timeCost->layout);

          write(
            std::move(iter),
            [&](auto guid) { return (prefix / (guid + outputFilePostfix)).string(); },
            extension);
        }
      }
      else
      {
        writeDoc(extension, prefix, outputFilePostfix, data.format, nlohmann::json{}, exportOpt);
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
            layer::setGlobalResourceProvider(std::move(data.provider));
            const fs::path prefix = outputDir;
            fs::create_directory(prefix);
            if (isBitmap)
            {
              exporter::BuilderResult res;
              auto iter = exporter.render(data.format, nlohmann::json{}, opts, exportOpt, res);
              std::cout << "Expand Time Cost: " << res.timeCost->expand << std::endl;
              std::cout << "Layout Time Cost: " << res.timeCost->layout << std::endl;
              write(
                std::move(iter),
                [&](auto guid) { return (prefix / (guid + outputFilePostfix)).string(); },
                extension);
            }
            else
            {
              writeDoc(
                extension,
                prefix,
                outputFilePostfix,
                data.format,
                nlohmann::json{},
                exportOpt);
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
  else if (auto repl = program.present<bool>("--repl"))
  {
    const char* prompt = "exporter(input, output)> ";

    std::string line;
    while (std::getline(std::cin, line))
    {
      std::cout << prompt;
      if (line == "exit")
        return 0;

      auto view =
        line | std::views::split(',') |
        std::views::transform(
          [](auto&& rng) { return std::string_view(&*rng.begin(), std::ranges::distance(rng)); });

      // have not trimmed the string, becareful the space before and after the comma
      std::vector<std::string_view> args(view.begin(), view.end());
      if (args.empty())
      {
        std::cout << prompt;
        continue;
      }
      std::cout << "Input: " << args[0] << ", Output: " << args[1] << std::endl;
      fs::path filename = args[0];
      fs::path output = args.size() > 1 ? args[1] : ".";
      fs::path ext = filename.extension().string();
      auto     r = load(ext);
      if (r)
      {
        auto data = r->read(filename);
        layer::setGlobalResourceProvider(std::move(data.provider));
        auto rp = static_cast<layer::FileResourceProvider*>(
          layer::getGlobalResourceProvider()); // we assume it's a file resource provider
        rp->setDirPrefix(filename.parent_path());
        fs::create_directory(output);
        if (isBitmap)
        {
          exporter::BuilderResult res;
          auto iter = exporter.render(data.format, data.layout, opts, exportOpt, res);
          INFO("Expand Time Cost: [%f]", res.timeCost->expand);
          INFO("Layout Time Cost: [%f]", res.timeCost->layout);
          write(
            std::move(iter),
            [&](auto guid) { return (output / guid).string(); },
            extension);
        }
        else
        {
          writeDoc(
            extension,
            fs::path{},
            outputFilePostfix,
            data.format,
            nlohmann::json{},
            exportOpt);
        }
      }
      else
      {
        std::cout << "Failed to load file: " << filename << std::endl;
      }
    }
  }
  return 0;
}
