#include "DirFs.h"
#include "FileMan.h"

ST::string DirFs::basePath() {
    return m_basePath;
}

ST::string DirFs::absolutePath(const ST::string &path) {
    return FileMan::joinPaths(m_basePath, path);
}

SGPFile *DirFs::openForWriting(const ST::string &path, bool truncate) {
    return FileMan::openForWriting(absolutePath(path), truncate);
}

SGPFile *DirFs::openForAppend(const ST::string &path) {
    return FileMan::openForAppend(absolutePath(path));
}

SGPFile *DirFs::openForReadWrite(const ST::string &path) {
    return FileMan::openForReadWrite(absolutePath(path));
}

SGPFile *DirFs::openForReading(const ST::string &path) {
    return FileMan::openForReading(absolutePath(path));
}

void DirFs::deleteFile(const ST::string &path) {
    return FileMan::deleteFile(absolutePath(path));
}

void DirFs::createDir(const ST::string &path) {
    return FileMan::createDir(absolutePath(path));
}

void DirFs::eraseDir(const ST::string &path) {
    return FileMan::eraseDir(absolutePath(path));
}

std::vector<ST::string>
DirFs::findFilesInDir(const ST::string &path, const ST::string &ext, bool caseInsensitive, bool returnOnlyNames, bool sortResults, bool recursive) {
    return FileMan::findFilesInDir(absolutePath(path), ext, caseInsensitive, returnOnlyNames, sortResults, recursive);
}

std::vector<ST::string>
DirFs::findAllFilesInDir(const ST::string &path, bool sortResults, bool recursive, bool returnOnlyNames) {
    return FileMan::findAllFilesInDir(absolutePath(path), sortResults, recursive, returnOnlyNames);
}

std::vector<ST::string>
DirFs::findAllDirsInDir(const ST::string &path, bool sortResults, bool recursive, bool returnOnlyNames) {
    return FileMan::findAllDirsInDir(absolutePath(path), sortResults, recursive, returnOnlyNames);
}

ST::string DirFs::resolveExistingComponents(const ST::string &path) {
    return FileMan::resolveExistingComponents(absolutePath(path)).substr(m_basePath.size() + 1);
}

bool DirFs::isFile(const ST::string &path) {
    return FileMan::isFile(absolutePath(path));
}

bool DirFs::isDir(const ST::string &path) {
    return FileMan::isDir(absolutePath(path));
}

bool DirFs::isReadOnly(const ST::string &path) {
    return FileMan::isReadOnly(absolutePath(path));
}

bool DirFs::exists(const ST::string &path) {
    return FileMan::exists(absolutePath(path));
}

void DirFs::moveFile(const ST::string &from, const ST::string &to) {
    return FileMan::moveFile(absolutePath(from), absolutePath(to));
}

double DirFs::getLastModifiedTime(const ST::string &path) {
    return FileMan::getLastModifiedTime(absolutePath(path));
}

uint64_t DirFs::getFreeSpace(const ST::string& path) {
    return FileMan::getFreeSpace(absolutePath(path));
}
