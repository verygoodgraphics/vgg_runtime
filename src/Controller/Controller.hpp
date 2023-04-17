#pragma once

#include <string>

class VggWork;
class VggExec;

class Controller
{
public:
  enum RunMode
  {
    NormalMode,
    EditMode
  };

  Controller(RunMode mode = NormalMode);
  ~Controller() = default;

  bool start(const std::string& filePath);
  bool start(const std::vector<unsigned char>& buffer);

  void onClick(const std::string& path);

private:
  RunMode m_mode;
  std::shared_ptr<VggWork> m_work;

  const std::shared_ptr<VggExec>& vggExec();
};
