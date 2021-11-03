/*
 * Copyright (C) 2021 Chaoya Li <harry75369@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __FILE_UTILS_HPP__
#define __FILE_UTILS_HPP__

#include <string>

namespace VGG
{

namespace FileUtils
{

static inline std::string getFileName(const std::string& fp)
{
  int b = fp.find_last_of("/");
  int e = fp.find_last_of(".");
  if (b == fp.npos)
  {
    b = -1;
  }
  if (e == fp.npos)
  {
    e = fp.size();
  }
  if (b + 1 < fp.size() && e >= 1 && b + 1 < e)
  {
    return fp.substr(b + 1, e - b - 1);
  }
  return fp;
}

static inline std::string getLoweredFileExt(const std::string& fp)
{
  int e = fp.find_last_of(".");
  if (e == fp.npos)
  {
    return fp;
  }
  std::string ext = fp.substr(e + 1, fp.size() - e);
  for (size_t i = 0; i < ext.size(); i++)
  {
    ext[i] = tolower(ext[i]);
  }
  return ext;
}

}; // namespace FileUtils

}; // namespace VGG

#endif // __FILE_UTILS_HPP__
