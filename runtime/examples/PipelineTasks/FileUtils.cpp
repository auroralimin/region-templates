/*
 * FileUtils.cpp
 *
 *  Created on: Sep 26, 2011
 *      Author: tcpan
 *
 */

#include "FileUtils.h"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <queue>
#include <sys/stat.h>



const int FileUtils::DIRECTORY = 1;
const int FileUtils::FILE = 2;
const char *FileUtils::DIR_SEPARATOR = "/";


FileUtils::FileUtils()
{
}

FileUtils::FileUtils(const std::string & _suffix)
{
	exts.push_back(_suffix);
}

FileUtils::FileUtils(const std::vector<std::string> _suffixes)
{
	exts.assign(_suffixes.begin(), _suffixes.end());
}

FileUtils::~FileUtils()
{
    exts.clear();
}

std::string FileUtils::getDir(const std::string& name) {
	size_t pos = name.rfind(FileUtils::DIR_SEPARATOR);
	if (pos == std::string::npos) return std::string("");
	else if (pos == 0) return std::string(FileUtils::DIR_SEPARATOR);
	else return name.substr(0, pos);
}
std::string FileUtils::getFile(const std::string& filename) {
	size_t pos = filename.rfind(FileUtils::DIR_SEPARATOR);
//	printf("getFile input is %s length, position found for / is %ld, substd::string from there is %s\n", filename.c_str(), pos, filename.substr(0, pos).c_str());
	if (pos == std::string::npos) return filename;
	else if (pos == (filename.length() - 1)) return std::string("");
	else return filename.substr(pos + 1);
}
std::string FileUtils::getExt(const std::string& filename) {
	std::string fn = getFile(filename);  // needed to avoid "." in directory part of name.
	size_t pos = fn.rfind('.');
//	printf("getEXT input is %s, position found for . is %ld, substd::string from there is %s\n", fn.c_str(), pos, fn.substr(pos+1).c_str());
	if (pos == std::string::npos) return std::string("");
	else if (pos == (fn.length() - 1)) return std::string("");
	else return fn.substr(pos + 1);
}

bool FileUtils::hasExt(const std::string &filename, const std::string &ext) {
	size_t len = ext.length();
	return filename.compare(filename.length() - len, len, ext) == 0;
}

bool FileUtils::inDir(const std::string &name, const std::string &dir) {
	return name.compare(0, dir.length(), dir) == 0;
}

/**
 * have to handle both names being directory, both names being files, one name being directory
 */
std::string FileUtils::getCommonDir(const std::string& name1, const std::string &name2) {
	int len = std::min(name1.length(), name2.length());
	if (len == 0) return std::string("");

	std::string sn = (name1.length() > name2.length() ? name2 : name1);
	std::string ln = (name1.length() > name2.length() ? name1 : name2);

	// compare character by character.
	// at the end we reach the first character that is not the same
	int i;
	for (i = 0; i < len; ) {
		if (sn[i] != ln[i]) break;
		++i;
	}
	// now set up the positions
	int p = i-1; // last match pos.
	if (p < 0 ) return std::string(""); // none matched.
	// else matched between 1 and len number of characters.

	// at this point, we have a common substd::string that could end with / (ok), or not.
	// if not then we are either in the middle of a partial name (middle of sn),
	//    or the short std::string is completely matched (without a trailing /)
	//		to determine, check the long std::string's first diff character to see if it's /
	//		if long std::string ended at i also, then whether the common is a directory or not
	//			is determined by the last char == /.  if yes then dir (ok), if not then file (ok)
	//      else
	//			if first diff is /, set pos to i to include the /
//	if (sn[pos] == FileUtils::DIR_SEPARATOR) // both have / definitely dir name
//	else // no /
//		if (sn.length == i)  // at sn end.  dir or not depend on next char in ln.
//			if (ln.length == i)  // neither have /.  filenames.
//			else  // check next char
//				if (ln[i] == FileUtils::DIR_SEPARATOR) pos = i;  // ln next is /  so sn is dir name
//				else // ln next is not /, so middle of a directory component.
//		else // middle of a directory component
	// in most cases pos stays put.  only in 1 path pos is updated.
	if (sn[p] != *(FileUtils::DIR_SEPARATOR) &&
			i == sn.length() &&
			i < ln.length() &&
			ln[i] == *(FileUtils::DIR_SEPARATOR)) p = i;
	// now get the substd::string (from ln)
	std::string dirname = ln.substr(0, p+1);

	// and run it through getDir
	return FileUtils::getDir(dirname);
}


/***  directories are assume to have a trailing directory separator
 * basically, use std::string replace.
 */
std::string FileUtils::replaceDir(const std::string& filename, const std::string& oldStr, const std::string& newStr) {
	if (filename.length() == 0) {
		printf("WARNING: filename is empty.\n");
		return std::string();
	}

	if (!inDir(filename, oldStr)) {
		printf("ERROR: dir %s is not in filename %s.  returning empty std::string\n", oldStr.c_str(), filename.c_str());
		return std::string("");
	}

	std::string output = filename;
	return output.replace(0, oldStr.length(), newStr);
}

