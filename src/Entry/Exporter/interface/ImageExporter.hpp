#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string>
#include <tuple>
#include <map>
#include <nlohmann/json.hpp>

namespace VGG::exporter
{

struct ImageOption
{
  int imageQuality = 100;
  int resolutionLevel = 2;
};

enum class EBackend
{
  VULKAN,
};
struct BackendInfo
{
  EBackend type;
  int majorVersion;
  int minorVersion;
};

struct ExporterInfo
{
  std::string buildCommit;
  BackendInfo backend;
};

using OutputCallback = std::function<bool(const std::string&, const std::vector<char>&)>;
using Resource = std::map<std::string, std::vector<char>>;

class Exporter__pImpl;
class Exporter
{
  std::unique_ptr<Exporter__pImpl> d_impl; // NOLINT
public:
  class Iterator__pImpl;
  class Iterator
  {
    std::unique_ptr<Iterator__pImpl> d_impl; // NOLINT
    Iterator(Exporter& exporter, nlohmann::json json, Resource resource, const ImageOption& opt);
    friend class Exporter;

  public:
    bool next(std::string& key, std::vector<char>& image);
    Iterator(Iterator&& other) noexcept;
    Iterator& operator=(Iterator&& other) noexcept;
    ~Iterator();
  };

  Exporter();
  void info(ExporterInfo* info);

  Iterator render(nlohmann::json j,
                  std::map<std::string, std::vector<char>> resources,
                  const ImageOption& opt)
  {
    return Exporter::Iterator{ *this, std::move(j), std::move(resources), opt };
  }
  void setOutputCallback(OutputCallback callback);
  ~Exporter();
};

void setGlobalConfig(const std::string& fileName);

}; // namespace VGG::exporter
