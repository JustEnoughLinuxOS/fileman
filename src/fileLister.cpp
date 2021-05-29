#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <string.h>
#include "fileLister.h"

bool compareNoCase(const T_FILE& p_s1, const T_FILE& p_s2)
{
    return strcasecmp(p_s1.m_name.c_str(), p_s2.m_name.c_str()) <= 0;
}

CFileLister::CFileLister(void)
{
}

CFileLister::~CFileLister(void)
{
}

const bool CFileLister::list(const std::string &p_path)
{
    // Open dir
    DIR *l_dir = opendir(p_path.c_str());
    if (l_dir == NULL)
    {
        std::cerr << "CFileLister::list: Error opening dir " << p_path << std::endl;
        return false;
    }
    // Clean up
    m_listFiles.clear();
    m_listDirs.clear();
    // Read dir
    std::string l_file("");
    std::string l_fileFull("");
    struct stat l_stat;
    struct dirent *l_dirent = readdir(l_dir);
    while (l_dirent != NULL)
    {
        l_file = l_dirent->d_name;
        // Filter the '.' and '..' dirs
        if (l_file != "." && l_file != "..")
        {
            // Stat the file
            l_fileFull = p_path + "/" + l_file;
            if (stat(l_fileFull.c_str(), &l_stat) == -1)
            {
                std::cerr << "CFileLister::list: Error stat " << l_fileFull << std::endl;
            }
            else
            {
                // Check type
                if (S_ISDIR(l_stat.st_mode))
                    // It's a directory
                    m_listDirs.push_back(T_FILE(l_file, l_stat.st_size));
                else
                    // It's a file
                    m_listFiles.push_back(T_FILE(l_file, l_stat.st_size));
            }
        }
        // Next
        l_dirent = readdir(l_dir);
    }
    // Close dir
    closedir(l_dir);
    // Sort lists
    sort(m_listFiles.begin(), m_listFiles.end(), compareNoCase);
    sort(m_listDirs.begin(), m_listDirs.end(), compareNoCase);
    // Add "..", always at the first place
    if (p_path != "/")
        m_listDirs.insert(m_listDirs.begin(), T_FILE("..", 0));
    return true;
}

T_FILE &CFileLister::operator[](const unsigned int p_i)
{
    if (p_i < m_listDirs.size())
        return m_listDirs[p_i];
    else
        return m_listFiles[p_i - m_listDirs.size()];
}

const unsigned int CFileLister::getNbDirs(void) const
{
    return m_listDirs.size();
}

const unsigned int CFileLister::getNbFiles(void) const
{
    return m_listFiles.size();
}

const unsigned int CFileLister::getNbTotal(void) const
{
    return m_listDirs.size() + m_listFiles.size();
}

const bool CFileLister::isDirectory(const unsigned int p_i) const
{
    return p_i < m_listDirs.size();
}

const unsigned int CFileLister::searchDir(const std::string &p_name) const
{
    unsigned int l_ret = 0;
    bool l_found = false;
    // Search name in dirs
    for (std::vector<T_FILE>::const_iterator l_it = m_listDirs.begin(); (!l_found) && (l_it != m_listDirs.end()); ++l_it)
    {
        if ((*l_it).m_name == p_name)
            l_found = true;
        else
            ++l_ret;
    }
    return l_found ? l_ret : 0;
}

// Set selected status for all files (except '..')
void CFileLister::setSelectedAll(const bool p_selected)
{
    auto it = m_listDirs.begin();
    for (; it != m_listDirs.end(); ++it)
        if (it->m_name != "..")
            it->m_selected = p_selected;
    for (it = m_listFiles.begin(); it != m_listFiles.end(); ++it)
        it->m_selected = p_selected;
}

// Number of selected files
const unsigned int CFileLister::getNbSelected(void) const
{
    unsigned int nb = 0;
    auto it = m_listDirs.begin();
    for (; it != m_listDirs.end(); ++it)
        nb += it->m_selected;
    for (it = m_listFiles.begin(); it != m_listFiles.end(); ++it)
        nb += it->m_selected;
    return nb;
}

// Get list of selected files, with full path
void CFileLister::getSelectList(const std::string &p_path, std::vector<std::string> &p_list) const
{
    // Clear existing list
    p_list.clear();
    // Add a '/' between dir and file if necessary
    std::string separator("/");
    if (p_path == "/")
        separator = "";
    // Add selected dirs
    auto it = m_listDirs.begin();
    for (; it != m_listDirs.end(); ++it)
        if (it->m_selected)
            p_list.push_back(p_path + separator + it->m_name);
    // Add selected files
    it = m_listFiles.begin();
    for (; it != m_listFiles.end(); ++it)
        if (it->m_selected)
            p_list.push_back(p_path + separator + it->m_name);
}