std::string FileUtils::replaceExt(const std::string& filename, const std::string& oldExt, const std::string& newExt) {
	if (filename.length() == 0) {
		printf("WARNING: filename is empty.\n");
		return std::string();
	}

	if (!hasExt(filename, oldExt)) {
		printf("ERROR: filename %s does not have %s extension.  returning empty std::string\n", filename.c_str(), oldExt.c_str());
		return std::string();		
	}
	
	std::string output = filename;
	return output.replace(filename.length() - oldExt.length(), oldExt.length(), newExt);
}

std::string FileUtils::replaceExt(const std::string& filename, const std::string& newExt) {
	if (filename.length() == 0) {
		printf("WARNING: filename is empty.\n");
		return std::string();
	}

	std::string output = filename;
	for (std::vector<std::string>::iterator iter=exts.begin();
			iter != exts.end(); iter++) {
			if (hasExt(output, *iter)) {
      	return output.replace(filename.length() - (*iter).length(), (*iter).length(), newExt);
       }
   }
		printf("ERROR: filename %s does not match specified extensions.  returning empty std::string\n", filename.c_str());
		return std::string();		
}


bool FileUtils::traverseDirectory(const std::string &directory, std::vector<std::string> &list, int types, bool recursive) {
	if (types == 0) {
		printf("ERROR: no capture types defined for directory traversal\n");
		return false;  // not asking to capture anything.
	}
	if ((types & FileUtils::FILE) == 0 && (types & FileUtils::DIRECTORY) == 0) {
		printf("ERROR: unknown capture type %d for directory traversal\n", types);
		return false;
	}
	if (directory.length() == 0) {
		printf("ERROR: specified empty directory name for directory traversal\n");
		return false;
	}
	
	struct dirent *ent;
	std::string d, s;
	DIR *dir;
	int status;
	struct stat st_buf;

	std::queue<std::string> dirList;

	status = stat (directory.c_str(), &st_buf);
	if (status != 0) {
		printf("ERROR: unable to inspect directory %s\n", directory.c_str());
		return false;
	}
	if (S_ISDIR (st_buf.st_mode)) dirList.push(directory);


	while (!dirList.empty()) {
		d = dirList.front();
		dirList.pop();

		// open a directory
		dir=opendir(d.c_str());
		if (dir == NULL) {
			printf("ERROR: can't open directory %s for listing.\n", d.c_str());
			continue;
		}
		while((ent=readdir(dir)) != NULL) {
			// loop until the directory is traveled thru

			// push directory or filename to the list
			s.assign("");
			s.append(d);
			s.append(FileUtils::DIR_SEPARATOR);
			s.append(ent->d_name);


			status = stat (s.c_str(), &st_buf);

			if (S_ISDIR (st_buf.st_mode)) {
				if (strcmp(ent->d_name, ".") != 0 &&
						strcmp(ent->d_name, "..") != 0) {
					if (recursive) dirList.push(s);
					if ((types & FileUtils::DIRECTORY) != 0) list.push_back(s);
				}
			} else if (S_ISREG (st_buf.st_mode)) {
				// a file, add to full list.
				if ((types & FileUtils::FILE) != 0) {
					if (exts.empty()) {
						list.push_back(s);
					} else {
						// need to check
						for (std::vector<std::string>::iterator iter=exts.begin();
								iter != exts.end(); iter++) {
							if (hasExt(s, *iter)) {
								list.push_back(s);
								break;
							}
						}
					}
				}
			} else printf("ERROR: not regular file nor directory.  what is it? %s\n", s.c_str());
		}
		// close up
		closedir(dir);
	}
	return true;
}


void FileUtils::traverseDirectoryRecursive(const std::string & directory, std::vector<std::string> & fullList)
{
//	struct dirent *ent;
//	std::string d, s;
//	DIR *dir;
//	int status;
//	struct stat st_buf;
//
//	std::queue<std::string> dirList;
//
//	status = stat (directory.c_str(), &st_buf);
//	if (status != 0) {
//		printf("ERROR: unable to inspect directory %s\n", directory.c_str());
//		return;
//	}
//	if (S_ISDIR (st_buf.st_mode)) dirList.push(directory);
//
//
//	while (!dirList.empty()) {
//		d = dirList.front();
//		dirList.pop();
//
//		// open a directory
//		dir=opendir(d.c_str());
//		if (dir == NULL) {
//			printf("ERROR: can't open directory %s for listing.\n", d.c_str());
//			continue;
//		}
//		while((ent=readdir(dir)) != NULL) {
//			// loop until the directory is traveled thru
//
//			// push directory or filename to the list
//			std::stringstream fullname;
//
//			fullname << d << "/" << ent->d_name;
//			s = fullname.str();
//
//			status = stat (s.c_str(), &st_buf);
//
//			if (S_ISDIR (st_buf.st_mode)) {
//				if (strcmp(ent->d_name, ".") &&
//						strcmp(ent->d_name, "..")) {
//					dirList.push(s);
//				}
//			} else if (S_ISREG (st_buf.st_mode)) {
//				// a file, add to full list.
//				if (exts.empty()) {
//					fullList.push_back(s);
//				} else {
//					// need to check
//					for (std::vector<std::string>::iterator iter=exts.begin();
//							iter != exts.end(); iter++) {
//						if (hasExt(s, *iter)) {
//							fullList.push_back(s);
//							break;
//						}
//					}
//				}
//			} else printf("ERROR: not regular file nor directory.  what is it? %s\n", s.c_str());
//		}
//		// close up
//		closedir(dir);
//	}
	traverseDirectory(directory, fullList, FileUtils::FILE, true);
}

