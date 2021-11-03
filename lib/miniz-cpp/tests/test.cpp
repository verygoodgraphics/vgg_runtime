#include <array>
#include <cassert>
#include <fstream>
#include <filesystem>

#include "test.h"
#include <zip_file.hpp>

namespace {

static const char *existing_file = "test.xlsx";
static const char *temp_file = "temp.zip";
static const char *expected_content_types_string = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\"><Default Extension=\"xml\" ContentType=\"application/xml\"/><Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/><Default Extension=\"jpeg\" ContentType=\"image/jpg\"/><Default Extension=\"png\" ContentType=\"image/png\"/><Default Extension=\"bmp\" ContentType=\"image/bmp\"/><Default Extension=\"gif\" ContentType=\"image/gif\"/><Default Extension=\"tif\" ContentType=\"image/tif\"/><Default Extension=\"pdf\" ContentType=\"application/pdf\"/><Default Extension=\"mov\" ContentType=\"application/movie\"/><Default Extension=\"vml\" ContentType=\"application/vnd.openxmlformats-officedocument.vmlDrawing\"/><Default Extension=\"xlsx\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet\"/><Override PartName=\"/docProps/core.xml\" ContentType=\"application/vnd.openxmlformats-package.core-properties+xml\"/><Override PartName=\"/docProps/app.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.extended-properties+xml\"/><Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/><Override PartName=\"/xl/sharedStrings.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml\"/><Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/><Override PartName=\"/xl/theme/theme1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.theme+xml\"/><Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/></Types>\n";
static const char *expected_atxt_string = "aaa\r\nbbb\r\nccc\n\n\n";
static const char *expected_printdir_string = "  Length      Date    Time    Name\n---------  ---------- -----   ----\n      587  07/31/2014 19:19   _rels/.rels\n      299  07/31/2014 19:19   docProps/core.xml\n      231  07/31/2014 19:19   docProps/app.xml\n      415  07/31/2014 19:19   xl/workbook.xml\n      697  07/31/2014 19:19   xl/_rels/workbook.xml.rels\n    26038  07/31/2014 19:19   xl/theme/theme1.xml\n      291  07/31/2014 19:19   xl/theme/_rels/theme1.xml.rels\n     6415  07/31/2014 19:19   xl/worksheets/sheet1.xml\n      223  07/31/2014 19:19   xl/sharedStrings.xml\n     3188  07/31/2014 19:19   xl/styles.xml\n   489200  07/31/2014 19:19   xl/media/image1.png\n     1736  07/31/2014 19:19   [Content_Types].xml\n---------                     -------\n   529320                     12 files\n";

void remove_temp_file()
{
    std::remove(temp_file);
}

bool files_equal(const std::string &a, const std::string &b)
{
    if(a == b)
    {
        return true;
    }

    std::ifstream stream_a(a, std::ios::binary), stream_b(a, std::ios::binary);

    while(stream_a && stream_b)
    {
        if(stream_a.get() != stream_b.get())
        {
            return false;
        }
    }

    return true;
}

void test_load_file()
{
    remove_temp_file();
    miniz_cpp::zip_file f(existing_file);
    f.save(temp_file);
    assert(files_equal(existing_file, temp_file));
    remove_temp_file();
}

void test_load_stream()
{
    remove_temp_file();
    {
        std::ifstream in_stream(existing_file, std::ios::binary);
        miniz_cpp::zip_file f(in_stream);
        std::ofstream out_stream(temp_file, std::ios::binary);
        f.save(out_stream);
    }
    assert(files_equal(existing_file, temp_file));
    remove_temp_file();
}

void test_load_bytes()
{
    remove_temp_file();

    miniz_cpp::zip_file f;
    std::vector<unsigned char> source_bytes, result_bytes;
    std::ifstream in_stream(existing_file, std::ios::binary);
    while(in_stream)
    {
        source_bytes.push_back(static_cast<unsigned char>(in_stream.get()));
    }
    f.load(source_bytes);
    f.save(temp_file);

    miniz_cpp::zip_file f2;
    f2.load(temp_file);
    result_bytes = std::vector<unsigned char>();
    f2.save(result_bytes);

    assert(source_bytes == result_bytes);

    remove_temp_file();
}

void test_reset()
{
    miniz_cpp::zip_file f(existing_file);

    assert(!f.namelist().empty());

    try
    {
        f.read("[Content_Types].xml");
    }
    catch(std::exception e)
    {
        assert(false);
    }

    f.reset();

    assert(f.namelist().empty());

    try
    {
        f.read("[Content_Types].xml");
        assert(false);
    }
    catch(std::exception e)
    {
    }
}

void test_getinfo()
{
    miniz_cpp::zip_file f(existing_file);
    miniz_cpp::zip_info info = f.getinfo("[Content_Types].xml");
    assert(info.filename == "[Content_Types].xml");
}

void test_infolist()
{
    miniz_cpp::zip_file f(existing_file);
    assert(f.infolist().size() == 12);
}

void test_namelist()
{
    miniz_cpp::zip_file f(existing_file);
    assert(f.namelist().size() == 12);
}

void test_open_by_name()
{
    miniz_cpp::zip_file f(existing_file);
    std::stringstream ss;
    ss << f.open("[Content_Types].xml").rdbuf();
    std::string result = ss.str();
    assert(result == expected_content_types_string);
}

void test_open_by_info()
{
    miniz_cpp::zip_file f(existing_file);
    std::stringstream ss;
    ss << f.open("[Content_Types].xml").rdbuf();
    std::string result = ss.str();
    assert(result == expected_content_types_string);
}

void test_extract_path()
{
    miniz_cpp::zip_file f(existing_file);

    auto tempdir = std::filesystem::temp_directory_path();
    f.extract("[Content_Types].xml", tempdir);

    std::string source = f.read("[Content_Types].xml");

    auto path = miniz_cpp::detail::join_path({ tempdir, "[Content_Types].xml" });
    std::ifstream ifs(path);
    std::string result((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    assert(source == result);
    std::filesystem::remove(path);
}

void test_extractall_path()
{
    miniz_cpp::zip_file f(existing_file);

    auto tempdir = std::filesystem::temp_directory_path();
    auto path = miniz_cpp::detail::join_path({ tempdir, f.get_filename() });
    std::filesystem::remove_all(path);
    assert(std::filesystem::create_directories(path));

    f.extractall(path);

    for (auto name : f.namelist())
    {
      std::string source = f.read(name);
      auto fp = miniz_cpp::detail::join_path({ path, name });
      std::ifstream ifs(fp);
      std::string result((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

      assert(source == result);
    }
    assert(std::filesystem::remove_all(path));
}

void test_printdir()
{
    miniz_cpp::zip_file f(existing_file);
    std::stringstream ss;
    f.printdir(ss);
    std::string printed = ss.str();
    assert(printed == expected_printdir_string);
}

void test_read()
{
    miniz_cpp::zip_file f(existing_file);
    assert(f.read("[Content_Types].xml") == expected_content_types_string);
    assert(f.read(f.getinfo("[Content_Types].xml")) == expected_content_types_string);
}

void test_testzip()
{
    miniz_cpp::zip_file f(existing_file);
    assert(f.testzip().first);
}

void test_write()
{
    remove_temp_file();

    miniz_cpp::zip_file f;
    f.write("a.txt");
    f.write("a.txt", "b.txt");
    f.save(temp_file);

    miniz_cpp::zip_file f2(temp_file);
    assert(f2.read("a.txt") == expected_atxt_string);
    assert(f2.read("b.txt") == expected_atxt_string);

    remove_temp_file();
}

void test_writestr()
{
    remove_temp_file();

    miniz_cpp::zip_file f;
    f.writestr("a.txt", "a\na");
    miniz_cpp::zip_info info;
    info.filename = "b.txt";
    f.writestr(info, "b\nb");
    f.save(temp_file);

    miniz_cpp::zip_file f2(temp_file);
    assert(f2.read("a.txt") == "a\na");
    assert(f2.read(f2.getinfo("b.txt")) == "b\nb");

    remove_temp_file();
}

void test_comment()
{
    remove_temp_file();

    miniz_cpp::zip_file f;
    f.comment = "comment";
    f.save(temp_file);

    miniz_cpp::zip_file f2(temp_file);
    assert(f2.comment == "comment");

    remove_temp_file();
}

void write_existing()
{
    std::ofstream stream(existing_file, std::ios::binary);
    std::array<char, test_xlsx_len> test_xlsx_chars = {{0}};
    std::copy(test_xlsx, test_xlsx + test_xlsx_len, test_xlsx_chars.begin());
    stream.write(test_xlsx_chars.data(), test_xlsx_chars.size());
    std::ofstream stream2("a.txt", std::ios::binary);
    stream2 << expected_atxt_string;
}

void remove_existing()
{
    std::remove(existing_file);
    std::remove("a.txt");
}

void test_zip()
{
    write_existing();
    test_load_file();
    test_load_stream();
    test_load_bytes();
    test_reset();
    test_getinfo();
    test_infolist();
    test_namelist();
    test_open_by_name();
    test_open_by_info();
    test_extract_path();
    test_extractall_path();
    test_printdir();
    test_read();
    test_testzip();
    test_write();
    test_writestr();
    test_comment();
    remove_existing();
}

} // namespace

int main()
{
    test_zip();
    std::cout << "all tests passed" << std::endl;
    return 0;
}
