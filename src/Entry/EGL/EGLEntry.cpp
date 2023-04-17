#include <argparse/argparse.hpp>
#include <fstream>
#include "../../Utils/Version.hpp"
#include "../../Utils/FileManager.hpp"
#include "EGLRuntime.h"

using namespace VGG;

int main(int argc, char** argv)
{
  argparse::ArgumentParser program("vgg", Version::get());
    program.add_argument("-l", "--load").help("load from vgg or sketch file");
    program.add_argument("-w", "--width").help("width of output image").scan<'i', int>().default_value(1200);
    program.add_argument("-h", "--height").help("height of output image").scan<'i', int>().default_value(800);
    program.add_argument("-s", "--sketch").help("input sketch file");
    program.add_argument("-o", "--output").help("output image file").default_value(std::string("image.png"));
  try
  {
    program.parse_args(argc, argv);
  }
  catch (const std::exception & err)
  {
    std::cout << err.what() << std::endl;
    std::cout << program << std::endl;
    exit(0);
  }

  if (auto loadfile = program.present("-s"))
  {
    auto fp = loadfile.value();
    if(!FileManager::loadFile(fp)){
      WARN("Failed to load sketch file");
    }
  }

  auto w = program.get<int>("-w");
  auto h = program.get<int>("-h");

  if(init(w,h)!=0){
    FAIL("Failed to create app");
    return 0;
  }
  Image out;
  out.data = nullptr;

  ImageInfo info;
  info.format = PNG;
  info.quality = 100;
  info.height = w;
  info.width = h;

  if(renderAsImage(0, &info, &out) == 0){
    std::ofstream ofs(program.get("-o"), std::ios::binary);
    if(ofs.is_open()){
      ofs.write((const char *)out.data,out.size);
      INFO("write to file ... %d byte(s)", out.size);
      return 0;
    }
    FAIL("failed to open file");
  }else{
    FAIL("filed to render image");
  }
  releaseImage(&out);
  return 0;
}