void FileUtils::getFilesInDirectory(const std::string & directory, std::vector<std::string> & fileList)
{
//    struct dirent *ent;
//    std::string d, s;
//	DIR *dir;
//	int status;
//	struct stat st_buf;
//
//	status = stat (directory.c_str(), &st_buf);
//	if (status != 0) {
//		printf("ERROR: unable to inspect directory %s\n", directory.c_str());
//		return;
//	}
//	if (!(S_ISDIR (st_buf.st_mode))) return;
//
//
//	d = directory;
//
//	// open a directory
//    	dir=opendir(d.c_str());
//    	if (dir == NULL) {
//    		printf("ERROR: can't open directory %s for listing.\n", d.c_str());
//    		return;
//	}
//    	while((ent=readdir(dir)) != NULL) {
//    		// loop until the directory is traveled thru
//
//		// push directory or filename to the list
//		std::stringstream fullname;
//
//		fullname << d << "/" << ent->d_name;
//		s = fullname.str();
//
//		status = stat (s.c_str(), &st_buf);
//
//		if (S_ISDIR (st_buf.st_mode)) continue;
//		else if (S_ISREG (st_buf.st_mode)) {
//			// a file, add to full list.
//			if (exts.empty()) {
//				fileList.push_back(s);
//			} else {
//				// need to check
//				for (std::vector<std::string>::iterator iter=exts.begin();
//					iter != exts.end(); iter++) {
//					if (hasExt(s, *iter)) {
//						fileList.push_back(s);
//						break;
//					}
//				}
//			}
//		} else printf("ERROR: not regular file nor directory.  what is it? %s\n", s.c_str());
//	}
//	// close up
//	closedir(dir);
	traverseDirectory(directory, fileList, FileUtils::FILE, false);

}

void FileUtils::getDirectoriesInDirectory(const std::string & directory, std::vector<std::string> & dirList)
{
//
//
//    struct dirent *ent;
//    std::string d, s;
//	DIR *dir;
//	int status;
//	struct stat st_buf;
//
//	status = stat (directory.c_str(), &st_buf);
//	if (status != 0) {
//		printf("ERROR: unable to inspect directory %s\n", directory.c_str());
//		return;
//	}
//	if (!(S_ISDIR (st_buf.st_mode))) return;
//
//
//	d = directory;
//
//	// open a directory
//    	dir=opendir(d.c_str());
//    	if (dir == NULL) {
//    		printf("ERROR: can't open directory %s for listing.\n", d.c_str());
//    		return;
//	}
//
//	while((ent=readdir(dir)) != NULL) // loop until the directory is traveled thru
//	{
//		// push directory or filename to the list
//		std::stringstream fullname;
//		fullname << directory << "/" << ent->d_name;
//		s = fullname.str();
//
//		status = stat (s.c_str(), &st_buf);
//
//		if (S_ISDIR (st_buf.st_mode)) {
//
//			if (strcmp(ent->d_name, ".") &&
//					strcmp(ent->d_name, "..")) {
//				dirList.push_back(s);
//
//				//printf("here:  found %s\n", s.c_str());
//
//			}
//		} else if (S_ISREG (st_buf.st_mode)) continue;
//		else printf("ERROR: not regular file nor directory.  what is it? %s\n", s.c_str());
//
//	}
//	// close up
//	closedir(dir);
	traverseDirectory(directory, dirList, FileUtils::DIRECTORY, false);
}

bool FileUtils::mkdirs(const std::string & d)
{
	DIR *dir = opendir(d.c_str());
	if (dir == 0 && d.size() > 0) {
		// find the parent
		std::string parent = FileUtils::getDir(d);
		printf("dir to create: %s, parent: %s\n", d.c_str(), parent.c_str());
		if (FileUtils::mkdirs(parent)) {
			int n = mkdir(d.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			if (n != 0) return false;
		} else {
			return false;
		}
	} else {
		closedir(dir);
	}
	return true;
}



